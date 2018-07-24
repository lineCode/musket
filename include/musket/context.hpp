//--------------------------------------------------------
// musket/include/musket/context.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_CONTEXT_HPP_
#define MUSKET_CONTEXT_HPP_

#include <memory>
#include <vector>
#include <mutex>
#include <spirea/windows/api.hpp>
#include <spirea/windows/d2d1.hpp>
#include <spirea/windows/dwrite.hpp>
#include <spirea/windows/undef.hpp>

namespace musket {

namespace detail {

	struct context_impl
	{
		std::wstring locale_name;
		spirea::d2d1::factory3 d2d1;
		spirea::dwrite::factory5 dwrite;

		static context_impl const& instance()
		{
			static std::unique_ptr< context_impl > obj{ new context_impl };
			return *obj;
		}
		
	private:
		context_impl() :
			locale_name{
				spirea::windows::api::get_user_default_locale_name()
			},
			d2d1{ spirea::try_result( 
				spirea::d2d1::create_factory< decltype( d2d1 ) >( spirea::d2d1::factory_type::multi_threaded )
			) },
			dwrite{ spirea::try_result(
				spirea::dwrite::create_factory< decltype( dwrite ) >( spirea::dwrite::factory_type::shared )
			) }
		{ }
	};

} // namespace detail

	inline detail::context_impl const& context()
	{
		return detail::context_impl::instance();
	}	

} // namespace musket

#endif // MUSKET_CONTEXT_HPP_