/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__PROPERTY_H__INCLUDED
#define XEFIS__CORE__PROPERTY_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <string>
#include <limits>

// Boost:
#include <boost/optional.hpp>
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_node.h"
#include "property_storage.h"


namespace Xefis {

/**
 * Indicates that property tried to be read
 * could not be found in the node tree.
 */
class PropertyNotFound: public Exception
{
  public:
	explicit PropertyNotFound (std::string const& message);
};


/**
 * Indicates that the property is singular (not attached to any tree)
 * and can't be written or read.
 */
class SingularProperty: public Exception
{
  public:
	explicit SingularProperty (std::string const& message);
};


/**
 * Indicates that the operation is invalid on certain
 * node type.
 */
class InvalidOperation: public Exception
{
  public:
	explicit InvalidOperation (std::string const& message);
};


/**
 * Indicates that there's type conflict between existing property
 * and property requested to be created.
 */
class TypeConflict: public Exception
{
  public:
	explicit TypeConflict (std::string const& path);
};


/**
 * Indicates that given string is not a valid supported type.
 */
class BadType: public Exception
{
  public:
	explicit BadType (std::string const& name);
};


/**
 * Indicates that there was an error during stringify operation.
 */
class StringifyError: public Exception
{
  public:
	explicit StringifyError (std::string const& message);
};


class GenericProperty
{
  public:
	// Ctor
	GenericProperty();

	// Ctor
	GenericProperty (GenericProperty const&) = default;

	// Assignment
	GenericProperty&
	operator= (GenericProperty const&) = default;

	// Ctor
	GenericProperty (std::string path);

	// Ctor
	GenericProperty (PropertyDirectoryNode* root, std::string path);

	// Dtor
	virtual ~GenericProperty();

	/**
	 * Return true if property is nil.
	 */
	bool
	is_nil() const;

	/**
	 * Set property to the nil value.
	 */
	void
	set_nil();

	/**
	 * Return true if the property is singular,
	 * that is uninitialized.
	 */
	bool
	is_singular() const noexcept;

	/**
	 * Configured means not singular and having path != "".
	 */
	bool
	configured() const noexcept;

	/**
	 * Valid means not singular and not nil.
	 */
	bool
	valid() const noexcept;

	/**
	 * Return property path.
	 */
	std::string const&
	path() const noexcept;

	/**
	 * Point this property to another PropertyNode.
	 */
	void
	set_path (std::string const&);

	/**
	 * Return the serial value of the property.
	 */
	PropertyNode::Serial
	serial() const;

	/**
	 * Return true if the PropertyNode value has changed
	 * since last read().
	 */
	bool
	fresh() const;

	/**
	 * Convenience method that checks for valid()ity and fresh()ness.
	 */
	bool
	valid_and_fresh() const;

	/**
	 * Check whether the node this property points to,
	 * is a PropertyValueNode of given Target type.
	 */
	template<class Target>
		bool
		is_type() const;

	/**
	 * Return proper node. If cached node's path
	 * matches this property's path, return it.
	 * Otherwise, locate it.
	 */
	PropertyNode*
	get_node() const;

	/**
	 * Return humanized value (eg. value with unit).
	 */
	virtual std::string
	stringify() const;

	/**
	 * Return humanized value (using boost::format and specified unit).
	 */
	virtual std::string
	stringify (boost::format, std::string const& unit, std::string nil_value) const;

	/**
	 * Return binary representation of the value.
	 */
	virtual Blob
	binarify() const;

	/**
	 * Return float-like value of the property.
	 */
	virtual double
	floatize (std::string unit) const;

  protected:
	/**
	 * Reset flag that the property is fresh().
	 */
	void
	unfresh() const;

	/**
	 * Normalize path so if there's "//" in it,
	 * it will be replaced by leading "/".
	 */
	static std::string
	normalized_path (std::string path);

  protected:
	PropertyDirectoryNode*			_root = nullptr;
	mutable PropertyNode*			_node = nullptr;
	std::string						_path;
	mutable PropertyNode::Serial	_last_read_serial = 0;
};


/**
 * Common base for typed properties, that is not templateized
 * yet. One can use methods like stringify(), parse() or floatize()
 * to get value from the node.
 */
class TypedProperty: public GenericProperty
{
  public:
	TypedProperty() = default;
	TypedProperty (TypedProperty const&) = default;
	TypedProperty& operator= (TypedProperty const&) = default;

