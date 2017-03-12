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

#ifndef XEFIS__MODULES__SYSTEMS__PC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__PC_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/range_smoother.h>


class PerformanceComputer: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<si::Velocity>			setting_tev_min_ias					{ this, 0_kt };

	/*
	 * Input
	 */

	x2::PropertyIn<si::Velocity>		input_speed_ias						{ this, "/speed.ias" };
	x2::PropertyIn<si::Velocity>		input_speed_tas						{ this, "/speed.tas" };
	x2::PropertyIn<si::Velocity>		input_speed_gs						{ this, "/speed.gs" };
	x2::PropertyIn<si::Velocity>		input_vertical_speed				{ this, "/vertical-speed" };
	x2::PropertyIn<si::Length>			input_altitude_amsl_std				{ this, "/altitude.amsl" };
	x2::PropertyIn<si::Angle>			input_track_lateral_true			{ this, "/track.lateral.true" };
	x2::PropertyIn<si::Angle>			input_orientation_heading_true		{ this, "/orientation.heading.true" };
	x2::PropertyIn<si::Angle>			input_magnetic_declination			{ this, "/magnetic-declination" };
	x2::PropertyIn<si::Length>			input_density_altitude				{ this, "/density-altitude" };
	x2::PropertyIn<si::Density>			input_air_density_static			{ this, "/air-density.static" };
	x2::PropertyIn<si::Mass>			input_aircraft_mass					{ this, "/aircraft-mass" };
	x2::PropertyIn<si::Angle>			input_flaps_angle					{ this, "/flaps-angle" };
	x2::PropertyIn<si::Angle>			input_spoilers_angle				{ this, "/spoilers-angle" };
	x2::PropertyIn<si::Angle>			input_aoa_alpha						{ this, "/aoa.alpha" };
	x2::PropertyIn<si::Acceleration>	input_load							{ this, "/load" };
	x2::PropertyIn<si::Angle>			input_bank_angle					{ this, "/bank-angle" };
	x2::PropertyIn<si::Acceleration>	input_y_acceleration				{ this, "/acceleration.y" };
	x2::PropertyIn<si::Acceleration>	input_z_acceleration				{ this, "/acceleration.x" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Angle>			output_wind_from_true				{ this, "/wind.from.true" };
	x2::PropertyOut<si::Angle>			output_wind_from_magnetic			{ this, "/wind.from.magnetic" };
	x2::PropertyOut<si::Velocity>		output_wind_tas						{ this, "/wind.tas" };
	x2::PropertyOut<double>				output_glide_ratio					{ this, "/glide-ratio" };
	x2::PropertyOut<std::string>		output_glide_ratio_string			{ this, "/glide-ratio-string" };
	x2::PropertyOut<si::Power>			output_total_energy_variometer		{ this, "/total-energy-variometer" };
	// Current stall IAS (depends on current bank angle):
	x2::PropertyOut<si::Velocity>		output_v_s							{ this, "/v.s" };
	// Stall IAS with wings level:
	x2::PropertyOut<si::Velocity>		output_v_s_0_deg					{ this, "/v.s-0-deg" };
	// Stall IAS at 5° bank angle:
	x2::PropertyOut<si::Velocity>		output_v_s_5_deg					{ this, "/v.s-5-deg" };
	// Stall IAS at 30° bank angle:
	x2::PropertyOut<si::Velocity>		output_v_s_30_deg					{ this, "/v.s-30-deg" };
	// Rotation IAS on take-off:
	x2::PropertyOut<si::Velocity>		output_v_r							{ this, "/v.r" };
	// Max maneuvering IAS:
	x2::PropertyOut<si::Velocity>		output_v_a							{ this, "/v.a" };
	// Approach IAS:
	x2::PropertyOut<si::Velocity>		output_v_approach					{ this, "/v.approach" };
	// Take-off decision speed: TODO compute:
	x2::PropertyOut<si::Velocity>		output_v_1							{ this, "/v.1" };
	// One engine inoperative decision IAS: TODO compute:
	x2::PropertyOut<si::Velocity>		output_v_2							{ this, "/v.2" };
	// Best unpowered range IAS (best glide IAS): TODO compute:
	x2::PropertyOut<si::Velocity>		output_v_bg							{ this, "/v.bg" };
	// Best powered range IAS: TODO compute
	x2::PropertyOut<si::Velocity>		output_v_br							{ this, "/v.br" };
	// Maximum unpowered airborne time IAS (minimum descent IAS): TODO compute
	x2::PropertyOut<si::Velocity>		output_v_md							{ this, "/v.md" };
	// Maximum powered airborne time IAS (best endurance IAS): TODO compute
	x2::PropertyOut<si::Velocity>		output_v_be							{ this, "/v.be" };
	// Best angle of climb (shortest ground distance climb): TODO compute
	x2::PropertyOut<si::Velocity>		output_v_x							{ this, "/v.x" };
	// Best rate of climb (shortest time climb): TODO compute
	x2::PropertyOut<si::Velocity>		output_v_y							{ this, "/v.y" };
	x2::PropertyOut<si::Angle>			output_critical_aoa					{ this, "/aoa.critical" };
	x2::PropertyOut<bool>				output_stall						{ this, "/stall" };
	x2::PropertyOut<double>				output_lift_coefficient				{ this, "/lift-coefficient" };
	x2::PropertyOut<si::Velocity>		output_estimated_ias				{ this, "/estimated.ias" };
	x2::PropertyOut<si::Velocity>		output_estimated_ias_error			{ this, "/estimated.ias-error" };
	x2::PropertyOut<si::Angle>			output_estimated_aoa				{ this, "/estimated.aoa" };
	x2::PropertyOut<si::Angle>			output_estimated_aoa_error			{ this, "/estimated.aoa-error" };
	x2::PropertyOut<si::Angle>			output_slip_skid					{ this, "/slip-skid" };

  public:
	// Ctor
	explicit PerformanceComputer (xf::Airframe*, std::string const& instance = {});

  protected:
	void
	process (x2::Cycle const&) override;

	void
	compute_wind();

	void
	compute_glide_ratio();

	void
	compute_total_energy_variometer();

	void
	compute_speeds();

	void
	compute_speeds_vbg();

	Optional<si::Velocity>
	get_stall_ias (Angle const& max_bank_angle) const;

	Optional<si::Velocity>
	tas_to_ias (si::Velocity const& tas) const;

	void
	compute_critical_aoa();

	void
	compute_C_L();

	void
	compute_C_D();

	void
	compute_estimations();

	void
	compute_slip_skid();

	/**
	 * Convert AOA to IAS for current environment and configuration.
	 * Automatically includes flaps/spoilers angle, so parameter @aoa
	 * should only be wings AOA.
	 *
	 * May return empty result if it's not possible to compute TAS.
	 */
	Optional<si::Velocity>
	aoa_to_tas_now (si::Angle const& aoa, Optional<si::Acceleration> const& load = {}) const;

  private:
	xf::Airframe*					_airframe;
	si::Energy						_prev_total_energy					= 0_J;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::RangeSmoother<si::Angle>	_wind_direction_smoother			{ { 0.0_deg, 360.0_deg }, 5_s };
	xf::Smoother<si::Velocity>		_wind_speed_smoother				{ 5_s };
	xf::Smoother<si::Power>			_total_energy_variometer_smoother	{ 1_s };
	xf::Smoother<double>			_cl_smoother						{ 1_s };
	x2::PropertyObserver			_wind_computer;
	x2::PropertyObserver			_glide_ratio_computer;
	x2::PropertyObserver			_total_energy_variometer_computer;
	x2::PropertyObserver			_speeds_computer;
	x2::PropertyObserver			_aoa_computer;
	x2::PropertyObserver			_cl_computer;
	x2::PropertyObserver			_estimations_computer;
	x2::PropertyObserver			_slip_skid_computer;
};

#endif