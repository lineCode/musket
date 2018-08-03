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
		std::optional< edge_property > edge;
		rgba_color_t text_color;
	};

	class label :
		public widget_facade
	{
		std::string str_;
		spirea::rect_t< float > rc_;
		spirea::dwrite::text_format format_;
		spirea::dwrite::text_layout text_;
		style_data_t< label_style > data_;

	public:
		template <typename Rect>
		label(
			Rect const& rc,
			std::string_view str,
			font_format const& font,
			label_style const& style
		) :
			widget_facade{ rc },
			str_{ str.begin(), str.end() },
			rc_{ spirea::rect_traits< decltype( rc_ ) >::construct( rc ) },
			data_{ style, {} }
		{
			format_ = create_text_format( font, ( rc_.bottom - rc_.top ) * 0.7f );
			format_->SetTextAlignment( font.align );
			format_->SetParagraphAlignment( spirea::dwrite::paragraph_alignment::far );
			text_ = create_text_layout( format_, rc_, str );
		}

		void set_text(std::string_view str)
		{
			text_ = create_text_layout( format_, rc_, str );
		}

		void on_event(window_event::draw, window& wnd)
		{
			if( !is_visible() ) {
				return;
			}

			auto const rc = spirea::rect_traits< spirea::d2d1::rect_f >::construct( size() );
			auto const rt = wnd.render_target();

			if( data_.style.bg_color ) {
				rt->FillRectangle( rc, data_.brush[appear::bg] );
			}
			if( data_.style.edge ) {
				rt->DrawRectangle( rc, data_.brush[appear::edge], data_.style.edge->size );
			}

			rt->DrawTextLayout( { rc.left, rc.top }, text_.get(), data_.brush[appear::text], spirea::d2d1::draw_text_options::clip );
		}

		void on_event(window_event::recreated_target, window& wnd)
		{
			data_.brush = { wnd.render_target(), data_.style.bg_color, data_.style.edge, data_.style.text_color };
		}
	};

} // namespace musket

#endif // MUSKET_WIDGET_LABEL_HPP_