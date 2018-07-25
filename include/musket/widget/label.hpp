//--------------------------------------------------------
// musket/include/musket/widget/label.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_WIDGET_LABEL_HPP_
#define MUSKET_WIDGET_LABEL_HPP_

#include "facade.hpp"

namespace musket {

	struct label_style
	{
		std::optional< rgba_color_t > bg_color;
		std::optional< rgba_color_t > edge_color;
		rgba_color_t text_color;
		float edge_size = 1.0f;
	};

	class label :
		public widget_facade
	{
		enum struct color : std::uint8_t
		{
			bg, edge, text,
		};

		struct style_data
		{
			label_style style;
			brush_holder< color, optional_brush( color::bg ), optional_brush( color::edge ), color::text > brush;
		};

		std::string str_;
		spirea::rect_t< float > rc_;
		spirea::dwrite::text_layout text_;
		style_data data_;

		bool visibility_ = true;

	public:
		template <typename Rect>
		label(
			std::string_view str,
			Rect const& rc,
			font_format const& font,
			label_style const& style
		) :
			widget_facade{ rc },
			str_{ str.begin(), str.end() },
			rc_{ spirea::rect_traits< decltype( rc_ ) >::construct( rc ) },
			data_{ style, {} }
		{
			auto format = create_text_format( font, ( rc_.bottom - rc_.top ) * 0.7f );
			text_ = create_text_layout( format, rc_, str );
			text_->SetTextAlignment( font.align );
			text_->SetParagraphAlignment( spirea::dwrite::paragraph_alignment::far );
		}

		void on_event(window_event::draw, window& wnd)
		{
			if( !is_visible() ) {
				return;
			}

			auto const rc = spirea::rect_traits< spirea::d2d1::rect_f >::construct( size() );
			auto const rt = wnd.render_target();

			if( data_.style.bg_color ) {
				rt->FillRectangle( rc, data_.brush[color::bg] );
			}
			if( data_.style.edge_color ) {
				rt->DrawRectangle( rc, data_.brush[color::edge], data_.style.edge_size );
			}

			rt->DrawTextLayout( { rc.left, rc.top }, text_.get(), data_.brush[color::text], spirea::d2d1::draw_text_options::clip );
		}

		void on_event(window_event::recreated_target, window& wnd)
		{
			data_.brush = { wnd.render_target(), data_.style.bg_color, data_.style.edge_color, data_.style.text_color };
		}
	};

} // namespace musket

#endif // MUSKET_WIDGET_LABEL_HPP_