	// Inherit other constructors:
	using GenericProperty::GenericProperty;

  public:
	/**
	 * Ensures that this property exists.
	 * Does nothing in this class.
	 */
	virtual void
	ensure_existence();

	/**
	 * Set value from humanized string (eg. "10 kt").
	 * This version (not overridden) doesn't create a nil node,
	 * when it can't find one, but quietly does nothing.
	 */
	virtual void
	parse (std::string const&);

	/**
	 * Set value from binary blob.
	 * This version (not overridden) doesn't create a nil node,
	 * when it can't find one, but quietly does nothing.
	 */
	virtual void
	parse_blob (Blob const&);

	/**
	 * Create new property node of given type.
	 * \throw	TypeConflict
	 * 			when there's another property under the same path with different type.
	 * \throw	BadType
	 * 			when type doesn't name correct type.
	 */
	static void
	create (std::string const& path, std::string const& type);
};


/**
 * A property reference. Doesn't hold the data, but only the path,
 * and queries property storage whenever needed.
 */
template<class tType>
	class Property: public TypedProperty
	{
	  public:
		typedef tType					Type;
		typedef PropertyValueNode<Type>	ValueNodeType;

	  public:
		/**
		 * Create a property with singular path.
		 */
		Property();

		/**
		 * Copy from other property.
		 */
		Property (Property const& other);

		/**
		 * Create a Property that belongs to Xefis::PropertyStorage::root()
		 * (the default storage) and is bound to given path.
		 * NOTE: The PropertyStorage must be initialized before attempting
		 * to use this constructor.
		 */
		Property (std::string const& path);

		/**
		 * Create a Property that belongs to a PropertyStorage,
		 * bound to given path.
		 */
		Property (PropertyDirectoryNode* root, std::string const& path);

		/**
		 * Copy from other property.
		 */
		Property&
		operator= (Property const& other);

		/**
		 * Get Optional value. Takes 'nil' value into account.
		 */
		Optional<Type>
		get_optional() const;

		/**
		 * Read property. If node can't be found, return default value.
		 */
		Type
		read (Type default_value = Type()) const;

		/**
		 * Read property. If node can't be found, throw PropertyNotFound.
		 */
		Type
		read_signalling() const;

		/**
		 * Unary * operator - same as read().
		 */
		Type
		operator*() const;

		/**
		 * Pointer operator for accessing members of the stored value.
		 * \throw	SingularProperty if property is singular.
		 * \throw	NilNode if node is a nil-node.
		 */
		Type const*
		operator->() const;

		/**
		 * Overload for write (Type&);
		 * \return	*this
		 */
		Property<Type>&
		operator= (Type const&);

		/**
		 * Overload for write (Optional<Type>&);
		 * \return	*this
		 */
		Property<Type>&
		operator= (Optional<Type> const&);

		/**
		 * Write to the property.
		 * If node can't be found, create it.
		 * If not possible, throw PropertyPathConflict.
		 */
		void
		write (Type const&);

		/**
		 * Write to the property.
		 * If Optional doesn't hold a value, set the property to nil.
		 */
		void
		write (Optional<Type> const&);

		/**
		 * Write to the property.
		 * If node can't be found, throw PropertyNotFound.
		 */
		void
		write_signalling (Type const&);

		/**
		 * Write to the property.
		 * If Optional doesn't hold a value, set the property to nil.
		 * If node can't be found, throw PropertyNotFound.
		 */
		void
		write_signalling (Optional<Type> const&);

		/**
		 * Sets value (like write) if property is not singular
		 * and if it's nil. Otherwise it's a no-op.
		 */
		void
		set_default (Type const&);

		/**
		 * Copy other node. Call PropertyNode::copy().
		 */
		void
		copy (Property const& other);

		// TypedProperty API
		void
		ensure_existence() override;

		// TypedProperty API
		void
		parse (std::string const&) override;

		// TypedProperty API
		void
		parse_blob (Blob const&) override;

		/**
		 * Return node casted to PropertyValueNode.
		 * If unable to cast, throw TypeConflict.
		 * If property node doesn't exist, return nullptr.
		 */
		ValueNodeType*
		get_value_node() const;

