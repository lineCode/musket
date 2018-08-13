//--------------------------------------------------------
// musket/include/musket/widget/scroll_bar.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_WIDGET_SCROLL_BAR_HPP_
#define MUSKET_WIDGET_SCROLL_BAR_HPP_

#include "facade.hpp"
#include "../widget.hpp"
#include "button.hpp"

namespace musket {

namespace scroll_bar_event {

	struct scroll
	{
		using type = void (std::uint32_t, std::uint32_t);
	};

} // namespace scroll_bar_event

	using scroll_bar_events = events_holder<
		scroll_bar_event::scroll
	>;

	struct scroll_bar_style
	{
		rgba_color_t bg_color;
		std::optional< edge_property > edge;
		float min_thumb_size;
	};

	struct scroll_bar_thumb_style
	{
		rgba_color_t fg_color;
		std::optional< edge_property > edge;
	};

	enum struct scroll_bar_thumb_state : std::uint8_t
	{
		default_, over, pressed,
	};

	template <axis_flag>
	class scroll_bar;

	template <axis_flag Axis>
	class default_style_t< scroll_bar< Axis > >
	{
		inline static scroll_bar_style style_ = {
			{ 0.4f, 0.4f, 0.4f, 1.0f }, {}, 10.0f
		};
		inline static scroll_bar_thumb_style thumb_default_ = {
			{ 0.65f, 0.65f, 0.65f, 1.0f }, {}
		};
		inline static scroll_bar_thumb_style thumb_over_ = {
			{ 0.75f, 0.75f, 0.75f, 1.0f }, {}
		};
		inline static scroll_bar_thumb_style thumb_pressed_ = {
			{ 0.9f, 0.9f, 0.9f, 1.0f }, {}
		};

		inline static std::mutex mtx_ = {};

	public:
		static void set(scroll_bar_style const& style) noexcept
		{
			std::lock_guard lock{ mtx_ };
			style_ = style;
		}

		static void set(scroll_bar_thumb_state state, scroll_bar_thumb_style const& style) noexcept
		{
			std::lock_guard lock{ mtx_ };
			switch( state ) {
			case scroll_bar_thumb_state::default_:
				thumb_default_ = style;
				break;
			case scroll_bar_thumb_state::over:
				thumb_over_ = style;
				break;
			case scroll_bar_thumb_state::pressed:
				thumb_pressed_ = style;
				break;
			}
		}

		static scroll_bar_style get() noexcept
		{
			std::lock_guard lock{ mtx_ };
			return style_;
		}

		static scroll_bar_thumb_style get(scroll_bar_thumb_state state) noexcept
		{
			std::lock_guard lock{ mtx_ };
			switch( state ) {
			case scroll_bar_thumb_state::default_:
				return thumb_default_;
			case scroll_bar_thumb_state::over:
				return thumb_over_;
			case scroll_bar_thumb_state::pressed:
				return thumb_pressed_;
			}

			return {};
		}
	};	

namespace detail {

