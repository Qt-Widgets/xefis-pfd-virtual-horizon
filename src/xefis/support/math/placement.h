/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__POSITION_ROTATION_H__INCLUDED
#define XEFIS__SUPPORT__MATH__POSITION_ROTATION_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>


namespace xf {

/*
 * NOTE
 * For rotation matrices this is true:
 * inv (R) == ~R
 */

// TODO Perhaps rename *_to_base() to *_to_world(), and maybe *_to_body() to *_to_local()?
template<class pBaseFrame = void, class pFrame = pBaseFrame>
	struct Placement
	{
	  public:
		using BaseFrame			= pBaseFrame;
		using Frame				= pFrame;

		using Position			= SpaceVector<si::Length, BaseFrame>;
		using RotationToBase	= RotationMatrix<BaseFrame, Frame>;
		using RotationToBody	= RotationMatrix<Frame, BaseFrame>;

	  public:
		// Ctor
		Placement() = default;

		// Ctor
		template<class = void>
			requires (!std::is_same<BaseFrame, Frame>())
			Placement (Position const&, RotationToBody const&);

		// Ctor
		Placement (Position const&, RotationToBase const&);

		/**
		 * Body position relative to the BaseFrame frame of reference.
		 */
		[[nodiscard]]
		Position const&
		position() const noexcept
			{ return _position; }

		/**
		 * Update body's position.
		 */
		void
		set_position (Position const& position)
			{ _position = position; }

		/**
		 * Body rotation matrix transforming from BaseFrame to Frame.
		 */
		[[nodiscard]]
		RotationToBody const&
		base_to_body_rotation() const noexcept
			{ return _base_to_body_rotation; }

		/**
		 * Set body's rotation matrix.
		 */
		void
		set_base_to_body_rotation (RotationToBody const&);

		/**
		 * Body rotation matrix transforming from Frame to BaseFrame.
		 */
		[[nodiscard]]
		RotationToBase const&
		body_to_base_rotation() const noexcept
			{ return _body_to_base_rotation; }

		/**
		 * Set body's rotation matrix.
		 */
		void
		set_body_to_base_rotation (RotationToBase const&);

		/**
		 * Translate in-place the body by a relative vector in BaseFrame.
		 */
		void
		translate_frame (Position const& translation)
			{ _position += translation; }

		/**
		 * Translate in-place the body by a relative vector in Frame.
		 */
		template<class = void>
			requires (!std::is_same<BaseFrame, Frame>())
			void
			translate_frame (SpaceVector<si::Length, Frame> const& vector)
				{ translate (_body_to_base_rotation * vector); }

		/**
		 * Rotate in-place the body.
		 */
		void
		rotate_body_frame (RotationMatrix<BaseFrame> const& rotation_matrix);

		/**
		 * Rotate in-place the body around the 0 point in base frame of reference.
		 * Modifies both position vector and rotation matrix.
		 */
		void
		rotate_base_frame (RotationMatrix<BaseFrame> const& rotation_matrix);

		/**
		 * Rotate in-place the body around different point than origin.
		 * Point is represented in base frame of reference.
		 * Modifies both position vector and rotation matrix.
		 */
		void
		rotate_base_frame_about (Position const& about_point, RotationMatrix<BaseFrame> const& rotation_matrix);

		/**
		 * Transform bound geometrical object from base to body space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			bound_transform_to_body (InputObject const& input) const
				{ return _base_to_body_rotation * (input - _position); }

		/**
		 * Transform unbound geometrical object from base to body space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			unbound_transform_to_body (InputObject const& input) const
				{ return _base_to_body_rotation * input; }

		/**
		 * Transform bound geometrical object from body to base space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			bound_transform_to_base (InputObject const& input) const
				{ return _body_to_base_rotation * input + _position; }

		/**
		 * Transform unbound geometrical object from body to base space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			unbound_transform_to_base (InputObject const& input) const
				{ return _body_to_base_rotation * input; }

	  private:
		SpaceVector<si::Length, BaseFrame>	_position				{ math::zero };
		RotationMatrix<Frame, BaseFrame>	_base_to_body_rotation	{ math::unit };
		RotationMatrix<BaseFrame, Frame>	_body_to_base_rotation	{ math::unit };
	};


template<class B, class F>
	template<class>
		requires (!std::is_same<B, F>())
		inline
		Placement<B, F>::Placement (Position const& position, RotationToBody const& rotation):
			_position (position),
			_base_to_body_rotation (rotation),
			_body_to_base_rotation (~rotation)
		{ }


template<class B, class F>
	inline
	Placement<B, F>::Placement (Position const& position, RotationToBase const& rotation):
		_position (position),
		_base_to_body_rotation (~rotation),
		_body_to_base_rotation (rotation)
	{ }


template<class B, class F>
	inline void
	Placement<B, F>::set_base_to_body_rotation (RotationToBody const& rotation)
	{
		_base_to_body_rotation = rotation;
		_body_to_base_rotation = ~rotation;
	}


template<class B, class F>
	inline void
	Placement<B, F>::set_body_to_base_rotation (RotationToBase const& rotation)
	{
		_body_to_base_rotation = rotation;
		_base_to_body_rotation = ~rotation;
	}


template<class B, class F>
	inline void
	Placement<B, F>::rotate_body_frame (RotationMatrix<BaseFrame> const& rotation_matrix)
	{
		_body_to_base_rotation = rotation_matrix * _body_to_base_rotation;
		_base_to_body_rotation = ~_body_to_base_rotation;
	}


template<class B, class F>
	inline void
	Placement<B, F>::rotate_base_frame (RotationMatrix<BaseFrame> const& rotation_matrix)
	{
		_position = rotation_matrix * _position;
		rotate_body_frame (rotation_matrix);
	}


template<class B, class F>
	inline void
	Placement<B, F>::rotate_base_frame_about (Position const& about_point, RotationMatrix<BaseFrame> const& rotation_matrix)
	{
		_position -= about_point;
		rotate_base_frame (rotation_matrix);
		_position += about_point;
	}

} // namespace xf

#endif

