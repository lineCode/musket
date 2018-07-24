//--------------------------------------------------------
// musket/include/musket/widget/button.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_INCLUDE_MUSKET_WIDGET_BUTTON_HPP_
#define MUSKET_INCLUDE_MUSKET_WIDGET_BUTTON_HPP_

#include "facade.hpp"

namespace musket {

	struct button_style
	{
		rgba_color_t bg_color;
		std::optional< rgba_color_t > edge_color;
		rgba_color_t text_color;
		float edge_size = 1.0f;
	};

namespace button_event {

	struct pressed
	{
		using type = void (spirea::point_t< std::int32_t > const&);
	};

	struct released
	{
		using type = void (spirea::point_t< std::int32_t > const&);
	};

} // namespace button_event

	using button_events = events_holder<
		button_event::pressed,
		button_event::released
	>;

	class button :
		public widget_facade
	{
		enum struct state : std::uint8_t
		{
			default_, over, pressed,
		};

		enum struct color : std::uint8_t
		{
			bg, edge, text,
		};

		struct style_data
		{
			button_style style;
			brush_holder< color, color::bg, optional_brush( color::edge ), color::text > brush;
		};

		using state_machine_type = state_machine< 
			style_data, state, 
			state::default_, state::over, state::pressed 
		>;

		std::string str_;
		state_machine_type states_;
		spirea::dwrite::text_layout text_;
		event_handler< button_events > event_handler_;

	public:
		template <typename Rect>
		button(
			std::string_view str, 
			Rect const& rc,
			font_format const& font,
			button_style const& default_style,
			button_style const& over_style,
			button_style const& pressed_style
		) :
			widget_facade{ rc },
			str_{ str.begin(), str.end() },
			states_{ 
				state::default_, 
				style_data{ default_style, {} }, style_data{ over_style, {} }, style_data{ pressed_style, {} } 
			}
		{
			auto const sz = size();
			spirea::dwrite::text_format format = create_text_format( font, ( sz.bottom - sz.top ) * 0.7f );
			text_ = create_text_layout( format, sz, str );
			text_->SetTextAlignment( spirea::dwrite::text_alignment::center );
			text_->SetParagraphAlignment( spirea::dwrite::paragraph_alignment::far );
		}

		template <typename Event, typename F>
		spirea::connection connect(Event, F&& f)
		{
			return event_handler_.connect( Event{}, std::forward< F >( f ) );
		}

		void on_event(window_event::draw, window& wnd)
		{
			if( !is_visible() ) {
				return;
			}

			auto const rc = spirea::rect_traits< spirea::d2d1::rect_f >::construct( size() );
			auto const rt = wnd.render_target();
			auto const& data = states_.get();

			rt->FillRectangle( rc, data.brush[color::bg] );
			if( data.style.edge_color ) {
				rt->DrawRectangle( rc, data.brush[color::edge], data.style.edge_size );
			}
			rt->DrawTextLayout( { rc.left, rc.top }, text_.get(), data.brush[color::text], spirea::d2d1::draw_text_options::clip );
		}

		void on_event(window_event::recreated_target, window& wnd)
		{
			auto create_brushes = [&](style_data& sd) {
				sd.brush = { wnd.render_target(), sd.style.bg_color, sd.style.edge_color, sd.style.text_color };
			};

			for( auto& i : states_.data() ) {
				create_brushes( i );
			}
		}

		void on_event(window_event::mouse_button_pressed, window& wnd, mouse_button btn, mouse_button, spirea::point_t< std::int32_t > const& pt)
		{
			if( spirea::enabled( btn, mouse_button::left ) ) {
				states_.trasition( state::pressed );
				event_handler_.invoke( button_event::pressed{}, pt );
				wnd.redraw();
			}
		}

		void on_event(window_event::mouse_button_released, window& wnd, mouse_button btn, mouse_button, spirea::point_t< std::int32_t > const& pt)
		{
			if( spirea::enabled( btn, mouse_button::left ) ) {
				states_.trasition( state::default_ );
				event_handler_.invoke( button_event::released{}, pt );
				wnd.redraw();
			}
		}

		void on_event(window_event::mouse_entered, window& wnd, mouse_button btns)
		{
			if( spirea::enabled( btns, mouse_button::left ) ) {
				states_.trasition( state::pressed );
			}
			else {
				states_.trasition( state::over );
			}
			wnd.redraw();
		}

		void on_event(window_event::mouse_leaved, window& wnd, mouse_button)
		{
			states_.trasition( state::default_ );
			wnd.redraw();
		}
	};

} // namespace musket	

#endif // MUSKET_INCLUDE_MUSKET_WIDGET_BUTTON_HPP_