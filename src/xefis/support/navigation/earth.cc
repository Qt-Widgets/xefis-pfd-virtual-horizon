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

// Standard:
#include <cstddef>
#include <cmath>
#include <algorithm>

// Boost:
#include <boost/format.hpp>

// Xefis:
#include <xefis/utility/numeric.h>

// Local:
#include "earth.h"


namespace xf {

si::Angle::Value
haversine (si::LonLat const& a, si::LonLat const& b)
{
	using std::sin;
	using std::cos;
	using std::atan2;
	using std::sqrt;

	si::Angle dlat = b.lat() - a.lat();
	si::Angle dlon = b.lon() - a.lon();

	si::Angle::Value latsin = sin (dlat / 2.0);
	si::Angle::Value lonsin = sin (dlon / 2.0);

	si::Angle::Value z = latsin * latsin
					   + lonsin * lonsin
					   * cos (a.lat())
					   * cos (b.lat());

	return 2.0 * atan2 (sqrt (z), sqrt (1.0 - z));
}


si::Angle
initial_bearing (si::LonLat const& a, si::LonLat const& b)
{
	using std::sin;
	using std::cos;
	using std::atan2;

	si::Angle dlon = b.lon() - a.lon();
	si::Angle lat1 = a.lat();
	si::Angle lat2 = b.lat();

	double y = sin (dlon) * cos (lat2);
	double x = cos (lat1) * sin (lat2)
			 - sin (lat1) * cos (lat2) * cos (dlon);

	return 1_rad * atan2 (y, x);
}


si::Angle
great_arcs_angle (si::LonLat const& a, si::LonLat const& common, si::LonLat const& b)
{
	using std::arg;

	LonLat z1 (a.lon() - common.lon(), a.lat() - common.lat());
	LonLat zero (0_deg, 0_deg);
	LonLat z2 (b.lon() - common.lon(), b.lat() - common.lat());

	std::complex<si::Angle::Value> x1 (z1.lon().quantity<si::Degree>(), z1.lat().quantity<si::Degree>());
	std::complex<si::Angle::Value> x2 (z2.lon().quantity<si::Degree>(), z2.lat().quantity<si::Degree>());

	return 1_deg * xf::floored_mod<double> ((1_rad * (arg (x1) - arg (x2))).quantity<si::Degree>(), 360.0);
}


std::string
to_dms (si::Angle a, bool three_digits)
{
	auto const angle_degs = a.quantity<si::Degree>();
	auto const degs = std::trunc (xf::floored_mod (angle_degs, -180.0, +180.0));
	auto const remainder = 60.0 * std::abs (angle_degs - degs);
	auto const mins = std::floor (remainder);
	auto const secs = 60.0 * std::abs (remainder - mins);

	char const* fmt = three_digits
		? "%03d°%02d'%02d\""
		: "%02d°%02d'%02d\"";
	return (boost::format (fmt) % static_cast<int> (degs) % static_cast<int> (mins) % static_cast<int> (secs)).str();
}


std::string
to_latitude_dms (Angle a)
{
	std::string dms = to_dms (a, false);

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return "S" + dms.substr (1);
	else
		return "N" + dms;
}


std::string
to_longitude_dms (Angle a)
{
	std::string dms = to_dms (a, true);

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return "W" + dms.substr (1);
	else
		return "E" + dms;
}


/**
 * Mean value for two angles on a circle.
 */
Angle
mean (Angle lhs, Angle rhs)
{
	using std::sin;
	using std::cos;
	using std::atan2;

	Angle::Value x = 0.5 * cos (lhs) + cos (rhs);
	Angle::Value y = 0.5 * sin (lhs) + sin (rhs);
	return 1_rad * atan2 (y, x);
}
} // namespace xf
