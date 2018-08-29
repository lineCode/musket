//--------------------------------------------------------
// musket/include/musket/event.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_EVENT_HPP_
#define MUSKET_EVENT_HPP_

#include <tuple>
#include <spirea/signal.hpp>
#include "geometry.hpp"
#include "device.hpp"

namespace musket {

	class window;

	template <typename T, typename... Events>
	struct events_holder
	{ };

namespace window_event {

	struct idle
	{
		using type = void (window&);
	};

	struct draw
	{
		using type = void (window&);
	};

	struct recreated_target
	{
		using type = void (window&);
	};

	struct resized
	{
		using type = void (window&, spirea::area_t< std::uint32_t > const&);
	};

	struct attached_widget
	{
		using type = void (window&);
	};

	struct mouse_button_pressed
	{
		using type = void (window&, mouse_button, mouse_button, spirea::point_t< std::int32_t > const&);
	};

	struct mouse_button_released
	{
		using type = void (window&, mouse_button, mouse_button, spirea::point_t< std::int32_t > const&);
	};

namespace detail {

	struct mouse_moved_distributor
	{
		using type = void (window&, mouse_button);
	};

} // namespace detail

	struct mouse_moved
	{
		using type = void (window&, mouse_button, spirea::point_t< std::int32_t > const&);
	};

	struct mouse_entered
	{
		using type = void (window&, mouse_button);
	};

	struct mouse_leaved
	{
		using type = void (window&, mouse_button);
	};

namespace detail {

	struct auto_resize
	{
		using type = void (window&, spirea::point_t< float > const&);
	};

	struct auto_relocation
	{
		using type = void (window&, spirea::point_t< float > const&);
	};

} // namespace detail

} // namespace window_event

	using window_events = events_holder<
		window_event::idle,
		window_event::draw,
		window_event::recreated_target,
		window_event::resized,
		window_event::attached_widget,
		window_event::mouse_button_pressed,
		window_event::mouse_button_released,
		window_event::detail::mouse_moved_distributor,
		window_event::detail::auto_resize,
		window_event::detail::auto_relocation
	>;

	using default_window_events = events_holder<
		window_event::idle,
		window_event::draw,
		window_event::recreated_target,
		window_event::resized,
		window_event::attached_widget,
		window_event::mouse_button_pressed,
		window_event::mouse_button_released,
		window_event::mouse_entered,
		window_event::mouse_leaved,
		window_event::mouse_moved
	>;

	template <typename Widget, typename Event, typename = typename Event::type, typename = void>
	struct has_on_event :
		public std::false_type
	{ };

	template <typename Widget, typename Event, typename R, typename... Args>
	struct has_on_event< 
		Widget, Event, R (Args...), 
		std::void_t< decltype( std::declval< Widget >()->on_event( Event{}, std::declval< Args >()... ) ) >
	> :
		public std::true_type
	{ };

namespace detail {
	
	template <typename Event, typename EventFunc = typename Event::type, typename = void>
	class event_handler_element_default
	{
		spirea::signal< EventFunc, spirea::signal_option::fold > signal_;

	public:
		template <typename F>
		spirea::connection connect(F&& f)
		{
			return signal_.connect( std::forward< F >( f ) );
		}

		template <typename... Args>
		auto invoke(Args&&... args)
		{
			return signal_.invoke( std::forward< Args >( args )... );
		}

		void shrink_to_fit()
		{
			signal_.shrink_to_fit();
		}
	};

	template <typename Event, typename EventFunc = typename Event::type, typename = void>
	class event_handler_element_to_widget
	{
		spirea::signal< EventFunc, spirea::signal_option::fold > signal_;

	public:
		template <typename F>
		spirea::connection connect(F&& f)
		{
			return signal_.connect( std::forward< F >( f ) );
		}

		template <typename... Args>
		auto invoke(Args&&... args)
		{
			return signal_.invoke( std::forward< Args >( args )... );
		}

		void shrink_to_fit()
		{
			signal_.shrink_to_fit();
		}
	};

