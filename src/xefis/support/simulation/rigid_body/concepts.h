/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__CONCEPTS_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__CONCEPTS_H__INCLUDED

// Standard:
#include <concepts>
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf::rigid_body {

class Body;
class Constraint;
class BasicFramePrecalculation;


template<class T>
	concept BodyConcept = std::derived_from<T, Body>;


template<class T>
	concept ConstraintConcept = std::derived_from<T, Constraint>;


template<class T>
	concept BasicFramePrecalculationConcept = std::derived_from<T, BasicFramePrecalculation>;

} // namespace xf::rigid_body

#endif