		/**
		 * Return node casted to PropertyValueNode.
		 * If unable to cast, throw TypeConflict.
		 * If property node doesn't exist, throw PropertyNotFound.
		 */
		ValueNodeType*
		get_value_node_signalling() const;

	  private:
		/**
		 * Ensure that the property exists in the tree.
		 */
		template<class V>
			ValueNodeType*
			ensure_path (std::string const& path, V value);
	};


inline
PropertyNotFound::PropertyNotFound (std::string const& message):
	Exception (message)
{ }


inline
SingularProperty::SingularProperty (std::string const& message):
	Exception (message)
{ }


inline
InvalidOperation::InvalidOperation (std::string const& message):
	Exception (message)
{ }


inline
TypeConflict::TypeConflict (std::string const& path):
	Exception ("property under path '" + path + "' already exists and has different type")
{ }


inline
BadType::BadType (std::string const& name):
	Exception ("'" + name + "' is not valid type name")
{ }


inline
StringifyError::StringifyError (std::string const& message):
	Exception (message)
{ }


inline
GenericProperty::GenericProperty():
	_root (PropertyStorage::default_storage()->root())
{ }


inline
GenericProperty::GenericProperty (std::string path):
	_root (PropertyStorage::default_storage()->root()),
	_path (normalized_path (path))
{ }


inline
GenericProperty::GenericProperty (PropertyDirectoryNode* root, std::string path):
	_root (root),
	_path (normalized_path (path))
{ }


inline
GenericProperty::~GenericProperty()
{ }


inline bool
GenericProperty::is_nil() const
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
		{
			TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
			if (val_node)
				return val_node->is_nil();
			else
				throw InvalidOperation ("can't check if directory node is nil: " + _path);
		}
		else
			return true;
	}
	else
		throw SingularProperty ("can't read from a singular property: " + _path);
}


inline void
GenericProperty::set_nil()
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
		{
			TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
			if (val_node)
				val_node->set_nil();
			else
				throw InvalidOperation ("can't set directory node to nil: " + _path);
		}
	}
	else
		throw SingularProperty ("can't write to a singular property: " + _path);
}


inline bool
GenericProperty::is_singular() const noexcept
{
	return !_root;
}


inline bool
GenericProperty::configured() const noexcept
{
	return !is_singular() && !path().empty();
}


inline bool
GenericProperty::valid() const noexcept
{
	return !is_singular() && !is_nil();
}


inline std::string const&
GenericProperty::path() const noexcept
{
	return _path;
}


inline void
GenericProperty::set_path (std::string const& new_path)
{
	_path = normalized_path (new_path);
	// The node will be localized again, when it's needed:
	_node = nullptr;
}


inline PropertyNode::Serial
GenericProperty::serial() const
{
	PropertyNode* node = get_node();
	if (node)
		return node->serial();
	// If node becomes unavailable (not valid, singular or so),
	// return -1u.
	return std::numeric_limits<PropertyNode::Serial>::max();
}


inline bool
GenericProperty::fresh() const
{
	return serial() + 1 > _last_read_serial;
}


inline bool
GenericProperty::valid_and_fresh() const
{
	return valid() && fresh();
}


template<class Target>
	inline bool
	GenericProperty::is_type() const
	{
		PropertyNode const* node = get_node();
		if (!node)
			return false;
		return !!dynamic_cast<PropertyValueNode<Target> const*> (node);
	}


inline PropertyNode*
GenericProperty::get_node() const
{
	if (_root && !_path.empty())
	{
		if (_node && _node->path() == _path)
			return _node;

		// Recache:
		return _node = _root->locate (_path);
	}
	else
		return nullptr;
}


inline std::string
GenericProperty::stringify() const
{
	TypedPropertyValueNode* node = dynamic_cast<TypedPropertyValueNode*> (get_node());
	if (node)
		return node->stringify();
	return "";
}


inline std::string
GenericProperty::stringify (boost::format format, std::string const& unit, std::string nil_value) const
{
	try {
		if (is_nil())
			return nil_value;
		else if (is_type<std::string>())
			return (format % stringify()).str();
		else if (is_type<bool>())
			return (stringify() == "true") ? "ON" : "OFF";
		else
			return (format % floatize (unit)).str();
	}
	catch (UnsupportedUnit const&)
	{
		throw StringifyError ("unit error");
	}
	catch (boost::io::too_few_args const&)
	{
		throw StringifyError ("format: too few args");
	}
	catch (boost::io::too_many_args const&)
	{
		throw StringifyError ("format: too many args");
	}
	catch (...)
	{
		throw StringifyError ("format error");
	}
}


