//--------------------------------------------------------
// musket/include/musket/widget/attributes.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_WIDGET_ATTRIBUTES_HPP_
#define MUSKET_WIDGET_ATTRIBUTES_HPP_

#include "../window.hpp"
#include "../event.hpp"

namespace musket {

namespace event {

	struct auto_resize
	{
		using type = void (window&, spirea::rect_t< float > const&);
	};

	struct auto_relocation
	{
		using type = void (window&, spirea::point_t< float > const&);
	};

} // namespace event

	template <typename Widget, axis_flag AxisFlag = axis_flag::all>
	class auto_resizer :
		public Widget
	{
	public:
		using Widget::Widget;
		using Widget::on_event;

		void on_event(event::detail::auto_resize, window& wnd, spirea::point_t< float > const& offset)
		{
			auto rc = this->size();

			if constexpr( spirea::enabled( AxisFlag, axis_flag::horizontal ) ) {
				rc.right += offset.x;
				if( rc.right < rc.left ) {
					rc.right = rc.left;
				}
			}
			if constexpr( spirea::enabled( AxisFlag, axis_flag::vertical ) ) {
				rc.bottom += offset.y;
				if( rc.bottom < rc.top ) {
					rc.bottom = rc.top;
				}
			}

			if constexpr( has_on_event< decltype( this ), event::auto_resize >::value ) {
				this->on_event( event::auto_resize{}, wnd, rc );
			}

			this->resize( rc );
		}
	};

	template <typename Widget, axis_flag AxisFlag>
	class auto_relocator :
		public Widget
	{
	public:
		using Widget::Widget;
		using Widget::on_event;

		void on_event(event::detail::auto_relocation, window& wnd, spirea::point_t< float > const& offset)
		{
			auto rc = this->size();
			auto const width = rc.width();
			auto const height = rc.height();

			if constexpr( spirea::enabled( AxisFlag, axis_flag::horizontal ) ) {
				rc.left += offset.x;
			}
			if constexpr( spirea::enabled( AxisFlag, axis_flag::vertical ) ) {
				rc.top += offset.y;
			}
			rc.right = rc.left + width;
			rc.bottom = rc.top + height;

			if constexpr( has_on_event< decltype( this ), event::auto_relocation >::value ) {
				this->on_event( event::auto_relocation{}, wnd, rc );
			}

			this->resize( rc );
		}
	};

} // namespace musket

#endif // MUSKET_WIDGET_ATTRIBUTES_HPP_