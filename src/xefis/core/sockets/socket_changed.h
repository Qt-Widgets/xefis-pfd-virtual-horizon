/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__SOCKET_CHANGED_H__INCLUDED
#define XEFIS__CORE__SOCKETS__SOCKET_CHANGED_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>
#include <xefis/core/sockets/socket.h>


namespace xf {

/**
 * Base for change-observing objects.
 */
class SocketChanged
{
  public:
	// Ctor
	explicit
	SocketChanged (BasicSocket& socket):
		_socket (socket)
	{ }

	// Dtor
	virtual
	~SocketChanged() = default;

	/**
	 * Return true if socket's serial number changed since last cycle.
	 */
	[[nodiscard]]
	bool
	serial_changed (Cycle const& cycle);

	[[nodiscard]]
	BasicSocket&
	socket() noexcept
		{ return _socket; }

	[[nodiscard]]
	BasicSocket const&
	socket() const noexcept
		{ return _socket; }

  protected:
	virtual bool
	perhaps_shift_cycles (Cycle const& cycle);

  private:
	BasicSocket&		_socket;
	BasicSocket::Serial	_prev_serial		{ 0 };
	BasicSocket::Serial	_curr_serial		{ 0 };
	Cycle::Number		_prev_cycle_number	{ 0 };
	Cycle::Number		_curr_cycle_number	{ 0 };
};


/**
 * Checks if socket's value changed since the previous loop cycle.
 */
template<class pValue>
	class SocketValueChanged: public SocketChanged
	{
	  public:
		using Value = pValue;

	  public:
		// Ctor
		explicit
		SocketValueChanged (Socket<Value>& socket):
			SocketChanged (socket),
			_socket (socket)
		{ }

		/**
		 * Return true if socket's value changed since last cycle.
		 */
		[[nodiscard]]
		bool
		value_changed (Cycle const&);

		/**
		 * Return true if socket's value changed to given value since last cycle.
		 */
		[[nodiscard]]
		bool
		value_changed_to (std::optional<Value> const& expected_value, Cycle const& cycle)
			{ return value_changed (cycle) && _socket.get_optional() == expected_value; }

		[[nodiscard]]
		Socket<Value>&
		socket() noexcept
			{ return _socket; }

		[[nodiscard]]
		Socket<Value> const&
		socket() const noexcept
			{ return _socket; }

	  protected:
		bool
		perhaps_shift_cycles (Cycle const& cycle) override;

	  private:
		Socket<Value>&			_socket;
		std::optional<Value>	_prev_value;
		std::optional<Value>	_curr_value;
	};


template<class V>
	inline bool
	SocketValueChanged<V>::value_changed (Cycle const& cycle)
	{
		perhaps_shift_cycles (cycle);
		return _prev_value != _curr_value;
	}


template<class V>
	bool
	SocketValueChanged<V>::perhaps_shift_cycles (Cycle const& cycle)
	{
		auto const shifted = SocketChanged::perhaps_shift_cycles (cycle);

		if (shifted)
		{
			_prev_value = _curr_value;
			_curr_value = _socket.get_optional();
		}

		return shifted;
	}

} // namespace xf

#endif

