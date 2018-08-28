//--------------------------------------------------------
// musket/include/musket/widget/style.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_WIDGET_STYLE_HPP_
#define MUSKET_WIDGET_STYLE_HPP_

#include <cassert>
#include <string>
#include <optional>
#include <mutex>
#include <spirea/windows/d2d1.hpp>
#include <spirea/windows/dwrite.hpp>
#include "../context.hpp"
#include "../color.hpp"

namespace musket {

	struct text_format
	{
		std::string name = "Yu Gothic";
		float size = 15.0f;
		spirea::dwrite::font_weight weight = spirea::dwrite::font_weight::normal;
		spirea::dwrite::font_style style = spirea::dwrite::font_style::normal;
		spirea::dwrite::font_stretch stretch = spirea::dwrite::font_stretch::normal;
	};

	class default_text_format
	{
		inline static text_format format_;
		inline static std::mutex mtx_;

	public:
		static void set(text_format const& format) noexcept
		{
			std::lock_guard lock{ mtx_ };
			format_ = format;
		}

		static text_format get() noexcept
		{
			std::lock_guard lock{ mtx_ };
			return format_;
		}
	};

	inline text_format deref_text_format(std::optional< text_format > tf) noexcept
	{
		return tf ? *tf : default_text_format::get();
	}

	inline spirea::dwrite::text_format create_text_format(text_format const& tf) 
	{
		spirea::dwrite::text_format format;
		spirea::windows::try_hresult( context().dwrite->CreateTextFormat(
			spirea::windows::multibyte_to_widechar( spirea::windows::code_page::utf8, tf.name ).c_str(),
			nullptr,
			tf.weight, tf.style, tf.stretch,
			tf.size,
			context().locale_name.c_str(), format.pp()
		) );

		return format;
	}

	template <typename Rect>
	inline spirea::dwrite::text_layout create_text_layout(spirea::dwrite::text_format const& format, Rect const& rc, std::string_view str)
	{
		spirea::dwrite::text_layout layout;
		std::wstring const wstr = spirea::windows::multibyte_to_widechar( spirea::windows::code_page::utf8, str );
		spirea::windows::try_hresult( context().dwrite->CreateTextLayout(
			wstr.c_str(), wstr.size(), format.get(), 
			spirea::rect_traits< Rect >::width( rc ), 
			spirea::rect_traits< Rect >::height( rc ), 
			layout.pp()
		) );

		return layout;
	}

	struct edge_property
	{
		rgba_color_t color;
		float size = 1.0f;
	};

	template <typename Widget>
	class default_style_t;

	template <typename Widget, typename Style, typename... Args>
	inline Style deref_style(std::optional< Style > const& style, Args&&... args) noexcept
	{
		return style ? *style : default_style_t< Widget >::get( std::forward< Args >( args )... );
	}

namespace style_detail {

	enum struct location : std::uint8_t
	{
		none, bg, fg, edge, text,
	};

	template <typename StyleType, typename = void>
	struct has_fg_color
	{
		static constexpr auto value = location::none;
	};

	template <typename StyleType>
	struct has_fg_color< StyleType, std::void_t< decltype( std::declval< StyleType >().fg_color ) > >
	{
		static constexpr auto value = location::fg;
	};

	template <typename StyleType, typename = void>
	struct has_bg_color
	{
		static constexpr auto value = location::none;
	};

	template <typename StyleType>
	struct has_bg_color< StyleType, std::void_t< decltype( std::declval< StyleType >().bg_color ) > >
	{
		static constexpr auto value = location::bg;
	};

	template <typename StyleType, typename = void>
	struct has_edge
	{
		static constexpr auto value = location::none;
	};

	template <typename StyleType>
	struct has_edge< StyleType, std::void_t< decltype( std::declval< StyleType >().edge ) > >
	{
		static constexpr auto value = location::edge;
	};

	template <typename StyleType, typename = void>
	struct has_text_color
	{
		static constexpr auto value = location::none;
	};

	template <typename StyleType>
	struct has_text_color< StyleType, std::void_t< decltype( std::declval< StyleType >().text_color ) > >
	{
		static constexpr auto value = location::text;
	};

	template <location...>
	struct location_holder
	{ };

	template <typename StyleType>
	using has_location_holder_t = location_holder<
		has_fg_color< StyleType >::value,
		has_bg_color< StyleType >::value,
		has_edge< StyleType >::value,
		has_text_color< StyleType >::value
	>;

	template <typename Result, typename Src, typename = void>
	struct make_location_holder_impl;

