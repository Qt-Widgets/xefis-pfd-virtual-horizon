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

#ifndef XEFIS__MODULES__SYSTEMS__FLY_BY_WIRE_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLY_BY_WIRE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>


class FlyByWire: public Xefis::Module
{
  public:
	enum class AttitudeMode
	{
		Manual			= 0,
		Stabilized		= 1,
		FlightDirector	= 2,
	};

	enum class ThrottleMode
	{
		Manual			= 0,
		Autothrottle	= 1,
	};

  public:
	// Ctor
	FlyByWire (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	rescue() override;

	/**
	 * Do all FBW computations and write to output properties.
	 */
	void
	compute_fbw();

	/**
	 * Do integration of joystick axes, to compute user-desired attitude
	 * for stabilized control mode.
	 */
	void
	integrate_manual_input (Time update_dt);

	/**
	 * Check properties and diagnose problem on the log.
	 */
	void
	diagnose();

  private:
	Xefis::PropertyObserver		_fbw_computer;
	// Used with joystick input:
	Xefis::PIDControl<double>	_manual_pitch_pid;
	Xefis::PIDControl<double>	_manual_roll_pid;
	Angle						_computed_output_pitch	= 0_deg;
	Angle						_computed_output_roll	= 0_deg;
	// Stabilizer PIDs:
	Xefis::PIDControl<double>	_elevator_pid;
	Xefis::PIDControl<double>	_ailerons_pid;
	Xefis::PIDControl<double>	_rudder_pid;
	Xefis::Smoother<double>		_elevator_smoother		= 50_ms;
	Xefis::Smoother<double>		_ailerons_smoother		= 50_ms;
	// Settings:
	double						_stabilization_gain;
	double						_pitch_gain;
	double						_pitch_p;
	double						_pitch_i;
	double						_pitch_d;
	double						_pitch_error_power;
	double						_roll_gain;
	double						_roll_p;
	double						_roll_i;
	double						_roll_d;
	double						_roll_error_power;
	double						_yaw_gain;
	double						_yaw_p;
	double						_yaw_i;
	double						_yaw_d;
	double						_yaw_error_power;
	// Input:
	// TODO different stabilization parameters for joystick input and for F/D input.
	Xefis::PropertyInteger		_attitude_mode;
	Xefis::PropertyInteger		_throttle_mode;
	Xefis::PropertyAngle		_pitch_extent;
	Xefis::PropertyAngle		_roll_extent;
	Xefis::PropertyFloat		_input_pitch_axis;
	Xefis::PropertyFloat		_input_roll_axis;
	Xefis::PropertyFloat		_input_yaw_axis;
	Xefis::PropertyAngle		_input_pitch;
	Xefis::PropertyAngle		_input_roll;
	Xefis::PropertyFloat		_input_throttle_axis;
	Xefis::PropertyFloat		_input_throttle;
	Xefis::PropertyAngle		_measured_pitch;
	Xefis::PropertyAngle		_measured_roll;
	Xefis::PropertyFloat		_measured_slip_skid_g;
	Xefis::PropertyFloat		_elevator_minimum;
	Xefis::PropertyFloat		_elevator_maximum;
	Xefis::PropertyFloat		_ailerons_minimum;
	Xefis::PropertyFloat		_ailerons_maximum;
	Xefis::PropertyFloat		_rudder_minimum;
	Xefis::PropertyFloat		_rudder_maximum;
	// Output:
	Xefis::PropertyBoolean		_serviceable;
	Xefis::PropertyAngle		_output_control_stick_pitch;
	Xefis::PropertyAngle		_output_control_stick_roll;
	Xefis::PropertyAngle		_output_pitch;
	Xefis::PropertyAngle		_output_roll;
	Xefis::PropertyFloat		_output_elevator;
	Xefis::PropertyFloat		_output_ailerons;
	Xefis::PropertyFloat		_output_rudder;
	Xefis::PropertyFloat		_output_throttle;
};

#endif
