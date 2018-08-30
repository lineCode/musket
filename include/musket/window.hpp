//--------------------------------------------------------
// musket/include/musket/window.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_WINDOW_HPP_
#define MUSKET_WINDOW_HPP_

#include <spirea/windows/api.hpp>
#include <spirea/windows/window.hpp>
#include <spirea/windows/d2d1.hpp>
#include "geometry.hpp"
#include "color.hpp"
#include "context.hpp"
#include "event.hpp"

namespace musket {

namespace window_type {

	struct overlapped
	{ 
		static constexpr DWORD style = WS_OVERLAPPEDWINDOW;
		static constexpr DWORD ex_style = 0;
	};

	struct tool
	{
		static constexpr DWORD style = WS_OVERLAPPED | WS_SYSMENU;
		static constexpr DWORD ex_style = WS_EX_TOOLWINDOW;
	};

	struct popup
	{
		static constexpr DWORD style = WS_POPUP;
		static constexpr DWORD ex_style = 0;
	};

} // namespace window_type

namespace detail {

	struct window_context;

} // namespace detail

	using window_events = events_holder<
		event::idle,
		event::draw,
		event::recreated_target,
		event::resized,
		event::mouse_button_pressed,
		event::mouse_button_released,
		event::detail::mouse_moved_distributor,
		event::detail::auto_resize,
		event::detail::auto_relocation
	>;

	using default_window_events = events_holder<
		event::idle,
		event::draw,
		event::recreated_target,
		event::resized,
		event::mouse_button_pressed,
		event::mouse_button_released,
		event::mouse_entered,
		event::mouse_leaved,
		event::mouse_moved
	>;

	template <typename>
	class widget;
	
	class window
	{
		std::shared_ptr< detail::window_context > p_;

	public:
		window() = default;
		window(window const&) = default;
		window(window&&) = default;
		window& operator=(window const&) = default;
		window& operator=(window&&) = default;

		template <typename Rect, typename Color = rgba_color_t, typename T = window_type::overlapped>
		window(
			Rect const& rc, 
			std::string_view caption, 
			Color const& bg_color = rgba_color_t{ 0.2f, 0.2f, 0.2f, 0.0f }, 
			T = window_type::overlapped{}
		);

		void show() noexcept;
		void hide() noexcept;
		void redraw() const noexcept;
		void close() noexcept;

		spirea::rect_t< float > client_area_size() const noexcept;

		spirea::windows::window window_handle() const noexcept;
		spirea::d2d1::hwnd_render_target render_target() const noexcept;

		template <typename T>
		void attach_widget(widget< T >& w);

		template <typename Event, typename F>
		spirea::connection connect(Event, F&& f);
	};

	inline int loop()
	{
		return spirea::windows::window::loop();
	}

	inline int idle_loop()
	{
		return spirea::windows::window::idle_loop();
	}

} // namespace musket

#endif // MUSKET_WINDOW_HPP_