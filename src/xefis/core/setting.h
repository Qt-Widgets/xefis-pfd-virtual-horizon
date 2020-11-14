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

#ifndef XEFIS__CORE__SETTING_H__INCLUDED
#define XEFIS__CORE__SETTING_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>

// Neutrino:
#include <neutrino/time.h>
#include <neutrino/time_helper.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/module_socket_path.h>
#include <xefis/utility/blob.h>


namespace xf {

class ModuleIO;


/**
 * Common base class for all Setting<*> types.
 */
class BasicSetting
{
  public:
	/**
	 * Tag for creating a setting that doesn't not need to be set by user explicitly, but doesn't necessarily
	 * have any default value.
	 */
	enum OptionalTag { Optional };

  public:
	/**
	 * Ctor
	 * Stores pointer to owning module.
	 */
	explicit
	BasicSetting (ModuleIO* owner, std::string const& name);

	/**
	 * Ctor
	 * Stores pointer to owning module.
	 */
	explicit
	BasicSetting (ModuleIO* owner, std::string const& name, OptionalTag);

	// Dtor
	virtual
	~BasicSetting() = default;

	/**
	 * Return owning module.
	 */
	ModuleIO*
	io() const noexcept;

	/**
	 * Return true if setting has a value.
	 */
	virtual
	operator bool() const noexcept = 0;

	/**
	 * Return setting name.
	 */
	std::string const&
	name() const noexcept;

	/**
	 * Return true if setting is required to have a value.
	 */
	bool
	required() const noexcept;

  private:
	ModuleIO*	_owner;
	std::string	_name;
	bool		_required;
};


/**
 * Wrapper for setting variables.
 */
template<class pValue>
	class Setting: public BasicSetting
	{
	  public:
		typedef pValue Value;

		class Uninitialized: public xf::FastException
		{
			using FastException::FastException;
		};

	  public:
		/**
		 * Create a setting object that requires explicit setting of a value.
		 */
		explicit
		Setting (ModuleIO* owner, std::string const& name);

		/**
		 * Creates a setting object that has an initial value.
		 */
		explicit
		Setting (ModuleIO* owner, std::string const& name, Value const& initial_value);

		/**
		 * Creates a setting that doesn't have and doesn't require any value.
		 */
		explicit
		Setting (ModuleIO* owner, std::string const& name, OptionalTag);

		/**
		 * Copy-assignment operator.
		 */
		Setting const&
		operator= (Value const& new_value);

		/**
		 * Read the setting value.
		 */
		Value&
		operator*();

		/**
		 * Read the setting value.
		 */
		Value const&
		operator*() const;

		/**
		 * Read the setting value.
		 */
		Value const*
		operator->() const;

		// BasicSetting API
		operator bool() const noexcept override;

	  private:
		std::optional<Value>	_value;
	};


inline
BasicSetting::BasicSetting (ModuleIO* owner, std::string const& name):
	_owner (owner),
	_name (name),
	_required (true)
{ }


inline
BasicSetting::BasicSetting (ModuleIO* owner, std::string const& name, OptionalTag):
	_owner (owner),
	_name (name),
	_required (false)
{ }


inline ModuleIO*
BasicSetting::io() const noexcept
{
	return _owner;
}


inline std::string const&
BasicSetting::name() const noexcept
{
	return _name;
}


inline bool
BasicSetting::required() const noexcept
{
	return _required;
}


template<class V>
	inline
	Setting<V>::Setting (ModuleIO* owner, std::string const& name):
		BasicSetting (owner, name)
	{
		ModuleIO::ProcessingLoopAPI (*owner).register_setting (*this);
	}


template<class V>
	inline
	Setting<V>::Setting (ModuleIO* owner, std::string const& name, Value const& initial_value):
		BasicSetting (owner, name),
		_value (initial_value)
	{
		ModuleIO::ProcessingLoopAPI (*owner).register_setting (*this);
	}


template<class V>
	inline
	Setting<V>::Setting (ModuleIO* owner, std::string const& name, OptionalTag optional_tag):
		BasicSetting (owner, name, optional_tag)
	{
		ModuleIO::ProcessingLoopAPI (*owner).register_setting (*this);
	}


template<class V>
	inline Setting<V> const&
	Setting<V>::operator= (Value const& value)
	{
		_value = value;
		return *this;
	}


template<class V>
	inline auto
	Setting<V>::operator*() -> Value&
	{
		if (!_value)
			throw Uninitialized ("reading uninitialized setting");

		return *_value;
	}


template<class V>
	inline auto
	Setting<V>::operator*() const -> Value const&
	{
		if (!_value)
			throw Uninitialized ("reading uninitialized setting");

		return *_value;
	}


template<class V>
	inline auto
	Setting<V>::operator->() const -> Value const*
	{
		if (!_value)
			throw Uninitialized ("reading uninitialized setting");

		return &_value.value();
	}


template<class V>
	inline
	Setting<V>::operator bool() const noexcept
	{
		return !!_value;
	}

} // namespace xf

#endif

