//--------------------------------------------------------
// musket/include/musket/geometry.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_GEOMETRY_HPP_
#define MUSKET_GEOMETRY_HPP_

#include <spirea/geometry/point.hpp>
#include <spirea/geometry/area.hpp>
#include <spirea/geometry/rect.hpp>
#include <spirea/bit_flags.hpp>

namespace musket {

	enum struct axis_flag : std::uint8_t
	{
		vertical = 1,
		horizontal = 1 << 1,
		all = vertical | horizontal,
	};

} // namespace musket

namespace spirea {

	template <>
	struct enable_bit_flags_operators< musket::axis_flag >
	{ };

} // namespace spirea

#endif // MUSKET_GEOMETRY_HPP_