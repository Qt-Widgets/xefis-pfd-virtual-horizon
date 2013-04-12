/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__PROPERTY_NODE_H__INCLUDED
#define XEFIS__CORE__PROPERTY_NODE_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <stdint.h>

// Boost:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>

#include "property_storage.h"


namespace Xefis {

// Forward declarations:
class PropertyDirectoryNode;
class PropertyStorage;
template<class T>
	class PropertyValueNode;

typedef std::list<PropertyNode*> PropertyNodeList;


/**
 * Indicates invalid operation on node of a specific type,
 * for example attempt to read int value of a directory-type
 * node.
 */
class PropertyAccessError: public Exception
{
  public:
	PropertyAccessError (std::string const& message);
};


/**
 * Indicates that there was a path conflict while creating
 * directory path with mkpath().
 */
class PropertyPathConflict: public Exception
{
  public:
	PropertyPathConflict (std::string const& message);
};


/**
 * Property tree node.
 */
class PropertyNode
{
	friend class PropertyStorage;
	friend class PropertyDirectoryNode;

  protected:
	/**
	 * Create a root node.
	 */
	PropertyNode (PropertyStorage*);

	// Ctor
	PropertyNode (std::string const& name);

  public:
	// Dtor
	virtual ~PropertyNode();

	/**
	 * Return node name.
	 */
	std::string const&
	name() const noexcept;

	/**
	 * Return node path.
	 */
	std::string const&
	path() const noexcept;

	/**
	 * Return parent node.
	 * Root node has no parent.
	 */
	PropertyDirectoryNode*
	parent() const noexcept;

	/**
	 * Return root node. Traverse parents until root node.
	 * Return self if this node is the root node.
	 */
	PropertyDirectoryNode*
	root() noexcept;

	/**
	 * Return pointer to the PropertyStorage object.
	 * If called on non-root property, it will take
	 * additional time to traverse to the root node
	 * to return its storage().
	 */
	PropertyStorage*
	storage() noexcept;

  private:
	/**
	 * Update self-cached location.
	 */
	void
	update_path();

  protected:
	PropertyDirectoryNode*	_parent		= nullptr;
	PropertyStorage*		_storage	= nullptr;
	std::string				_name;
	std::string				_path;
};


/**
 * PropertyNode that is a directory and can
 * have children nodes.
 */
class PropertyDirectoryNode: public PropertyNode
{
	friend class PropertyStorage;

	typedef std::map<std::string, PropertyNodeList::iterator> NameLookup;

  private:
	// Ctor
	PropertyDirectoryNode (PropertyStorage*);

  public:
	// Ctor
	PropertyDirectoryNode (std::string const& name);

	/**
	 * Deletes child properties.
	 */
	~PropertyDirectoryNode();

	/**
	 * Return list of child nodes.
	 */
	PropertyNodeList
	children() const;

	/**
	 * Find a child by its name.
	 * Return nullptr, if not found.
	 */
	PropertyNode*
	child (std::string const& name);

	/**
	 * Search for a property matching given path.
	 * "/" at the beginning jumps to the root node.
	 * "//" at any point jumps to the root node.
	 * ".." jumps to the parent node.
	 * Return nullptr if the node is not found.
	 * For accessing direct descendands, child() is faster.
	 */
	PropertyNode*
	locate (std::string const& path);

	/**
	 * Create directory hierarchy. Return bottom-leaf directory node.
	 * If there's already an existing node in the path,
	 * and it's not a directory-type node, throw PropertyPathConflict.
	 * The part already created will remain in there.
	 */
	PropertyDirectoryNode*
	mkpath (std::string const& path);

	/**
	 * Add new property as a subproperty.
	 */
	PropertyNode*
	add_child (PropertyNode*);

	/**
	 * Remove child property.
	 * Do not delete the child.
	 */
	void
	remove_child (PropertyNode*);

	/**
	 * Removes and deletes child properties.
	 */
	void
	clear();

  private:
	PropertyNodeList	_children;
	NameLookup			_children_by_name;
};


/**
 * Non-template base for PropertyValueNode.
 */
class BasePropertyValueNode: public PropertyNode
{
	template<class T>
		friend class PropertyValueNode;

  public:
	BasePropertyValueNode (std::string const& name);

	/**
	 * Return true if property is nil.
	 */
	bool
	is_nil() const noexcept;

	/**
	 * Inverse of is_nil().
	 */
	bool
	valid() const noexcept;

	/**
	 * Write nil value to this property.
	 */
	void
	set_nil() noexcept;