	template <typename Event, typename... Args>
	class event_handler_element_to_widget< Event, void (Args...), std::enable_if_t<
		std::is_same_v< Event, window_event::mouse_button_pressed >
		|| std::is_same_v< Event, window_event::mouse_button_released > 
	> >
	{
		spirea::signal< bool (spirea::point_t< std::int32_t > const&, Args...), spirea::signal_option::maybe > signal_;

	public:
		template <typename Widget>
		spirea::connection connect(Widget& w)
		{
			auto wp = w.weak();
			return signal_.connect( [wp](spirea::point_t< std::int32_t > const& pt, Args... args) -> bool {
				auto w = Widget{ wp };
				auto const rc = w->size();
				if( pt.x >= rc.left && pt.x <= rc.right && pt.y >= rc.top && pt.y <= rc.bottom ) {
					w->on_event( Event{}, args... );
					return false;
				}
				return true;
			} );
		}

		template <typename... As>
		void invoke(spirea::point_t< std::int32_t > const& pt, As&&... args)
		{
			signal_.invoke( pt, std::forward< As >( args )... );
		}

		void shrink_to_fit()
		{
			signal_.shrink_to_fit();
		}
	};

	template <typename Event, typename... Args>
	class event_handler_element_to_widget< Event, void (Args...), std::enable_if_t<
		std::is_same_v< Event, window_event::detail::mouse_moved_distributor > 
	> >
	{
		struct object
		{
			std::weak_ptr< void > wp;
			std::function< spirea::rect_t< float >() > size;
			std::function< typename window_event::mouse_moved::type > moved;
			std::function< typename window_event::mouse_entered::type > entered;
			std::function< typename window_event::mouse_leaved::type > leaved;
		};

		using container_type = std::vector< std::shared_ptr< object > >;

		spirea::signal< void (container_type&) > signal_;
		container_type buf_;
		std::shared_ptr< object > over_;

	public:
		template <typename Widget>
		spirea::connection connect(Widget& w)
		{
			auto wp = w.weak();
			auto const obj = std::make_shared< object >();

			obj->wp = wp;
			obj->size = [wp](){ auto w = Widget{ wp }; return w->size(); };

			if constexpr( has_on_event< Widget, window_event::mouse_moved >::value ) {
				obj->moved = [wp](Args... args, spirea::point_t< std::int32_t > const& pt){ 
					auto w = Widget{ wp }; w->on_event( window_event::mouse_moved{}, args..., pt ); 
				};
			}
			if constexpr( has_on_event< Widget, window_event::mouse_entered >::value ) {
				obj->entered = [wp](Args... args){ auto w = Widget{ wp }; w->on_event( window_event::mouse_entered{}, args... ); };
			}
			if constexpr( has_on_event< Widget, window_event::mouse_leaved >::value ) {
				obj->leaved = [wp](Args... args){ auto w = Widget{ wp }; w->on_event( window_event::mouse_leaved{}, args... ); };
			}

			return signal_.connect( [obj](container_type& buf) {
				buf.push_back( obj );
			} );
		}

		template <typename... As>
		void invoke(spirea::point_t< std::int32_t > const& pt, As&&... args)
		{
			buf_.clear();
			signal_.invoke( buf_ );

			std::shared_ptr< object > hit;
			for( auto itr = buf_.rbegin(); itr != buf_.rend(); ++itr ) {
				auto const rc = (*itr)->size();
				if( pt.x >= rc.left && pt.x <= rc.right && pt.y >= rc.top && pt.y <= rc.bottom ) {
					hit = *itr;
					break;
				}
			}

			if( hit ) {
				if( over_ ) {
					if( over_->wp.lock() == hit->wp.lock() ) {
						if( over_->moved ) {
							over_->moved( std::forward< Args >( args )..., pt );
						}
						return;
					}
					if( over_->leaved ) {
						over_->leaved( std::forward< Args >( args )... );
					}
				}

				if( hit->entered ) {
					hit->entered( std::forward< Args >( args )... );
				}
			}
			else {
				if( over_ && over_->leaved ) {
					over_->leaved( std::forward< Args >( args )... );
				}
			}
			over_ = hit;
		}

		template <typename... As>
		void invoke(window_event::mouse_leaved, As&&... args)
		{
			if( over_ ) {
				if( over_->leaved ) {
					over_->leaved( std::forward< As >( args )... );
				}
				over_ = {};
			}
		}

		void shrink_to_fit()
		{
			signal_.shrink_to_fit();
		}
	};

} // namespace detail

