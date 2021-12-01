/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_DELTA_DECODER_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_DELTA_DECODER_H__INCLUDED

// Standard:
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/utility/actions.h>


namespace xf {

template<class T>
	concept DeltaDecoderValueConcept = std::signed_integral<T>;


template<DeltaDecoderValueConcept pInteger = int64_t>
	class SocketDeltaDecoder
	{
	  public:
		using Integer	= pInteger;
		using Callback	= std::function<void (std::optional<Integer> delta)>;

	  public:
		// Ctor
		explicit
		SocketDeltaDecoder (Socket<Integer> const& socket, Callback callback, Integer initial_value = {});

		/**
		 * Signals that sockets have been updated. May call the callback.
		 */
		void
		operator()();

		/**
		 * Force callback to be called with given delta value, but don't change internal state of the decoder.
		 */
		void
		force_callback (std::optional<Integer> delta) const;

	  private:
		Integer					_previous;
		Socket<Integer> const&	_value_socket;
		PropChanged<Integer>	_socket_changed	{ _value_socket };
		Callback				_callback;
	};


template<DeltaDecoderValueConcept I>
	inline
	SocketDeltaDecoder<I>::SocketDeltaDecoder (Socket<Integer> const& socket, Callback callback, Integer initial_value):
		_previous (initial_value),
		_value_socket (socket),
		_callback (callback)
	{ }


template<DeltaDecoderValueConcept I>
	inline void
	SocketDeltaDecoder<I>::operator()()
	{
		if (_callback && _socket_changed())
		{
			if (_value_socket)
			{
				auto const current = *_value_socket;
				_callback (current - _previous);
				_previous = current;
			}
			else
				_callback (std::nullopt);
		}
	}


template<DeltaDecoderValueConcept I>
	inline void
	SocketDeltaDecoder<I>::force_callback (std::optional<Integer> delta) const
	{
		_callback (delta);
	}

} // namespace xf

#endif

