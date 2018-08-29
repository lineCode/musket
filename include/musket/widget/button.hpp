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
#include "attributes.hpp"

namespace musket {

	class button;

	struct button_style
	{
		std::optional< rgba_color_t > bg_color;
		std::optional< edge_property > edge;
		std::optional< rgba_color_t > text_color;
	};

	enum struct button_state : std::uint8_t
	{
		idle, over, pressed,
	};

	template <>
	class default_style_t< button >
	{
		inline static button_style idle = {
			rgba_color_t{ 0.25f, 0.25f, 0.25f, 1.0f },
			musket::edge_property{ { 0.5f, 0.5f, 0.5f, 1.0f }, 1.0f },
			rgba_color_t{ 1.0f, 1.0f, 1.0f, 1.0f },
		};

		inline static button_style over_ = {
			rgba_color_t{ 0.4f, 0.4f, 0.4f, 1.0f },
			musket::edge_property{ { 0.5f, 0.5f, 0.5f, 1.0f }, 1.0f },
			rgba_color_t{ 1.0f, 1.0f, 1.0f, 1.0f },
		};

		inline static button_style pressed_ = {
			rgba_color_t{ 0.6f, 0.6f, 0.6f, 1.0f },
			musket::edge_property{ { 1.0f, 1.0f, 0.0f, 1.0f }, 1.0f },
			rgba_color_t{ 1.0f, 1.0f, 1.0f, 1.0f },
		};

		inline static std::mutex mtx_;

	public:
		static void set(button_state state, button_style const& style) noexcept
		{
			std::lock_guard lock{ mtx_ };

			switch( state ) {
			case button_state::idle:
				idle = style;
				break;
			case button_state::over:
				over_ = style;
				break;
			case button_state::pressed:
				pressed_ = style;
				break;
			}
		}

		static button_style get(button_state state) noexcept
		{
			std::lock_guard lock{ mtx_ };

			switch( state ) {
			case button_state::idle:
				return idle;
			case button_state::over:
				return over_;
			case button_state::pressed:
				return pressed_;
			}

			return {};
		}
	};

	struct button_property
	{
		std::optional< text_format > text_fmt = {};
		std::optional< button_style > idle_style = {};
		std::optional< button_style > over_style = {};
		std::optional< button_style > pressed_style = {};
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
		using style_data_type = style_data_t< button_style >;

		using state_machine_type = state_machine< 
			style_data_type, button_state, 
			button_state::idle, button_state::over, button_state::pressed 
		>;

		std::string str_;
		state_machine_type states_;
		spirea::dwrite::text_layout text_;
		event_handler< button_events > event_handler_;

	public:
		template <typename Rect>
		button(
			Rect const& rc,
			std::string_view str, 
			button_property const& prop = {}
		) :
			widget_facade{ rc },
			str_{ str.begin(), str.end() },
			states_{ 
				button_state::idle, 
				style_data_type{ 
					deref_style< button >( prop.idle_style, button_state::idle )
				}, 
				style_data_type{ 
					deref_style< button >( prop.over_style, button_state::over ) 
				}, 
				style_data_type{ 
					deref_style< button >( prop.pressed_style, button_state::pressed )
				} 
			}
		{
			spirea::dwrite::text_format format = create_text_format( deref_text_format( prop.text_fmt ) );
			format->SetTextAlignment( spirea::dwrite::text_alignment::center );
			format->SetParagraphAlignment( spirea::dwrite::paragraph_alignment::center );
			text_ = create_text_layout( format, this->size(), str );
		}

		template <typename Event, typename F>
		spirea::connection connect(Event, F&& f)
		{
			return event_handler_.connect( Event{}, std::forward< F >( f ) );
		}

		void on_event(window_event::draw, window& wnd)
		{
			if( !this->is_visible() ) {
				return;
			}

			auto const rc = spirea::rect_traits< spirea::d2d1::rect_f >::construct( this->size() );
			auto const rt = wnd.render_target();
			auto const& data = states_.get();

			data.draw_background( rt, rc );
			data.draw_edge( rt, rc );
			data.draw_text( rt, { rc.left, rc.top }, text_ );
		}

		void on_event(window_event::recreated_target, window& wnd)
		{
			auto const rt = wnd.render_target();
			for( auto& i : states_.data() ) {
				i.recreated_target( rt );
			}
		}

		void on_event(window_event::mouse_button_pressed, window& wnd, mouse_button btn, mouse_button, spirea::point_t< std::int32_t > const& pt)
		{
			if( spirea::enabled( btn, mouse_button::left ) ) {
				states_.trasition( button_state::pressed );
				event_handler_.invoke( button_event::pressed{}, pt );
				wnd.redraw();
			}
		}

		void on_event(window_event::mouse_button_released, window& wnd, mouse_button btn, mouse_button, spirea::point_t< std::int32_t > const& pt)
		{
			if( spirea::enabled( btn, mouse_button::left ) ) {
				states_.trasition( button_state::idle );
				event_handler_.invoke( button_event::released{}, pt );
				wnd.redraw();
			}
		}

		void on_event(window_event::mouse_entered, window& wnd, mouse_button btns)
		{
			if( spirea::enabled( btns, mouse_button::left ) ) {
				states_.trasition( button_state::pressed );
			}
			else {
				states_.trasition( button_state::over );
			}
			wnd.redraw();
		}

		void on_event(window_event::mouse_leaved, window& wnd, mouse_button)
		{
			states_.trasition( button_state::idle );
			wnd.redraw();
		}

		void on_event(event::auto_resize, window& wnd, spirea::rect_t< float > const& rc)
		{
			text_->SetMaxWidth( rc.width() );
			text_->SetMaxHeight( rc.height() );
		}
	};

} // namespace musket	

#endif // MUSKET_INCLUDE_MUSKET_WIDGET_BUTTON_HPP_