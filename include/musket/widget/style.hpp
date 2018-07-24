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
		spirea::dwrite::font_weight weight;
		spirea::dwrite::font_style style;
		spirea::dwrite::font_stretch stretch;
		std::optional< float > size = {};
	};

	inline spirea::dwrite::text_format create_text_format(font_format const& font, float default_size) 
	{
		spirea::dwrite::text_format format;
		spirea::windows::try_hresult( context().dwrite->CreateTextFormat(
			spirea::windows::multibyte_to_widechar( spirea::windows::code_page::utf8, font.name ).c_str(),
			nullptr,
			font.weight, font.style, font.stretch,
			font.size ? *font.size : default_size,
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
	constexpr T optional_brush(T v) noexcept
	{
		using underlying = std::underlying_type_t< T >;
		return static_cast< T >( static_cast< underlying >( v ) | ( 1 << ( sizeof( underlying ) * 8 - 1 ) ) );
	}

	template <typename T>
	constexpr bool is_optional_brush(T v) noexcept
	{
		using underlying = std::underlying_type_t< T >;
		return ( static_cast< underlying >( v ) >> ( sizeof( underlying ) * 8 - 1 ) ) != 0;
	}

	template <typename T, T... Elements>
	class brush_holder
	{
		template <T Elem>
		struct value_holder
		{
			static constexpr T value = Elem;
		};

		std::array< spirea::d2d1::solid_color_brush, sizeof...( Elements ) > brushes_;

	public:
		brush_holder() = default;

		template <typename... Colors>
		brush_holder(spirea::d2d1::render_target const& rt, Colors const&... colors)
		{
			static_assert( sizeof...( Elements ) == sizeof...( Colors ) );
			( ..., assign_brush( value_holder< Elements >{}, rt, colors ) );
		}

		auto operator[](T n) const noexcept
		{
			return brushes_[index_of( n )].get();
		}

	private:
		template <typename Element, typename Color>
		void assign_brush(Element, spirea::d2d1::render_target const& rt, Color const& c)
		{
			if constexpr( is_optional_brush( Element::value ) ) {
				if( c ) {
					auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( *c );
					rt->CreateSolidColorBrush( color, brushes_[index_of( Element::value )].pp() ); 
				}
			}
			else {
				auto const color = rgba_color_traits< spirea::d2d1::color_f >::construct( c );
				rt->CreateSolidColorBrush( color, brushes_[index_of( Element::value )].pp() ); 
			}
		}

		static constexpr std::size_t index_of(T n) noexcept
		{
			using underlying = std::underlying_type_t< T >;
			return static_cast< std::size_t >( static_cast< underlying >( n ) & ~( 1 << ( sizeof( underlying ) * 8 - 1 ) ) );
		}
	};

} // namespace musket

#endif // MUSKET_WIDGET_STYLE_HPP_