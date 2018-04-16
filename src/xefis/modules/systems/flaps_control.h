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

#ifndef XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/utility/range.h>
#include <xefis/utility/v2/actions.h>


class FlapsControlIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	// How fast flaps should extend/retract:
	xf::Setting<si::AngularVelocity>	angular_velocity	{ this, "angular_velocity", 10_deg / 1_s };
	// Range of output_control property:
	xf::Setting<xf::Range<double>>		control_extents		{ this, "control_extents", { 0.0, 1.0 } };

	/*
	 * Input
	 */

	xf::PropertyIn<bool>				up					{ this, "/input/up", false };
	xf::PropertyIn<bool>				down				{ this, "/input/down", false };
	xf::PropertyIn<si::Angle>			input_setting		{ this, "/input/setting", 0_deg };

	/*
	 * Output
	 */

	xf::PropertyOut<si::Angle>			output_setting		{ this, "/output/setting" };
	xf::PropertyOut<si::Angle>			current				{ this, "/output/current" };
	xf::PropertyOut<double>				control				{ this, "/output/control" };
};


class FlapsControl: public xf::Module<FlapsControlIO>
{
	static constexpr si::Time kUpdateInterval = 10_ms;

  public:
	// Ctor
	explicit
	FlapsControl (std::unique_ptr<FlapsControlIO>, xf::Airframe&, std::string const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	update_flap_position();

  private:
	std::set<si::Angle>			_settings_list;
	xf::Range<si::Angle>		_extents;
	si::Angle					_setting;
	si::Angle					_current;
	Unique<QTimer>				_timer;
	xf::PropChangedTo<bool>		_input_up_clicked		{ io.up, true };
	xf::PropChangedTo<bool>		_input_down_clicked		{ io.down, true };
	xf::PropChanged<si::Angle>	_input_setting_changed	{ io.input_setting };
};

#endif
