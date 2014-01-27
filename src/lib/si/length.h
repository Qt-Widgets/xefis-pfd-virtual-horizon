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

#ifndef SI__LENGTH_H__INCLUDED
#define SI__LENGTH_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Length: public LinearValue<double, Length>
{
	friend class LinearValue<double, Length>;
	friend constexpr Length operator"" _m (long double);
	friend constexpr Length operator"" _m (unsigned long long);
	friend constexpr Length operator"" _km (long double);
	friend constexpr Length operator"" _km (unsigned long long);
	friend constexpr Length operator"" _ft (long double);
	friend constexpr Length operator"" _ft (unsigned long long);
	friend constexpr Length operator"" _nm (long double);
	friend constexpr Length operator"" _nm (unsigned long long);
	friend constexpr Length operator"" _mil (long double);
	friend constexpr Length operator"" _mil (unsigned long long);

  protected:
	/**
	 * Used by the _inhg, _hpa, _psi and similar suffix operators.
	 * To create an Length use these operators directly.
	 */
	explicit constexpr
	Length (ValueType psi);

  public:
	constexpr
	Length() = default;

	constexpr
	Length (Length const&) = default;

	std::vector<std::string> const&
	supported_units() const override;

	constexpr ValueType
	m() const noexcept;

	constexpr ValueType
	km() const noexcept;

	constexpr ValueType
	ft() const noexcept;

	constexpr ValueType
	nm() const noexcept;

	constexpr ValueType
	mil() const noexcept;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

  private:
	static std::vector<std::string> _supported_units;
};


/*
 * Global functions
 */


inline constexpr Length
operator"" _m (long double m)
{
	return Length (static_cast<Length::ValueType> (m));
}


inline constexpr Length
operator"" _m (unsigned long long m)
{
	return Length (static_cast<Length::ValueType> (m));
}


inline constexpr Length
operator"" _km (long double km)
{
	return Length (static_cast<Length::ValueType> (km) * 1000.0);
}


inline constexpr Length
operator"" _km (unsigned long long km)
{
	return Length (static_cast<Length::ValueType> (km) * 1000.0);
}


inline constexpr Length
operator"" _ft (long double ft)
{
	return Length (static_cast<Length::ValueType> (ft) * 0.3048);
}


inline constexpr Length
operator"" _ft (unsigned long long ft)
{
	return Length (static_cast<Length::ValueType> (ft) * 0.3048);
}


inline constexpr Length
operator"" _nm (long double nm)
{
	return Length (static_cast<Length::ValueType> (nm) * 1852.0);
}


inline constexpr Length
operator"" _nm (unsigned long long nm)
{
	return Length (static_cast<Length::ValueType> (nm) * 1852.0);
}


inline constexpr Length
operator"" _mil (long double mil)
{
	return Length (static_cast<Length::ValueType> (mil) * 1609.344);
}


inline constexpr Length
operator"" _mil (unsigned long long mil)
{
	return Length (static_cast<Length::ValueType> (mil) * 1609.344);
}


/*
 * Length implementation
 */


inline constexpr
Length::Length (ValueType m):
	LinearValue (m)
{ }


inline std::vector<std::string> const&
Length::supported_units() const
{
	return _supported_units;
}


inline constexpr Length::ValueType
Length::m() const noexcept
{
	return internal();
}


inline constexpr Length::ValueType
Length::km() const noexcept
{
	return internal() * 0.001;
}


inline constexpr Length::ValueType
Length::ft() const noexcept
{
	return internal() * 3.280839895;
}


inline constexpr Length::ValueType
Length::nm() const noexcept
{
	return internal() * 0.0005399568;
}


inline constexpr Length::ValueType
Length::mil() const noexcept
{
	return internal() * 0.0006213711;
}


inline void
Length::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "m")
		*this = p.first * 1_m;
	else if (p.second == "km")
		*this = p.first * 1_km;
	else if (p.second == "ft")
		*this = p.first * 1_ft;
	else if (p.second == "nm")
		*this = p.first * 1_nm;
	else if (p.second == "mil")
		*this = p.first * 1_mil;
}


inline std::string
Length::stringify() const
{
	return boost::lexical_cast<std::string> (m()) + " m";
}


inline double
Length::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "m")
		return m();
	else if (unit == "km")
		return km();
	else if (unit == "ft")
		return ft();
	else if (unit == "nm")
		return nm();
	else if (unit == "mil")
		return mil();
	else
		throw UnsupportedUnit ("can't convert Length to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Length.
 * Forwards Length::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Length>: public numeric_limits<SI::Length::ValueType>
	{ };

} // namespace std

#endif

