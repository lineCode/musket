//--------------------------------------------------------
// musket/include/musket/widget.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_WIDGET_HPP_
#define MUSKET_WIDGET_HPP_

#include <memory>
#include <cassert>
#include <type_traits>
#include "operator.hpp"
#include "geometry.hpp"
#include "color.hpp"
#include "event.hpp"
#include "window.hpp"

namespace musket {

	template <typename T>
	class widget;

namespace widget_event {

	struct detached
	{
		using type = void ();
	};

} // namespace widget_event

namespace detail {

	struct window_context;

	template <typename T>
	struct widget_object
	{
		struct connection_deleter
		{
			void operator()(event_connections< window_events >* p) const noexcept
			{
				p->disconnect_all();
				delete p;
			}
		};

		std::unique_ptr< T > handle;
		std::weak_ptr< detail::window_context > wnd;
		std::unique_ptr< event_connections< window_events >, connection_deleter > conns;

		void attach(std::shared_ptr< detail::window_context >& w, event_connections< window_events >&& c)
		{
			wnd = w;
			conns.reset( new event_connections< window_events >{ std::move( c ) } );
		}

		void detach()
		{
			wnd.reset();
			conns.reset();

			if constexpr( has_on_event< T, widget_event::detached >::value ) {
				handle->on_event( widget_event::detached{} );
			}
		}
	};

} // namespace detail

	template <typename T>
	class widget
	{
		std::shared_ptr< detail::widget_object< T > > p_;

	public:
		using type = T;

		widget() = default;
		widget(widget const&) = default;
		widget(widget&&) = default;
		widget& operator=(widget const&) = default;
		widget& operator=(widget&&) = default;

		template <
			typename Arg, typename... Args, 
			std::enable_if_t< !std::is_same_v< Arg, std::weak_ptr< detail::widget_object< T > > >, std::nullptr_t > = nullptr
		>
		widget(Arg&& arg, Args&&... args) :
			p_{ new detail::widget_object< T >{
				std::unique_ptr< T >{ new T{ std::forward< Arg >( arg ), std::forward< Args >( args )... } },
				{}, {}
			} }
		{ }

		explicit widget(std::weak_ptr< detail::widget_object< T > > const& wp) :
			p_{ wp.lock() }
		{ }

		bool is_attached() const noexcept
		{
			if( !p_ ) {
				return false;
			}
			return !( p_->wnd.expired() );
		}

		void detach() noexcept
		{
			if( is_attached() ) {
				p_->detach();
			}
		}
		
		T* operator->() noexcept
		{
			assert( p_ );
			return p_->handle.get();
		}

		T const* operator->() const noexcept
		{
			assert( p_ );
			return p_->handle.get();
		}

		explicit operator bool() const noexcept
		{
			return static_cast< bool >( p_ );
		}

		std::weak_ptr< detail::widget_object< T > > weak() const noexcept
		{
			return { p_ };
		}

	private:
		void set_window(std::shared_ptr< detail::window_context >& wnd, event_connections< window_events >&& conns)
		{
			p_->attach( wnd, std::move( conns ) );
		}

		friend class window;
	};

	template <typename T>
	inline bool is_attached(widget< T > const& w) noexcept
	{
		return w.is_attached();
	}

	template <typename T>
	inline void detach(widget< T >& w) noexcept
	{
		w.detach();
	}

} // namespace musket

#endif // MUSKET_WIDGET_HPP_