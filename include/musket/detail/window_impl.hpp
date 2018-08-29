//--------------------------------------------------------
// musket/include/musket/detail/window_impl.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_DETAIL_WINDOW_IMPL_HPP_
#define MUSKET_DETAIL_WINDOW_IMPL_HPP_

#include "../window.hpp"
#include <spirea/mp/algorithm.hpp>

namespace musket {

namespace detail {

	struct window_context
	{
		spirea::windows::window wnd;
		spirea::d2d1::hwnd_render_target rt;
		spirea::d2d1::color_f bg_color;
		event_handler< window_events, detail::event_handler_element_to_widget > to_widget_handler;
		event_handler< default_window_events > window_events_handler;

		bool mouse_entered = false;

		template <typename Rect, typename Color, typename T>
		window_context(Rect const& rc, std::string_view caption, Color const& bg_color, T) :
			bg_color{ rgba_color_traits< spirea::d2d1::color_f >::construct( bg_color ) }
		{ 
			auto trc = spirea::rect_traits< RECT >::construct( rc );
			auto const dpi = spirea::try_result( spirea::windows::api::get_dpi_for_monitor( 
				spirea::windows::api::monitor_from_point( { trc.left, trc.top } ) 
			) );
			constexpr auto default_dpi = spirea::windows::api::user_default_screen_dpi< LONG >;

			auto const width = ( trc.right - trc.left ) * static_cast< LONG >( dpi.x ) / default_dpi;
			auto const height = ( trc.bottom - trc.top ) * static_cast< LONG >( dpi.y ) / default_dpi;

			wnd = { caption, T::style, T::ex_style, trc.left, trc.top, width, height };

			recreate_target();
		}

		void recreate_target()
		{
			rt.reset();
			auto const rc = wnd.get_client_rect();
			
			spirea::windows::try_hresult( context().d2d1->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties( 
					wnd.handle(), 
					{ 
						static_cast< UINT >( spirea::rect_traits< RECT >::width( rc ) ),
						static_cast< UINT >( spirea::rect_traits< RECT >::height( rc ) ),
					} 
				),
				rt.pp()
			) );

			float const dpi = static_cast< float >( spirea::windows::api::get_dpi_for_window( wnd ) );
			rt->SetDpi( dpi, dpi );
		}
	};