	template <typename Result>
	struct make_location_holder_impl< Result, location_holder<>, void >
	{
		using type = Result;
	};

	template <typename Result, location... Rest>
	struct make_location_holder_impl< Result, location_holder< location::none, Rest... >, void >
	{
		using type = typename make_location_holder_impl<
			Result, location_holder< Rest... >
		>::type;
	};

	template <location... Locs, location Loc, location... Rest>
	struct make_location_holder_impl< 
		location_holder< Locs... >, 
		location_holder< Loc, Rest... >, 
		std::enable_if_t< Loc != location::none >
	>
	{
		using type = typename make_location_holder_impl<
			location_holder< Locs..., Loc >,
			location_holder< Rest... >
		>::type;
	};

	template <typename StyleType>
	using make_location_holder = typename make_location_holder_impl<
		location_holder<>,
		has_location_holder_t< StyleType >
	>::type;

	template <location Loc>
	class style_adapter;

	template <>
	class style_adapter< location::fg >
	{
		spirea::d2d1::solid_color_brush brush_;

	protected:
		template <typename StyleType>
		void recreated_target(spirea::d2d1::render_target const& rt, StyleType const& style)
		{
			if( !style.fg_color ) {
				return;
			}
			auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( *style.fg_color );
			rt->CreateSolidColorBrush( color, brush_.pp() );
		}

	public:
		void draw_foreground(spirea::d2d1::render_target const& rt, spirea::d2d1::rect_f const& rc) const
		{
			if( !brush_ ) {
				return;
			}
			rt->FillRectangle( rc, brush_.get() );
		}
	};

	template <>
	class style_adapter< location::bg >
	{
		spirea::d2d1::solid_color_brush brush_;

	protected:
		template <typename StyleType>
		void recreated_target(spirea::d2d1::render_target const& rt, StyleType const& style)
		{
			if( !style.bg_color ) {
				return;
			}
			auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( *style.bg_color );
			rt->CreateSolidColorBrush( color, brush_.pp() );
		}

	public:
		void draw_background(spirea::d2d1::render_target const& rt, spirea::d2d1::rect_f const& rc) const
		{
			if( !brush_ ) {
				return;
			}
			rt->FillRectangle( rc, brush_.get() );
		}
	};

	template <>
	class style_adapter< location::edge >
	{
		spirea::d2d1::solid_color_brush brush_;
		float sz_;

	protected:
		template <typename StyleType>
		void recreated_target(spirea::d2d1::render_target const& rt, StyleType const& style)
		{
			if( !style.edge ) {
				return;
			}
			auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( style.edge->color );
			rt->CreateSolidColorBrush( color, brush_.pp() );
			sz_ = style.edge->size;
		}

	public:
		void draw_edge(spirea::d2d1::render_target const& rt, spirea::d2d1::rect_f const& rc) const
		{
			if( !brush_ ) {
				return;
			}
			rt->DrawRectangle( rc, brush_.get(), sz_ );
		}
	};

	template <>
	class style_adapter< location::text >
	{
		spirea::d2d1::solid_color_brush brush_;

	protected:
		template <typename StyleType>
		void recreated_target(spirea::d2d1::render_target const& rt, StyleType const& style)
		{
			if( !style.text_color ) {
				return;
			}
			auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( *style.text_color );
			rt->CreateSolidColorBrush( color, brush_.pp() );
		}

	public:
		void draw_text(spirea::d2d1::render_target const& rt, spirea::d2d1::point_2f const& pt, spirea::dwrite::text_layout const& layout) const
		{
			if( !brush_ ) {
				return;
			}
			rt->DrawTextLayout( pt, layout.get(), brush_.get(), spirea::d2d1::draw_text_options::clip );
		}
	};

} // namespace style_detail

	template <typename StyleType, typename = style_detail::make_location_holder< StyleType >>
	class style_data_t;

	template <typename StyleType, style_detail::location... Locs>
	class style_data_t< StyleType, style_detail::location_holder< Locs... > > :
		public style_detail::style_adapter< Locs >...
	{
		StyleType style_;

	public:
		style_data_t() = default;

		style_data_t(StyleType const& style) :
			style_{ style }
		{ }

		StyleType const& get_style() const noexcept
		{
			return style_;
		}

		void recreated_target(spirea::d2d1::render_target const& rt)
		{
			( ..., style_detail::style_adapter< Locs >::recreated_target( rt, style_ ) );
		}
	};

} // namespace musket

#endif // MUSKET_WIDGET_STYLE_HPP_