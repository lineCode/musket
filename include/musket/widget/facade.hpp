//--------------------------------------------------------
// musket/include/musket/widget/facade.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_WIDGET_FACADE_HPP_
#define MUSKET_WIDGET_FACADE_HPP_

#include "../context.hpp"
#include "../event.hpp"
#include "../color.hpp"
#include "../state.hpp"
#include "../window.hpp"
#include "style.hpp"
#include <spirea/windows/undef.hpp>

namespace musket {

	class widget_facade
	{
		spirea::rect_t< float > rc_;
		bool visibility_;

	public:
		template <typename Rect>
		widget_facade(Rect const& rc, bool visibility = true) :
			rc_{ spirea::rect_traits< decltype( rc_ ) >::construct( rc ) },
			visibility_{ visibility }
		{ }

		spirea::rect_t< float > size() const noexcept
		{
			return rc_;
		}

		template <typename Rect>
		void resize(Rect const& rc) const noexcept
		{
			rc_ = spirea::rect_traits< decltype( rc_ ) >::construct( rc );
		}

		bool is_visible() const noexcept
		{
			return visibility_;
		}

		void show() noexcept
		{
			visibility_ = true;
		}

		void hide() noexcept
		{
			visibility_ = false;
		}
	};

} // namespace musket

#endif // MUSKET_WIDGET_FACADE_HPP_