inline Blob
GenericProperty::binarify() const
{
	TypedPropertyValueNode* node = dynamic_cast<TypedPropertyValueNode*> (get_node());
	if (node)
		return node->binarify();
	return Blob();
}


inline double
GenericProperty::floatize (std::string unit) const
{
	TypedPropertyValueNode* node = dynamic_cast<TypedPropertyValueNode*> (get_node());
	if (node)
		return node->floatize (unit);
	return 0.0;
}


inline void
GenericProperty::unfresh() const
{
	_last_read_serial = serial() + 1;
}


inline std::string
GenericProperty::normalized_path (std::string path)
{
	std::size_t p = path.rfind ("//");
	if (p != std::string::npos)
		return path.substr (p + 1);
	return path;
}


inline void
TypedProperty::ensure_existence()
{
	// Pass
}


inline void
TypedProperty::parse (std::string const& str_value)
{
	if (_root)
	{
		if (!_path.empty())
		{
			PropertyNode* node = get_node();
			if (node)
			{
				TypedPropertyValueNode* typed_node = dynamic_cast<TypedPropertyValueNode*> (node);
				if (typed_node)
					typed_node->parse (str_value);
			}
		}
	}
	else
		throw SingularProperty ("can't write to a singular property: " + _path);
}


inline void
TypedProperty::parse_blob (Blob const& value)
{
	if (_root)
	{
		if (!_path.empty())
		{
			PropertyNode* node = get_node();
			if (node)
			{
				TypedPropertyValueNode* typed_node = dynamic_cast<TypedPropertyValueNode*> (node);
				if (typed_node)
					typed_node->parse_blob (value);
			}
		}
	}
	else
		throw SingularProperty ("can't write to a singular property: " + _path);
}


template<class T>
	inline
	Property<T>::Property():
		Property ("")
	{ }


template<class T>
	inline
	Property<T>::Property (Property const& other):
		TypedProperty (other)
	{ }


template<class T>
	inline
	Property<T>::Property (std::string const& path):
		Property (PropertyStorage::default_storage()->root(), path)
	{
		if (!_root)
			throw SingularProperty ("PropertyStorage is not initialized, can't construct Property with default storage: " + _path);
	}


template<class T>
	inline
	Property<T>::Property (PropertyDirectoryNode* node, std::string const& path):
		TypedProperty (node->root(), path)
	{ }


template<class T>
	inline Property<T>&
	Property<T>::operator= (Property const& other)
	{
		TypedProperty::operator= (other);
		return *this;
	}


template<class T>
	inline void
	Property<T>::ensure_existence()
	{
		if (is_nil())
		{
			write (Type());
			set_nil();
		}
	}


template<class T>
	inline Optional<T>
	Property<T>::get_optional() const
	{
		if (is_nil())
			return Optional<T>();
		return Optional<T> (**this);
	}


template<class T>
	inline typename Property<T>::Type
	Property<T>::read (Type default_value) const
	{
		if (_root)
		{
			ValueNodeType* node = get_value_node();
			if (!node)
				return default_value;
			else
			{
				unfresh();
				return node->read (default_value);
			}
		}
		return default_value;
	}


template<class T>
	inline typename Property<T>::Type
	Property<T>::read_signalling() const
	{
		if (_root)
			return get_value_node_signalling()->read();
		else
			throw SingularProperty ("can't read from a singular property: " + _path);
	}


template<class T>
	inline typename Property<T>::Type
	Property<T>::operator*() const
	{
		return read();
	}


template<class T>
	inline typename Property<T>::Type const*
	Property<T>::operator->() const
	{
		if (_root)
			return &get_value_node_signalling()->read();
		else
			throw SingularProperty ("can't read from a singular property: " + _path);
	}


template<class T>
	inline Property<T>&
	Property<T>::operator= (Type const& value)
	{
		write (value);
		return *this;
	}


