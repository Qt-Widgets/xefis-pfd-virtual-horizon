/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__V2__INSTRUMENT_H__INCLUDED
#define XEFIS__CORE__V2__INSTRUMENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <atomic>
#include <string>
#include <type_traits>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/utility/noncopyable.h>


namespace xf {

class BasicInstrument: public BasicModule
{
  public:
	using BasicModule::BasicModule;

	// Dtor
	virtual ~BasicInstrument() = default;

	/**
	 * Paint the instrument onto given canvas.
	 */
	virtual void
	paint (QImage& canvas) const = 0;

	/**
	 * Return true if instrument wants to be repainted.
	 * Also unmark the instrument as dirty atomically.
	 */
	bool
	dirty_since_last_check() noexcept;

	/**
	 * Mark instrument as dirty (to be repainted).
	 */
	void
	mark_dirty() noexcept;

  private:
	std::atomic<bool>	_dirty	{ false };
};


template<class IO = ModuleIO>
	class Instrument: public BasicInstrument
	{
	  public:
		/**
		 * Ctor
		 * Version for modules that do have their own IO class.
		 */
		template<class = std::enable_if_t<!std::is_same_v<IO, ModuleIO>>>
			explicit
			Instrument (std::unique_ptr<IO> io, std::string const& instance = {});

		/**
		 * Ctor
		 * Version for modules that do not have any IO class.
		 */
		template<class = std::enable_if_t<std::is_same_v<IO, ModuleIO>>>
			explicit
			Instrument (std::string const& instance = {});

	  protected:
		IO& io;
	};


inline bool
BasicInstrument::dirty_since_last_check() noexcept
{
	return _dirty.exchange (false);
}


inline void
BasicInstrument::mark_dirty() noexcept
{
	_dirty.store (true);
}


template<class IO>
	template<class>
		inline
		Instrument<IO>::Instrument (std::unique_ptr<IO> module_io, std::string const& instance):
			BasicInstrument (std::move (module_io), instance),
			io (static_cast<IO&> (*io_base()))
		{ }


template<class IO>
	template<class>
		inline
		Instrument<IO>::Instrument (std::string const& instance):
			BasicInstrument (std::make_unique<IO>(), instance),
			io (static_cast<IO&> (*io_base()))
		{ }

} // namespace xf

#endif

