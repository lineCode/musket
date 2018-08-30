//--------------------------------------------------------
// musket/include/musket//color.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_COLOR_HPP_
#define MUSKET_COLOR_HPP_

#include <spirea/mp/container.hpp>
#include <spirea/windows/d2d1.hpp>

namespace musket {

	template <typename T>
	struct rgba_color_traits_impl;

namespace rgba_color_traits_detail {

	template <typename T, typename = void>
	struct has_construct_function :
		std::false_type
	{ };

	template <typename T>
	struct has_construct_function<
		T, std::void_t< decltype( rgba_color_traits_impl< T >::construct(
			std::declval< typename T::value_type >(),
			std::declval< typename T::value_type >(),
			std::declval< typename T::value_type >(),
			std::declval< typename T::value_type >()
		) ) > 
	> :
		std::true_type
	{ };

	template <typename T, typename = void>
	struct has_red_function :
		std::false_type
	{ };

	template <typename T>
	struct has_red_function< T, std::void_t< decltype( rgba_color_traits_impl< T >::red( std::declval< T& >() ) ) > > :
		std::true_type
	{ };

	template <typename T, typename = void>
	struct has_green_function :
		std::false_type
	{ };

	template <typename T>
	struct has_green_function< T, std::void_t< decltype( rgba_color_traits_impl< T >::green( std::declval< T& >() ) ) > > :
		std::true_type
	{ };

	template <typename T, typename = void>
	struct has_blue_function :
		std::false_type
	{ };

	template <typename T>
	struct has_blue_function< T, std::void_t< decltype( rgba_color_traits_impl< T >::blue( std::declval< T& >() ) ) > > :
		std::true_type
	{ };

	template <typename T, typename = void>
	struct has_alpha_function :
		std::false_type
	{ };

	template <typename T>
	struct has_alpha_function< T, std::void_t< decltype( rgba_color_traits_impl< T >::alpha( std::declval< T& >() ) ) > > :
		std::true_type
	{ };

} // namespace rgba_color_traits_detail

	template <typename T>
	struct rgba_color_traits
	{
		using type = std::remove_cv_t< std::remove_reference_t< T > >;
		using value_type = typename rgba_color_traits_impl< type >::value_type;

		static type construct(value_type r, value_type g, value_type b, value_type a) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_construct_function< T >::value ) {
				return rgba_color_traits_impl< type >::construct( r, g, b, a );
			}
			else {
				return type{ r, g, b, a };
			}
		}

		template <typename U>
		static type construct(U const& c) noexcept
		{
			return construct( 
				static_cast< value_type >( rgba_color_traits< U >::red( c ) ),
				static_cast< value_type >( rgba_color_traits< U >::green( c ) ),
				static_cast< value_type >( rgba_color_traits< U >::blue( c ) ),
				static_cast< value_type >( rgba_color_traits< U >::alpha( c ) )
			);
		}

		static value_type& red(type& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_red_function< T >::value ) {
				return rgba_color_traits_impl< type >::red( c );
			}
			else {
				return c.r;
			}
		}

		static value_type const& red(type const& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_red_function< T >::value ) {
				return rgba_color_traits_impl< type >::red( c );
			}
			else {
				return c.r;
			}
		}

		static value_type& green(type& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_green_function< T >::value ) {
				return rgba_color_traits_impl< type >::green( c );
			}
			else {
				return c.g;
			}
		}

		static value_type const& green(type const& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_green_function< T >::value ) {
				return rgba_color_traits_impl< type >::green( c );
			}
			else {
				return c.g;
			}
		}

		static value_type& blue(type& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_blue_function< T >::value ) {
				return rgba_color_traits_impl< type >::blue( c );
			}
			else {
				return c.b;
			}
		}

		static value_type const& blue(type const& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_blue_function< T >::value ) {
				return rgba_color_traits_impl< type >::blue( c );
			}
			else {
				return c.b;
			}
		}

		static value_type& alpha(type& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_alpha_function< T >::value ) {
				return rgba_color_traits_impl< type >::alpha( c );
			}
			else {
				return c.a;
			}
		}

		static value_type const& alpha(type const& c) noexcept
		{
			if constexpr( rgba_color_traits_detail::has_blue_function< T >::value ) {
				return rgba_color_traits_impl< type >::alpha( c );
			}
			else {
				return c.a;
			}
		}
	};

	template <>
	struct rgba_color_traits_impl< spirea::d2d1::color_f >
	{
		using value_type = float;
	};

	using rgba_color_t = spirea::d2d1::color_f;

} // namespace musket

#endif // MUSKET_COLOR_HPP_