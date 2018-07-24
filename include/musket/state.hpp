//--------------------------------------------------------
// musket/include/musket/state.hpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#ifndef MUSKET_STATE_HPP_
#define MUSKET_STATE_HPP_

#include <array>
#include <type_traits>

namespace musket {

	template <typename T, typename StateType, StateType... States>
	class state_machine
	{
		static_assert( std::is_enum_v< StateType > );

		StateType state_;
		std::array< T, sizeof...( States ) > data_;

	public:
		state_machine() = default;

		template <typename... Data>
		state_machine(StateType init, Data&&... data) :
			state_{ init },
			data_{ std::forward< Data >( data )... }
		{ }

		void trasition(StateType s) noexcept
		{
			state_ = s;
		}

		StateType state() const noexcept
		{
			return state_;
		}

		T const& get() const noexcept
		{
			return data_[static_cast< std::underlying_type_t< StateType > >( state_ )];
		}

		auto& data() noexcept
		{
			return data_;
		}

		auto const& data() const noexcept
		{
			return data_;
		}
	};

} // namespace musket

#endif // MUSKET_STATE_HPP_