	template <typename Parent, axis_flag Direction>
	class scroll_bar_thumb :
		public widget_facade
	{
		using state = scroll_bar_thumb_state;
		using style_data_type = style_data_t< scroll_bar_thumb_style >;
		using state_machine_type = state_machine<
			style_data_type, state, state::default_, state::over, state::pressed
		>;

		Parent* parent_;
		state_machine_type states_;
		spirea::point_t< std::int32_t > prev_pt_;
		spirea::connection conn_sliding_;
		spirea::connection conn_finish_sliding_;
		std::uint32_t pos_ = 0;
		event_handler< scroll_bar_events > handler_;

	public:
		scroll_bar_thumb(
			Parent* parent,
			spirea::rect_t< float > const& rc,
			scroll_bar_thumb_style const& default_style,
			scroll_bar_thumb_style const& over_style,
			scroll_bar_thumb_style const& pressed_style
		) :
			widget_facade{ rc },
			parent_{ parent },
			states_{
				state::default_,
				style_data_type{ default_style, {} }, 
				style_data_type{ over_style, {} }, 
				style_data_type{ pressed_style, {} }
			}
		{ }

		~scroll_bar_thumb() noexcept
		{
			conn_sliding_.disconnect();
		}

		template <typename Event, typename F>
		spirea::connection connect(Event, F&& f)
		{
			return handler_.connect( Event{}, std::forward< F >( f ) );
		}

		std::uint32_t position() const noexcept
		{
			return pos_;
		}

		void on_event(window_event::draw, window& wnd)
		{
			if( !is_visible() ) {
				return;
			}

			auto const rc = spirea::rect_traits< spirea::d2d1::rect_f >::construct( size() );
			auto const rt = wnd.render_target();
			auto const& data = states_.get();

			rt->FillRectangle( rc, data.brush[appear::fg] );
			if( data.style.edge ) {
				rt->DrawRectangle( rc, data.brush[appear::edge], data.style.edge->size );
			}
		}
		
		void on_event(window_event::recreated_target, window& wnd)
		{
			auto create_brushes = [&](style_data_type& sd) {
				sd.brush = { wnd.render_target(), sd.style.fg_color, sd.style.edge };
			};

			for( auto& i : states_.data() ) {
				create_brushes( i );
			}
		}

		void on_event(window_event::mouse_button_pressed, window& wnd, mouse_button btn, mouse_button, spirea::point_t< std::int32_t > const& pt)
		{
			if( spirea::enabled( btn, mouse_button::left ) ) {
				states_.trasition( state::pressed );
				press_left_button( wnd, pt );
			}
		}

		void on_event(window_event::mouse_button_released, window& wnd, mouse_button btn, mouse_button, spirea::point_t< std::int32_t > const&)
		{
			if( spirea::enabled( btn, mouse_button::left ) ) {
				states_.trasition( state::over );
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
			if( !conn_sliding_.is_connected() ) {
				states_.trasition( state::default_ );
			}
			wnd.redraw();
		}

	private:
		void press_left_button(window& wnd, spirea::point_t< std::int32_t > const& pt)
		{
			prev_pt_ = pt;

			conn_sliding_ = wnd.connect( 
				window_event::mouse_moved{}, 
				[this](window& wnd, mouse_button, spirea::point_t< std::int32_t > const& now_pt) mutable {
					auto const rc = size();
					auto const parent_rc = parent_->size();
					if constexpr( Direction == axis_flag::vertical ) {
						auto top = rc.top + ( now_pt.y - prev_pt_.y );
						if( top < parent_rc.top ) {
							top = parent_rc.top;
						} 
						if( top + rc.height() > parent_rc.bottom ) {
							top = parent_rc.bottom - rc.height();
						}
						resize( spirea::rect_t< float >{ { rc.left, top }, rc.area() } );

						auto const prev_pos = pos_;
						pos_ = static_cast< std::uint32_t >( std::floor( top * ( parent_->max_value() - parent_->page_value() ) / ( parent_rc.bottom - rc.height() ) ) );
						if( pos_ != prev_pos ) {
							handler_.invoke( scroll_bar_event::scroll{}, pos_, pos_ + parent_->page_value() );
						}
					}
					else {
						// still not implement horizontal
					}
					wnd.redraw();
					prev_pt_ = now_pt;
				} 
			);

			conn_finish_sliding_ = wnd.connect( 
				window_event::mouse_button_released{}, 
				[this](window& wnd, mouse_button btn, mouse_button, spirea::point_t< std::int32_t > const&) mutable {
					if( spirea::enabled( btn, mouse_button::left ) ) {
						states_.trasition( state::default_ );
						wnd.redraw();

						conn_sliding_.disconnect();
						conn_finish_sliding_.disconnect();
					}
				} 
			);

			wnd.redraw();
		}
	};

} // namespace detail