	/**
	 * Return human-readable value for UI.
	 */
	virtual std::string
	stringify() const = 0;

	/**
	 * Parse value and unit.
	 */
	virtual void
	parse (std::string const&) = 0;

  private:
	bool _is_nil = false;
};


/**
 * PropertyNode that holds a value.
 */
template<class tType>
	class PropertyValueNode: public BasePropertyValueNode
	{
	  public:
		typedef tType Type;

	  public:
		// Ctor
		PropertyValueNode (std::string const& name, Type value);

		/**
		 * Copies value and nil-flag from other property node.
		 * Can not be used on PropDirectory nodes.
		 */
		void
		copy (PropertyValueNode const& other);

		/**
		 * Return stored value.
		 */
		Type
		read (Type default_value = Type()) const;

		/**
		 * Write value to this node.
		 */
		void
		write (Type const& value);

		/**
		 * Return human-readable value for UI.
		 */
		std::string
		stringify() const override;

		/**
		 * Parse value and unit.
		 */
		void
		parse (std::string const&) override;

	  private:
		Type _value;
	};


inline
PropertyAccessError::PropertyAccessError (std::string const& message):
	Exception (message)
{ }


inline
PropertyPathConflict::PropertyPathConflict (std::string const& message):
	Exception (message)
{ }


inline
PropertyNode::PropertyNode (PropertyStorage* storage)
{
	_storage = storage;
}


inline
PropertyNode::PropertyNode (std::string const& name):
	_name (name)
{ }


inline
PropertyNode::~PropertyNode()
{ }


inline std::string const&
PropertyNode::name() const noexcept
{
	return _name;
}


inline std::string const&
PropertyNode::path() const noexcept
{
	return _path;
}


inline PropertyDirectoryNode*
PropertyNode::parent() const noexcept
{
	return _parent;
}


inline PropertyDirectoryNode*
PropertyNode::root() noexcept
{
	PropertyNode* p = this;
	while (p->_parent)
		p = p->_parent;
	return dynamic_cast<PropertyDirectoryNode*> (p);
}


inline PropertyStorage*
PropertyNode::storage() noexcept
{
	return _storage ? _storage : root()->_storage;
}


inline
PropertyDirectoryNode::PropertyDirectoryNode (PropertyStorage* storage):
	PropertyNode (storage)
{ }


inline
PropertyDirectoryNode::PropertyDirectoryNode (std::string const& name):
	PropertyNode (name)
{ }


inline
PropertyDirectoryNode::~PropertyDirectoryNode()
{
	clear();
}


inline
BasePropertyValueNode::BasePropertyValueNode (std::string const& name):
	PropertyNode (name)
{ }


inline bool
BasePropertyValueNode::is_nil() const noexcept
{
	return _is_nil;
}


inline bool
BasePropertyValueNode::valid() const noexcept
{
	return !_is_nil;
}


inline void
BasePropertyValueNode::set_nil() noexcept
{
	_is_nil = true;
}


template<class T>
	inline
	PropertyValueNode<T>::PropertyValueNode (std::string const& name, Type value):
		BasePropertyValueNode (name),
		_value (value)
	{ }


template<class T>
	inline void
	PropertyValueNode<T>::copy (PropertyValueNode const& other)
	{
		_value = other._value;
		_is_nil = other._is_nil;
	}


template<class T>
	inline typename PropertyValueNode<T>::Type
	PropertyValueNode<T>::read (Type default_value) const
	{
		if (_is_nil)
			return default_value;
		else
			return _value;
	}


template<class T>
	inline void
	PropertyValueNode<T>::write (Type const& value)
	{
		_is_nil = false;
		_value = value;
	}


template<class T>
	inline std::string
	PropertyValueNode<T>::stringify() const
	{
		return std::to_string (_value);
	}


template<>
	inline std::string
	PropertyValueNode<std::string>::stringify() const
	{
		return _value;
	}


template<class T>
	inline void
	PropertyValueNode<T>::parse (std::string const& str)
	{
		_value.parse (str);
	}


template<>
	inline void
	PropertyValueNode<bool>::parse (std::string const& str)
	{
		_value = (str == "true");
	}


template<>
	inline void
	PropertyValueNode<int64_t>::parse (std::string const& str)
	{
		_value = boost::lexical_cast<int64_t> (str);
	}


template<>
	inline void
	PropertyValueNode<double>::parse (std::string const& str)
	{
		_value = boost::lexical_cast<double> (str);
	}


template<>
	inline void
	PropertyValueNode<std::string>::parse (std::string const& str)
	{
		_value = str;
	}

} // namespace Xefis

#endif

