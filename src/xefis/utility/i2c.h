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

#ifndef XEFIS__UTILITY__I2C_H__INCLUDED
#define XEFIS__UTILITY__I2C_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>


namespace I2C {

enum Operation {
	Write,	// Message will be sent to slave
	Read	// Message will be read from slave
};


class Address
{
  public:
	/**
	 * Create I2C address.
	 * \param	address 7- or 10-bit I2C address.
	 * \param	ten_bit Tells whether address is 10-bit.
	 */
	explicit
	Address (uint16_t address, bool ten_bit = false) noexcept;

	/**
	 * Return address.
	 */
	uint16_t
	address() const noexcept;

	/**
	 * Return true if address is 10-bit.
	 */
	bool
	is_ten_bit() const noexcept;

  private:
	uint16_t	_address;
	bool		_ten_bit;
};


class Message
{
  public:
	/**
	 * Create I2C Message.
	 * Data is represented by range [begin, end). Message does not
	 * make a copy of the data.
	 */
	Message (Operation, Address const&, uint8_t* begin, uint8_t* end);

	/**
	 * Create I2C Message.
	 * Data is represented by data structure. Type size is determined
	 * automatically.
	 */
	template<class DataType>
		Message (Operation, Address const&, DataType* data_type);

	/**
	 * Generate struct for use with Linux I2C API.
	 */
	struct ::i2c_msg
	generate_i2c_msg() const noexcept;

  private:
	Operation	_operation;
	Address		_address;
	uint8_t*	_begin;
	uint8_t*	_end;
};


typedef std::vector<Message> Transaction;


class Bus: public Noncopyable
{
  public:
	/**
	 * Open Linux I2C device.
	 * \param	bus_number I2C bus number.
	 */
	explicit
	Bus (uint8_t bus_number) noexcept;

	// Dtor
	~Bus();

	/**
	 * Return bus number.
	 */
	uint8_t
	bus_number() const noexcept;

	/**
	 * Execute I2C transaction.
	 */
	void
	execute (Transaction const&);

  private:
	uint8_t		_bus_number;
	int			_device;
};


inline
Address::Address (uint16_t address, bool ten_bit) noexcept:
	_address (address),
	_ten_bit (ten_bit)
{ }


inline uint16_t
Address::address() const noexcept
{
	return _address;
}


inline bool
Address::is_ten_bit() const noexcept
{
	return _ten_bit;
}


template<class DataType>
	inline
	Message::Message (Operation operation, Address const& address, DataType* data_type):
		_operation (operation),
		_address (address),
		_begin (reinterpret_cast<uint8_t*> (data_type)),
		_end (_begin + sizeof (DataType))
	{ }


inline uint8_t
Bus::bus_number() const noexcept
{
	return _bus_number;
}

} // namespace I2C

#endif