	template <axis_flag Direction>
	class scroll_bar :
		public widget_facade
	{
		static_assert( Direction == axis_flag::vertical || Direction == axis_flag::horizontal );

		using style_data_type = style_data_t< scroll_bar_style >;

		widget< detail::scroll_bar_thumb< scroll_bar, Direction > > thumb_;
		style_data_type sd_;
		std::uint32_t page_value_;
		std::uint32_t max_value_;

	public:
		template <typename Rect>
		scroll_bar(
			Rect const& rc,
			std::uint32_t page_v,
			std::uint32_t max_v,
			std::optional< scroll_bar_style > style = {},
			std::optional< scroll_bar_thumb_style > default_style = {},
			std::optional< scroll_bar_thumb_style > over_style = {},
			std::optional< scroll_bar_thumb_style > pressed_style = {}
		) :
			widget_facade{ rc },
			sd_{ deref_style< scroll_bar >( style ), {} },
			page_value_{ page_v },
			max_value_{ max_v }
		{
			thumb_ = {
				this,
				spirea::rect_t< float >{ { rc.left, rc.top }, get_thumb_size( sd_.style ) }, 
				deref_style< scroll_bar >( default_style, scroll_bar_thumb_state::default_ ), 
				deref_style< scroll_bar >( over_style, scroll_bar_thumb_state::over ),
				deref_style< scroll_bar >( pressed_style, scroll_bar_thumb_state::pressed ) 
			};
		}

		void show() noexcept
		{
			widget_facade::show();
			thumb_->show();
		}

		void hide() noexcept
		{
			widget_facade::hide();
			thumb_->hide();
		}

		template <typename Rect>
		void resize(Rect const& rc) noexcept
		{
			widget_facade::resize( rc );

			spirea::point_t< float > pt;
			if constexpr( Direction == axis_flag::vertical ) {
				pt.x = rc.left;
				pt.y = rc.height() * static_cast< float >( thumb_->position() ) / static_cast< float >( max_value() - page_value() );
			}
			else {
				pt.x = rc.width() * static_cast< float >( thumb_->position() ) / static_cast< float >( max_value() - page_value() );
				pt.y = rc.top;
			}

			thumb_->resize( spirea::rect_t{ pt, get_thumb_size( sd_.style ) } );
		}

		std::uint32_t page_value() const noexcept
		{
			return page_value_;
		}

		std::uint32_t max_value() const noexcept
		{
			return max_value_;
		}

		std::uint32_t position() const noexcept
		{
			return thumb_->position();
		}

		void set_values(std::uint32_t page_value, std::uint32_t max_value) noexcept
		{
			auto const thumb_rc = thumb_->size();

			page_value_ = page_value;
			max_value_ = max_value;
			
			thumb_->resize( spirea::rect_t{ spirea::point_t{ thumb_rc.left, thumb_rc.top }, get_thumb_size( sd_.style ) } );
		} 

		template <typename Event, typename F>
		spirea::connection connect(Event, F&& f)
		{
			return thumb_->connect( Event{}, std::forward< F >( f ) );
		}

		void on_event(window_event::draw, window& wnd)
		{
			if( !is_visible() ) {
				return;
			}

			auto const rc = spirea::rect_traits< spirea::d2d1::rect_f >::construct( size() );
			auto const rt = wnd.render_target();

			rt->FillRectangle( rc, sd_.brush[appear::bg] );
			if( sd_.style.edge ) {
				rt->DrawRectangle( rc, sd_.brush[appear::edge], sd_.style.edge->size );
			}
		}

		void on_event(window_event::recreated_target, window& wnd)
		{
			sd_.brush = { wnd.render_target(), sd_.style.bg_color, sd_.style.edge };
		}

		void on_event(window_event::attached_widget, window& wnd)
		{
			wnd.attach_widget( thumb_ );
		}

	private:
		spirea::area_t< float > get_thumb_size(scroll_bar_style const& style) const noexcept
		{
			spirea::area_t< float > thumb_sz = size().area();

			if constexpr( Direction == axis_flag::vertical ) {
				thumb_sz.height = thumb_sz.height * page_value() / max_value();
				if( thumb_sz.height <= style.min_thumb_size ) {
					thumb_sz.height = style.min_thumb_size;
				}
			}
			else {
				thumb_sz.width = thumb_sz.width * page_value() / max_value();
				if( thumb_sz.width <= style.min_thumb_size ) {
					thumb_sz.width = style.min_thumb_size;
				}
			}

			return thumb_sz;
		}
	};

	template <axis_flag Direction>
	using auto_scaling_scroll_bar = std::conditional_t<
		Direction == axis_flag::vertical,
		auto_relocator< auto_resizer< scroll_bar< Direction >, axis_flag::vertical >, axis_flag::horizontal >,
		auto_relocator< auto_resizer< scroll_bar< Direction >, axis_flag::horizontal >, axis_flag::vertical >
	>;

} // namespace musket

#endif // MUSKET_WIDGET_SCROLL_BAR_HPP_