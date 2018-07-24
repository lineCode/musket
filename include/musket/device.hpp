//--------------------------------------------------------
// musket/include/musket/device.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_DEVICE_HPP_
#define MUSKET_DEVICE_HPP_

#include <spirea/bit_flags.hpp>
#include <spirea/windows/api.hpp>

namespace musket {

	enum struct mouse_button
	{
		none = 0,
		left = MK_LBUTTON,
		right = MK_RBUTTON,
		middle = MK_MBUTTON,
		xbutton1 = MK_XBUTTON1,
		xbutton2 = MK_XBUTTON2,
	};

} // namespace musket

namespace spirea {

	template <>
	struct enable_bit_flags_operators< musket::mouse_button >
	{ };

} // namespace spirea

#endif // MUSKET_DEVICE_HPP_