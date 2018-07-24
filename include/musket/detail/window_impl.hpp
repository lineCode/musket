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

namespace musket {

namespace detail {

	struct window_context
	{
		spirea::windows::window wnd;
		spirea::d2d1::hwnd_render_target rt;
		spirea::d2d1::color_f bg_color;
		event_handler< window_events > event_handler;

		template <typename Rect, typename Color, typename T>
		window_context(std::string_view caption, Rect const& rc, Color const& bg_color, T) :
			wnd{ caption, T::style, T::ex_style, spirea::rect_traits< RECT >::construct( rc ) },
			bg_color{ rgba_color_traits< spirea::d2d1::color_f >::construct( bg_color ) }
		{ 
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
			wc->event_handler.invoke( decltype( event ){}, pt, wnd, btn, static_cast< mouse_button >( wparam ), pt );
			return 0;
		};

		wc->wnd.connect( WM_LBUTTONDOWN, [invoker](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return invoker( window_event::mouse_button_pressed{}, mouse_button::left, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_RBUTTONDOWN, [invoker](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return invoker( window_event::mouse_button_pressed{}, mouse_button::right, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_MBUTTONDOWN, [invoker](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return invoker( window_event::mouse_button_pressed{}, mouse_button::middle, wnd, wparam, lparam );
		} );

		wc->wnd.connect( WM_LBUTTONUP, [invoker](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return invoker( window_event::mouse_button_released{}, mouse_button::left, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_RBUTTONUP, [invoker](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return invoker( window_event::mouse_button_released{}, mouse_button::right, wnd, wparam, lparam );
		} );
		wc->wnd.connect( WM_MBUTTONUP, [invoker](spirea::windows::window wnd, WPARAM wparam, LPARAM lparam) mutable {
			return invoker( window_event::mouse_button_released{}, mouse_button::middle, wnd, wparam, lparam );
		} );

		wc->wnd.connect( WM_MOUSEMOVE, [wnd, wc, invoker, mouse_position](spirea::windows::window w, WPARAM wparam, LPARAM lparam) mutable {
			auto const pt = mouse_position( w, lparam );
			wc->event_handler.invoke( window_event::detail::mouse_moved_distributor{}, pt, wnd, static_cast< mouse_button >( wparam ) );
			return 0;
		} );
	}

} // namespace detail

	template <typename Rect, typename Color, typename T>
	inline window::window(std::string_view caption, Rect const& rc, Color const& bg_color, T) :
		p_{ std::make_shared< detail::window_context >( caption, rc, bg_color, T{} ) }
	{
		detail::conect_mouse_events( *this, p_ ); 

		p_->wnd.connect( WM_PAINT, [this](spirea::windows::window, WPARAM, LPARAM) -> LRESULT {
			auto ps = spirea::windows::api::begin_paint( p_->wnd.handle() );

			p_->rt->BeginDraw();
			p_->rt->Clear( p_->bg_color );

			p_->event_handler.invoke( window_event::draw{}, *this );

			auto const res = p_->rt->EndDraw();
			if( res != S_OK ) {
				if( res == D2DERR_RECREATE_TARGET ) {
					p_->recreate_target();
					p_->event_handler.invoke( window_event::recreated_target{}, *this );
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
			p_->event_handler.invoke( window_event::resize{}, *this, spirea::area_t{ width, height } );

			return 0;
		} );

		p_->wnd.connect( WM_DPICHANGED, [this](spirea::windows::window, WPARAM, LPARAM lparam) -> LRESULT {
			auto const& rc = *reinterpret_cast< RECT const* >( lparam );
			SetWindowPos( p_->wnd.handle(), nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE );

			auto const dpi = spirea::windows::api::get_dpi_for_window( p_->wnd );
			p_->rt->SetDpi( static_cast< float >( dpi ), static_cast< float >( dpi ) );

			return 0;
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
		w.set_window( p_, connect_events( p_->event_handler, w ) );

		if constexpr( has_on_event< widget< T >, window_event::recreated_target >::value ) {
			w->on_event( window_event::recreated_target{}, *this );
		}
	}

} // namespace musket

#endif // MUSKET_DETAIL_WINDOW_IMPL_HPP_