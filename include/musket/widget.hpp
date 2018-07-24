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
		std::unique_ptr< event_connections< window_events >, connection_deleter > conns;
		std::weak_ptr< detail::window_context > wnd;
	};

	struct weak_widget_tag
	{ };

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
			return !p_->wnd.expired();
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
			p_->conns.reset( new event_connections< window_events >{ std::move( conns ) } );
			p_->wnd = wnd;
		}

		friend class window;
	};

} // namespace musket

#endif // MUSKET_WIDGET_HPP_