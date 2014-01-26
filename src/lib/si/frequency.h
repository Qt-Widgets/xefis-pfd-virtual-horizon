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

#ifndef SI__FREQUENCY_H__INCLUDED
#define SI__FREQUENCY_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Frequency: public LinearValue<double, Frequency>
{
	friend class LinearValue<double, Frequency>;
	friend constexpr Frequency operator"" _Hz (long double);
	friend constexpr Frequency operator"" _Hz (unsigned long long);
	friend constexpr Frequency operator"" _kHz (long double);
	friend constexpr Frequency operator"" _kHz (unsigned long long);
	friend constexpr Frequency operator"" _MHz (long double);
	friend constexpr Frequency operator"" _MHz (unsigned long long);

  protected:
	explicit constexpr
	Frequency (ValueType Hz);

  public:
	constexpr
	Frequency() = default;

	constexpr
	Frequency (Frequency const&) = default;

	std::vector<std::string> const&
	supported_units() const override;

	constexpr ValueType
	Hz() const noexcept;

	constexpr ValueType
	kHz() const noexcept;

	constexpr ValueType
	MHz() const noexcept;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

  private:
	static std::vector<std::string> _supported_units;
};


inline constexpr
Frequency::Frequency (ValueType Hz):
	LinearValue (Hz)
{ }


inline std::vector<std::string> const&
Frequency::supported_units() const
{
	return _supported_units;
}


inline constexpr Frequency::ValueType
Frequency::Hz() const noexcept
{
	return internal();
}


inline constexpr Frequency::ValueType
Frequency::kHz() const noexcept
{
	return internal() * 0.001;
}


inline constexpr Frequency::ValueType
Frequency::MHz() const noexcept
{
	return internal() * 0.000001;
}


inline void
Frequency::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "hz")
		*this = p.first * 1_Hz;
	else if (p.second == "khz")
		*this = p.first * 1_kHz;
	else if (p.second == "mhz")
		*this = p.first * 1_MHz;
}


inline std::string
Frequency::stringify() const
{
	return boost::lexical_cast<std::string> (kHz()) + " kHz";
}


inline double
Frequency::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "hz")
		return Hz();
	else if (unit == "khz")
		return kHz();
	else if (unit == "mhz")
		return MHz();
	else
		throw UnsupportedUnit ("can't convert Frequency to " + unit);
}


/*
 * Global functions
 */


inline constexpr Frequency
operator"" _Hz (long double Hz)
{
	return Frequency (static_cast<Frequency::ValueType> (Hz));
}


inline constexpr Frequency
operator"" _Hz (unsigned long long Hz)
{
	return Frequency (static_cast<Frequency::ValueType> (Hz));
}


inline constexpr Frequency
operator"" _kHz (long double kHz)
{
	return Frequency (static_cast<Frequency::ValueType> (kHz) * 1000.0);
}


inline constexpr Frequency
operator"" _kHz (unsigned long long kHz)
{
	return Frequency (static_cast<Frequency::ValueType> (kHz) * 1000.0);
}


inline constexpr Frequency
operator"" _MHz (long double MHz)
{
	return Frequency (static_cast<Frequency::ValueType> (MHz) * 1000000.0);
}


inline constexpr Frequency
operator"" _MHz (unsigned long long MHz)
{
	return Frequency (static_cast<Frequency::ValueType> (MHz) * 1000000.0);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Frequency.
 * Forwards Frequency::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Frequency>: public numeric_limits<SI::Frequency::ValueType>
	{ };

} // namespace std

#endif