template<class T>
	inline Property<T>&
	Property<T>::operator= (Optional<Type> const& value)
	{
		write (value);
		return *this;
	}


template<class T>
	inline void
	Property<T>::write (Type const& value)
	{
		if (_root)
		{
			if (!_path.empty())
			{
				try {
					get_value_node_signalling()->write (value);
				}
				catch (PropertyNotFound)
				{
					ensure_path (_path, value);
				}
			}
		}
		else
			throw SingularProperty ("can't write to a singular property: " + _path);
	}


template<class T>
	inline void
	Property<T>::write (Optional<Type> const& value)
	{
		if (value)
			write (*value);
		else
			set_nil();
	}


template<class T>
	inline void
	Property<T>::write_signalling (Type const& value)
	{
		if (_root)
			get_value_node_signalling()->write (value);
		else
			throw SingularProperty ("can't write to a singular property: " + _path);
	}


template<class T>
	inline void
	Property<T>::write_signalling (Optional<Type> const& value)
	{
		if (value)
			write_signalling (*value);
		else
			set_nil();
	}


template<class T>
	inline void
	Property<T>::set_default (Type const& value)
	{
		if (!is_singular() && is_nil())
			write (value);
	}


template<class T>
	inline void
	Property<T>::copy (Property<T> const& other)
	{
		write (other.read());
		if (other.is_nil())
			set_nil();
	}


template<class T>
	inline void
	Property<T>::parse (std::string const& value)
	{
		if (_root)
		{
			if (!_path.empty())
			{
				try {
					get_value_node_signalling()->parse (value);
				}
				catch (PropertyNotFound)
				{
					ValueNodeType* val_node = ensure_path (_path, Type());
					val_node->parse (value);
				}
			}
		}
		else
			throw SingularProperty ("can't write to a singular property: " + _path);
	}


template<class T>
	inline void
	Property<T>::parse_blob (Blob const& value)
	{
		if (_root)
		{
			if (!_path.empty())
			{
				try {
					get_value_node_signalling()->parse_blob (value);
				}
				catch (PropertyNotFound)
				{
					ValueNodeType* val_node = ensure_path (_path, Type());
					val_node->parse_blob (value);
				}
			}
		}
		else
			throw SingularProperty ("can't write to a singular property: " + _path);
	}


template<class T>
	inline typename Property<T>::ValueNodeType*
	Property<T>::get_value_node() const
	{
		PropertyNode* node = get_node();
		if (node)
		{
			ValueNodeType* val_node = dynamic_cast<ValueNodeType*> (node);
			if (val_node)
				return val_node;
			else
				throw TypeConflict ("incompatible type: " + _path);
		}
		else
			return nullptr;
	}


template<class T>
	inline typename Property<T>::ValueNodeType*
	Property<T>::get_value_node_signalling() const
	{
		ValueNodeType* val_node = get_value_node();
		if (val_node)
			return val_node;
		else
			throw PropertyNotFound ("could not find property by path: " + _path);
	}


template<class T>
	template<class V>
		inline typename Property<T>::ValueNodeType*
		Property<T>::ensure_path (std::string const& path, V value)
		{
			std::string::size_type s = path.find_last_of ('/');
			std::string dir = path.substr (0, s);
			std::string pro = path.substr (s + 1);

			PropertyDirectoryNode* parent = _root;
			if (s != std::string::npos)
				parent = _root->mkpath (dir);
			ValueNodeType* child = new ValueNodeType (pro, value);
			parent->add_child (child);
			return child;
		}


/*
 * Shortcut types
 */


typedef Property<bool>			PropertyBoolean;
typedef Property<int64_t>		PropertyInteger;
typedef Property<double>		PropertyFloat;
typedef Property<std::string>	PropertyString;
typedef Property<Acceleration>	PropertyAcceleration;
typedef Property<Angle>			PropertyAngle;
typedef Property<Capacity>		PropertyCapacity;
typedef Property<Current>		PropertyCurrent;
typedef Property<Pressure>		PropertyPressure;
typedef Property<Frequency>		PropertyFrequency;
typedef Property<Length>		PropertyLength;
typedef Property<Speed>			PropertySpeed;
typedef Property<Temperature>	PropertyTemperature;
typedef Property<Time>			PropertyTime;
typedef Property<Weight>		PropertyWeight;

} // namespace Xefis

#endif

