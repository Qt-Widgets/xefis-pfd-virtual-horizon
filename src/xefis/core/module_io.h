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

#ifndef XEFIS__CORE__MODULE_IO_H__INCLUDED
#define XEFIS__CORE__MODULE_IO_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/sequence.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>


class QWidget;

namespace xf {

class BasicModule;
class BasicSetting;
class BasicModuleIn;
class BasicModuleOut;


namespace module_io {

/**
 * Exception thrown when some settings in a module have not been initialized as required.
 */
class UninitializedSettings: public FastException
{
  public:
	// Ctor
	explicit
	UninitializedSettings (std::vector<BasicSetting*>);

  private:
	/**
	 * Create a message for the exception.
	 */
	std::string
	make_message (std::vector<BasicSetting*>);
};


/**
 * Exception thrown when there's general logic error in configuration.
 */
class InvalidConfig: public Exception
{
  public:
	// Ctor
	explicit
	InvalidConfig (std::string const& message):
		Exception ("logic error in ModuleIO configuration: " + message)
	{ }
};


/**
 * Exception thrown when trying to use ModuleIO::module(), but no Module has been
 * associated with the ModuleIO object.
 */
class ModuleNotAssigned: public Exception
{
  public:
	// Ctor
	explicit
	ModuleNotAssigned():
		Exception ("ModuleIO doesn't have assigned Module object")
	{ };
};

} // namespace module_io


class ModuleIO
{
	friend class BasicModule;

  public:
	/**
	 * A set of methods for the processing loop to use on the module.
	 */
	class ProcessingLoopAPI
	{
	  public:
		// Ctor
		explicit
		ProcessingLoopAPI (ModuleIO&);

		// Dtor
		virtual
		~ProcessingLoopAPI() = default;

		/**
		 * Set reference to the module object.
		 */
		void
		set_module (BasicModule&);

		/**
		 * Iterate through registered settings and check that ones without default value have been initialized by user.
		 * If uninitialized settings are found, UninitializedSettings is thrown.
		 * Also call virtual ModuleIO::verify_settings().
		 */
		virtual void
		verify_settings();

		/**
		 * Register setting.
		 */
		void
		register_setting (BasicSetting&);

		/**
		 * Register an input socket with this module.
		 */
		void
		register_input_socket (BasicModuleIn&);

		/**
		 * Unregister an input socket.
		 */
		void
		unregister_input_socket (BasicModuleIn&);

		/**
		 * Register an output socket with this module.
		 */
		void
		register_output_socket (BasicModuleOut&);

		/**
		 * Unregister an output socket.
		 */
		void
		unregister_output_socket (BasicModuleOut&);

		/**
		 * Return registered settings.
		 */
		Sequence<std::vector<BasicSetting*>::const_iterator>
		settings() const noexcept;

		/**
		 * Return registered input sockets.
		 */
		Sequence<std::vector<BasicModuleIn*>::const_iterator>
		input_sockets() const noexcept;

		/**
		 * Return registered output sockets.
		 */
		Sequence<std::vector<BasicModuleOut*>::const_iterator>
		output_sockets() const noexcept;

	  private:
		ModuleIO& _io;
	};

  public:
	// Dtor
	virtual
	~ModuleIO();

	/**
	 * Return reference to the module that uses this ModuleIO object.
	 */
	BasicModule&
	module() const;

	/**
	 * User-provided settings verification procedure.
	 */
	virtual void
	verify_settings()
	{ }

  private:
	BasicModule*					_module = nullptr;
	std::vector<BasicSetting*>		_registered_settings;
	std::vector<BasicModuleIn*>		_registered_input_sockets;
	std::vector<BasicModuleOut*>	_registered_output_sockets;
};


inline
ModuleIO::ProcessingLoopAPI::ProcessingLoopAPI (ModuleIO& io):
	_io (io)
{ }


inline void
ModuleIO::ProcessingLoopAPI::set_module (BasicModule& module)
{
	_io._module = &module;
}


inline BasicModule&
ModuleIO::module() const
{
	if (!_module)
		throw module_io::ModuleNotAssigned();

	return *_module;
}


/*
 * Global functions
 */


/**
 * Return string identifying module and its instance, if any module is associated with the ModuleIO object.
 */
std::string
identifier (ModuleIO&);


/**
 * Same as identifier (ModuleIO&).
 */
std::string
identifier (ModuleIO*);

} // namespace xf

#endif

