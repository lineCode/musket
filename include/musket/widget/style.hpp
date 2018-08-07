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
#include <spirea/windows/d2d1.hpp>
#include <spirea/windows/dwrite.hpp>
#include "../context.hpp"
#include "../color.hpp"

namespace musket {

	struct font_format
	{
		std::string name;
		float size;
		spirea::dwrite::font_weight weight;
		spirea::dwrite::font_style style;
		spirea::dwrite::font_stretch stretch;
		spirea::dwrite::text_alignment align;
		spirea::dwrite::paragraph_alignment paragraph;
	};

	inline spirea::dwrite::text_format create_text_format(font_format const& font) 
	{
		spirea::dwrite::text_format format;
		spirea::windows::try_hresult( context().dwrite->CreateTextFormat(
			spirea::windows::multibyte_to_widechar( spirea::windows::code_page::utf8, font.name ).c_str(),
			nullptr,
			font.weight, font.style, font.stretch,
			font.size,
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

	template <typename T>
	struct is_optional :
		public std::false_type
	{ };

	template <typename T>
	struct is_optional< std::optional< T > > :
		public std::true_type
	{ };

	struct edge_property
	{
		rgba_color_t color;
		float size = 1.0f;
	};

	enum struct appearance : std::uint8_t
	{
		fg, bg, edge, text,
	};

	template <appearance... Elements>
	class brush_holder
	{
		std::array< spirea::d2d1::solid_color_brush, sizeof...( Elements ) > brushes_;

	public:
		brush_holder() = default;

		template <typename... Colors>
		brush_holder(spirea::d2d1::render_target const& rt, Colors const&... colors)
		{
			static_assert( sizeof...( Elements ) == sizeof...( Colors ) );
			( ..., assign_brush( Elements, rt, colors ) );
		}

		auto operator[](appearance n) const noexcept
		{
			auto const i = index_of( n );
			assert( i < sizeof...( Elements ) );
			return brushes_[i].get();
		}

	private:
		template <typename Color>
		void assign_brush(appearance n, spirea::d2d1::render_target const& rt, Color const& c)
		{
			if constexpr( std::is_same_v< Color, edge_property > ) {
				auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( c.color );
				rt->CreateSolidColorBrush( color, brushes_[index_of( n )].pp() ); 
			}
			else if constexpr( std::is_same_v< Color, std::optional< edge_property > > ) {
				if( c ) {
					auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( c->color );
					rt->CreateSolidColorBrush( color, brushes_[index_of( n )].pp() ); 
				}
			}
			else if constexpr( is_optional< Color >::value ) {
				if( c ) {
					auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( *c );
					rt->CreateSolidColorBrush( color, brushes_[index_of( n )].pp() ); 
				}
			}
			else {
				auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( c );
				rt->CreateSolidColorBrush( color, brushes_[index_of( n )].pp() ); 
			}
		}

		static constexpr std::size_t index_of(appearance n) noexcept
		{
			std::size_t i = 0;
			( ( ( n != Elements ) && ++i ) && ... );
			return i;
		}
	};

namespace appearance_detail {

	template <typename T, typename Member>
	constexpr appearance make_appearance(T v, Member) noexcept
	{
		return static_cast< appearance >( v );
	}

	constexpr auto invalid_appearance = static_cast< appearance >( std::numeric_limits< std::underlying_type_t< appearance > >::max() );

	template <typename StyleType, typename = void>
	struct has_fg_color
	{ 
		static constexpr auto value = invalid_appearance;
	};
	
	template <typename StyleType>
	struct has_fg_color< StyleType, std::void_t< decltype( std::declval< StyleType >().fg_color ) > >
	{ 
		static constexpr auto value = make_appearance( appearance::fg, decltype( std::declval< StyleType >().fg_color ){} );
	};

	template <typename StyleType, typename = void>
	struct has_bg_color
	{ 
		static constexpr auto value = invalid_appearance;
	};
	
	template <typename StyleType>
	struct has_bg_color< StyleType, std::void_t< decltype( std::declval< StyleType >().bg_color ) > >
	{ 
		static constexpr auto value = make_appearance( appearance::bg, decltype( std::declval< StyleType >().bg_color ){} );
	};

	template <typename StyleType, typename = void>
	struct has_edge
	{ 
		static constexpr auto value = invalid_appearance;
	};
	
	template <typename StyleType>
	struct has_edge< StyleType, std::void_t< decltype( std::declval< StyleType >().edge ) > >
	{ 
		static constexpr auto value = make_appearance( appearance::edge, decltype( std::declval< StyleType >().edge ){} );
	};

	template <typename StyleType, typename = void>
	struct has_text_color
	{ 
		static constexpr auto value = invalid_appearance;
	};
	
	template <typename StyleType>
	struct has_text_color< StyleType, std::void_t< decltype( std::declval< StyleType >().text_color ) > >
	{ 
		static constexpr auto value = make_appearance( appearance::text, decltype( std::declval< StyleType >().text_color ){} );
	};
	
	template <typename...>
	struct has_appearance_holder
	{ };

	template <typename StyleType>
	using has_appearance_holder_t = has_appearance_holder<
		has_fg_color< StyleType >,
		has_bg_color< StyleType >,
		has_edge< StyleType >,
		has_text_color< StyleType >
	>;

	template <appearance...>
	struct appearance_holder
	{ };

	template <typename StyleType, typename Init, typename Elems, typename = void>
	struct make_brush_holder_impl;

	template <typename StyleType, typename Result>
	struct make_brush_holder_impl< StyleType, Result, appearance_holder<>, void >
	{
		using type = Result;
	};

	template <typename StyleType, typename Result, appearance Val, appearance... Rest>
	struct make_brush_holder_impl< StyleType, Result, appearance_holder< Val, Rest... >, std::enable_if_t< Val == invalid_appearance > >
	{
		using type = typename make_brush_holder_impl< StyleType, Result, appearance_holder< Rest... > >::type;
	};

	template <typename StyleType, appearance... V, appearance Val, appearance... Rest>
	struct make_brush_holder_impl< StyleType, brush_holder< V... >, appearance_holder< Val, Rest... >, std::enable_if_t< Val != invalid_appearance > >
	{
		using type = typename make_brush_holder_impl< StyleType, brush_holder< V..., Val >, appearance_holder< Rest... > >::type;
	};
	
} // namespace appearance_detail

	template <typename StyleType, typename = appearance_detail::has_appearance_holder_t< StyleType >>
	struct make_brush_holder;

	template <typename StyleType, typename... Elems>
	struct make_brush_holder< StyleType, appearance_detail::has_appearance_holder< Elems... > >
	{
		using type = typename appearance_detail::make_brush_holder_impl<
			StyleType, brush_holder<>, appearance_detail::appearance_holder< Elems::value... >
		>::type;
	};

	template <typename StyleType>
	struct style_data_t
	{
		StyleType style;
		typename make_brush_holder< StyleType >::type brush;
	};

	using appear = appearance;

} // namespace musket

#endif // MUSKET_WIDGET_STYLE_HPP_