	template <typename Events, template <typename...> typename Element = detail::event_handler_element_default>
	class event_handler;

	template <typename... Events, template <typename...> typename Element>
	class event_handler< events_holder< Events... >, Element >
	{
		std::tuple< Element< Events >... > table_;

	public:
		template <typename Event, typename... Args>
		spirea::connection connect(Event, Args&&... args)
		{
			return std::get< Element< Event > >( table_ ).connect( std::forward< Args >( args )... );
		}

		template <typename Event, typename... Args>
		auto invoke(Event, Args&&... args)
		{
			return std::get< Element< Event > >( table_ ).invoke( std::forward< Args >( args )... );
		}

		template <typename Event>
		void shrink_to_fit(Event)
		{
			std::get< Element< Event > >( table_ ).shrink_to_fit();
		}
	};

	template <typename Events>
	class event_connections;

	template <typename... Events>
	class event_connections< events_holder< Events... > >
	{
		template <typename Event>
		struct element
		{
			spirea::connection conn;
		};

		std::tuple< element< Events >... > conns_;

	public:
		template <typename Event>
		void assign(Event, spirea::connection const& conn)
		{
			std::get< element< Event > >( conns_ ).conn = conn;
		}

		void disconnect_all()
		{
			( ..., std::get< element< Events > >( conns_ ).conn.disconnect() );
		}
	};

namespace detail {

	template <typename Events, template <typename...> typename Element, typename Event, typename R, typename... Args, typename Widget>
	inline spirea::connection connect_event_helper(event_handler< Events, Element >& eh, Event, R (*)(Args...), Widget& w)
	{
		if constexpr( 
			std::is_same_v< Event, window_event::mouse_button_pressed > 
			|| std::is_same_v< Event, window_event::mouse_button_released > 
			|| std::is_same_v< Event, window_event::detail::mouse_moved_distributor > 
		) {
			return eh.connect( Event{}, w );
		}
		else {
			return eh.connect( Event{}, [w](Args... args) mutable { return w->on_event( Event{}, args... ); } );
		}
	}

	template <typename Events, template <typename...> typename Element, typename Event, typename Widget>
	inline void connect_events_impl(event_handler< Events, Element >& eh, Event, Widget& w, event_connections< Events >& conns)
	{
		if constexpr( std::is_same_v< Event, window_event::detail::mouse_moved_distributor > ) {
			constexpr bool has_events = 
				has_on_event< Widget, window_event::mouse_moved >::value
				|| has_on_event< Widget, window_event::mouse_entered >::value
				|| has_on_event< Widget, window_event::mouse_leaved >::value;
			if constexpr( has_events ) {
				conns.assign( Event{}, connect_event_helper( eh, Event{}, std::add_pointer_t< typename Event::type >{}, w ) );
			}
		}
		if constexpr( has_on_event< Widget, Event >::value ) {
			conns.assign( Event{}, connect_event_helper( eh, Event{}, std::add_pointer_t< typename Event::type >{}, w ) );
		}
	}

} // namespace detail

	template <typename... Events, typename Widget, template <typename...> typename Element>
	inline event_connections< events_holder< Events... > > connect_events(event_handler< events_holder< Events... >, Element >& eh, Widget& w)
	{
		event_connections< events_holder< Events... > > conns;
		( ..., detail::connect_events_impl( eh, Events{}, w, conns ) );
		return conns;
	}

} // namespace musket

#endif // MUSKET_EVENT_HPP_