//--------------------------------------------------------
// musket/include/musket/utility.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_UTILITY_HPP_
#define MUSKET_UTILITY_HPP_

namespace musket {

	template <typename Widget>
	inline void switch_visibility(Widget& w) noexcept
	{
		if( w->is_visible() ) {
			w->hide();
		}
		else {
			w->show();
		}
	}

} // namespace musket

#endif // MUSKET_UTILITY_HPP_