	inline void conect_mouse_events(window& wnd, std::shared_ptr< window_context > wc)
	{
		auto mouse_position = [](spirea::windows::window const& w, LPARAM lparam) -> spirea::point_t< std::int32_t > {
			auto const dpi = spirea::windows::api::get_dpi_for_window( w );
			constexpr auto const default_dpi = spirea::windows::api::user_default_screen_dpi< std::int32_t >;
			return {
				GET_X_LPARAM( lparam ) * default_dpi / static_cast< std::int32_t >( dpi ),
				GET_Y_LPARAM( lparam ) * default_dpi / static_cast< std::int32_t >( dpi ),
			};
		};

		auto invoker = [wnd, wc, mouse_position](auto event, mouse_button btn, spirea::windows::window const& w, WPARAM wparam, LPARAM lparam) mutable {
			auto const pt = mouse_position( w, lparam );
			wc->window_events_handler.invoke( decltype( event ){}, wnd, btn, static_cast< mouse_button >( wparam ), pt );
			wc->to_widget_handler.invoke( decltype( event ){}, pt, wnd, btn, static_cast< mouse_button >( wparam ), pt );
			return 0;
		};

		auto pressed_event = [invoker](auto event, mouse_button btn, spirea::windows::window const& w, WPARAM wparam, LPARAM lparam) mutable {
			auto const res = invoker( decltype( event ){}, btn, w, wparam, lparam );
			SetCapture( w.handle() );
			return res;
		};

		auto released_event = [wc, invoker](auto event, mouse_button btn, spirea::windows::window const& w, WPARAM wparam, LPARAM lparam) mutable {
			auto const res = invoker( decltype( event ){}, btn, w, wparam, lparam );
			ReleaseCapture();
			wc->window_events_handler.shrink_to_fit( window_event::mouse_moved{} );
			return res;
		};

		wc->wnd.connect( WM_LBUTTONDOWN, [pressed_event](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return pressed_event( window_event::mouse_button_pressed{}, mouse_button::left, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_RBUTTONDOWN, [pressed_event](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return pressed_event( window_event::mouse_button_pressed{}, mouse_button::right, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_MBUTTONDOWN, [pressed_event](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return pressed_event( window_event::mouse_button_pressed{}, mouse_button::middle, wnd, wparam, lparam );
		} );

		wc->wnd.connect( WM_LBUTTONUP, [released_event](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return released_event( window_event::mouse_button_released{}, mouse_button::left, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_RBUTTONUP, [released_event](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return released_event( window_event::mouse_button_released{}, mouse_button::right, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_MBUTTONUP, [released_event](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return released_event( window_event::mouse_button_released{}, mouse_button::middle, wnd, wparam, lparam );
		} );

		wc->wnd.connect( WM_MOUSEMOVE, [wnd, wc, mouse_position](spirea::windows::window w, WPARAM wparam, LPARAM lparam) mutable {
			if( !wc->mouse_entered ) {
				TRACKMOUSEEVENT tm = {};
				tm.cbSize = sizeof( TRACKMOUSEEVENT );
				tm.dwFlags = TME_LEAVE;
				tm.hwndTrack = wc->wnd.handle();
				tm.dwHoverTime = HOVER_DEFAULT;
				TrackMouseEvent( &tm );

				wc->mouse_entered = true;
			}

			auto const pt = mouse_position( w, lparam );
			wc->window_events_handler.invoke( window_event::mouse_moved{}, wnd, static_cast< mouse_button >( wparam ), pt );
			wc->to_widget_handler.invoke( window_event::detail::mouse_moved_distributor{}, pt, wnd, static_cast< mouse_button >( wparam ) );
			return 0;
		} );

		wc->wnd.connect( WM_MOUSELEAVE, [wnd, wc](spirea::windows::window, WPARAM, LPARAM) mutable {
			wc->to_widget_handler.invoke( window_event::detail::mouse_moved_distributor{}, window_event::mouse_leaved{}, wnd, get_mouse_button_states() );
			wc->mouse_entered = false;
			return 0;
		} );
	}

} // namespace detail

	template <typename Rect, typename Color, typename T>
	inline window::window(Rect const& rc, std::string_view caption,  Color const& bg_color, T) :
		p_{ std::make_shared< detail::window_context >( rc, caption, bg_color, T{} ) }
	{
		detail::conect_mouse_events( *this, p_ ); 

		p_->wnd.connect( WM_PAINT, [this](spirea::windows::window, WPARAM, LPARAM) -> LRESULT {
			auto ps = spirea::windows::api::begin_paint( p_->wnd.handle() );

			p_->rt->BeginDraw();
			p_->rt->Clear( p_->bg_color );

			p_->window_events_handler.invoke( window_event::draw{}, *this );
			p_->to_widget_handler.invoke( window_event::draw{}, *this );

			auto const res = p_->rt->EndDraw();
			if( res != S_OK ) {
				if( res == D2DERR_RECREATE_TARGET ) {
					p_->recreate_target();
					p_->window_events_handler.invoke( window_event::recreated_target{}, *this );
					p_->to_widget_handler.invoke( window_event::recreated_target{}, *this );
				}
				else {
					throw spirea::windows::hresult_error( res );
				}
			}

			return 0;
		} );

		p_->wnd.connect( WM_SIZE, [this](spirea::windows::window, WPARAM, LPARAM lparam) -> LRESULT {
			auto const rc = p_->wnd.get_client_rect();
			auto const width = static_cast< std::uint32_t >( rc.right - rc.left );
			auto const height = static_cast< std::uint32_t >( rc.bottom - rc.top );

			spirea::windows::try_hresult( p_->rt->Resize( { width, height } ) );

			auto const dpi = spirea::windows::api::get_dpi_for_window( p_->wnd );
			constexpr auto default_dpi = spirea::windows::api::user_default_screen_dpi< std::uint32_t >;

			spirea::area_t< std::uint32_t > sz = {
				width * default_dpi / dpi,
				height * default_dpi / dpi,
			};

			p_->window_events_handler.invoke( window_event::resized{}, *this, sz );
			p_->to_widget_handler.invoke( window_event::resized{}, *this, sz );

			return 0;
		} );

		p_->wnd.connect( WM_SIZING, [this](spirea::windows::window, WPARAM wparam, LPARAM lparam) -> LRESULT {
			auto const prev_rc = spirea::rect_traits< spirea::rect_t< float > >::construct( p_->wnd.get_window_rect() );
			auto const next_rc = spirea::rect_traits< spirea::rect_t< float > >::construct( *reinterpret_cast< RECT* >( lparam ) );
			auto const dpi = spirea::windows::api::get_dpi_for_window( p_->wnd );
			constexpr auto default_dpi = spirea::windows::api::user_default_screen_dpi< float >;

			auto const offset = spirea::point_t{ 
				( next_rc.width() - prev_rc.width() ) * default_dpi / dpi, 
				( next_rc.height() - prev_rc.height() ) * default_dpi / dpi,
			};

			p_->to_widget_handler.invoke( window_event::detail::auto_resize{}, *this, offset );
			p_->to_widget_handler.invoke( window_event::detail::auto_relocation{}, *this, offset );

			return DefWindowProcW( p_->wnd.handle(), WM_SIZING, wparam, lparam );
		} );

		p_->wnd.connect( WM_DPICHANGED, [this](spirea::windows::window, WPARAM, LPARAM lparam) -> LRESULT {
			auto const& rc = *reinterpret_cast< RECT const* >( lparam );
			SetWindowPos( p_->wnd.handle(), nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE );

			auto const dpi = spirea::windows::api::get_dpi_for_window( p_->wnd );
			p_->rt->SetDpi( static_cast< float >( dpi ), static_cast< float >( dpi ) );

			return 0;
		} );

		p_->wnd.connect_idle( [this](spirea::windows::window) {
			p_->window_events_handler.invoke( window_event::idle{}, *this );
			p_->to_widget_handler.invoke( window_event::idle{}, *this );
		} );
	}

	inline void window::show() noexcept
	{
		assert( p_ );
		p_->wnd.show();
	}

	inline void window::hide() noexcept
	{
		assert( p_ );
		p_->wnd.show();
	}

	inline void window::redraw() const noexcept
	{
		assert( p_ );
		spirea::windows::api::invalidate_rect( p_->wnd, nullptr, false );
	}

	inline void window::close() noexcept 
	{
		assert( p_ );
		p_->wnd.close();
	}

	spirea::rect_t< float > window::client_area_size() const noexcept
	{
		auto rc = spirea::rect_traits< spirea::rect_t< float > >::construct( p_->wnd.get_client_rect() );
		auto const dpi = spirea::windows::api::get_dpi_for_window( p_->wnd );
		constexpr auto default_dpi = spirea::windows::api::user_default_screen_dpi< float >;

		rc.right = rc.right * default_dpi / dpi;
		rc.bottom = rc.bottom * default_dpi / dpi;

		return rc;
	}

	inline spirea::windows::window window::window_handle() const noexcept
	{
		assert( p_ );
		return p_->wnd;
	}

	inline spirea::d2d1::hwnd_render_target window::render_target() const noexcept
	{
		assert( p_ );
		return p_->rt;
	}

	template <typename T>
	inline void window::attach_widget(widget< T >& w)
	{
		assert( p_ );
		w.set_window( p_, connect_events( p_->to_widget_handler, w ) );

		if constexpr( has_on_event< widget< T >, window_event::attached_widget >::value ) {
			w->on_event( window_event::attached_widget{}, *this );
		}
		if constexpr( has_on_event< widget< T >, window_event::recreated_target >::value ) {
			w->on_event( window_event::recreated_target{}, *this );
		}
	}

	template <typename Event, typename F>
	inline spirea::connection window::connect(Event, F&& f)
	{
		assert( p_ );
		return p_->window_events_handler.connect( Event{}, std::forward< F >( f ) );
	}

} // namespace musket

#endif // MUSKET_DETAIL_WINDOW_IMPL_HPP_