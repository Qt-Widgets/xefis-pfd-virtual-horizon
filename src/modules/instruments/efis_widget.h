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

#ifndef XEFIS__MODULES__INSTRUMENTS__EFIS_WIDGET_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__EFIS_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtGui/QPainterPath>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/utility/text_painter.h>


class EFISWidget: public Xefis::InstrumentWidget
{
	Q_OBJECT

	typedef std::map<QString, Knots> SpeedBugs;
	typedef std::map<QString, Feet> AltitudeBugs;

  public:
	// Ctor
	EFISWidget (QWidget* parent);

	/**
	 * Set how often lines should be drawn on speed ladder.
	 */
	void
	set_speed_ladder_line_every (int knots);

	/**
	 * Set how often numbers should be drawn on speed ladder.
	 */
	void
	set_speed_ladder_number_every (int knots);

	/**
	 * Set speed ladder scale extent.
	 */
	void
	set_speed_ladder_extent (int knots);

	/**
	 * Set how often lines should be drawn on altitude ladder.
	 */
	void
	set_altitude_ladder_line_every (int feet);

	/**
	 * Set how often numbers should be drawn on altitude ladder.
	 */
	void
	set_altitude_ladder_number_every (int feet);

	/**
	 * Set how often lines should be drawn bold on altitude ladder.
	 */
	void
	set_altitude_ladder_bold_every (int feet);

	/**
	 * Set altitude ladder scale extent.
	 */
	void
	set_altitude_ladder_extent (int feet);

	/**
	 * Return current pitch value.
	 */
	Angle
	pitch() const;

	/**
	 * Set pitch value.
	 */
	void
	set_pitch (Angle);

	/**
	 * Toggle pitch scale visibility.
	 * Toggles also artifical horizon.
	 */
	void
	set_pitch_visible (bool visible);

	/**
	 * Pitch limit.
	 */
	Angle
	pitch_limit() const;

	/**
	 * Set pitch limit (absolute value).
	 */
	void
	set_pitch_limit (Angle pitch_limit);

	/**
	 * Set pitch limit indicator visibility.
	 */
	void
	set_pitch_limit_visible (bool visible);

	/**
	 * Return current roll value.
	 */
	Angle
	roll() const;

	/**
	 * Set roll value.
	 */
	void
	set_roll (Angle);

	/**
	 * Return roll limit.
	 */
	Angle
	roll_limit() const;

	/**
	 * Set roll limit, where indicator will turn solid amber.
	 * 0.f disables the limit.
	 */
	void
	set_roll_limit (Angle limit);

	/**
	 * Toggle roll scale visibility.
	 * Toggles also artifical horizon.
	 */
	void
	set_roll_visible (bool visible);

	/**
	 * Return current heading value.
	 */
	Angle
	heading() const;

	/**
	 * Set heading value.
	 */
	void
	set_heading (Angle);

	/**
	 * Toggle heading scale visibility.
	 */
	void
	set_heading_visible (bool visible);

	/**
	 * Toggle heading scale numbers visibility (only on ADI, not on NAV widget).
	 */
	void
	set_heading_numbers_visible (bool visible);

	/**
	 * Return slip-skid indicator value.
	 */
	float
	slip_skid() const;

	/**
	 * Return slip-skid limit.
	 */
	float
	slip_skid_limit() const;

	/**
	 * Set slip-skid value.
	 */
	void
	set_slip_skid (float value);

	/**
	 * Set slip-skid limit, where indicator will turn solid amber.
	 * 0.f disables the limit.
	 */
	void
	set_slip_skid_limit (float limit);

	/**
	 * Set slip-skid indicator visibility.
	 */
	void
	set_slip_skid_visible (bool visible);

	/**
	 * Flight path vertical deviation.
	 */
	Angle
	flight_path_alpha() const;

	/**
	 * Set flight path vertical deviation.
	 */
	void
	set_flight_path_alpha (Angle);

	/**
	 * Flight path horizontal deviation.
	 */
	Angle
	flight_path_beta() const;

	/**
	 * Set flight path horizontal deviation.
	 */
	void
	set_flight_path_beta (Angle);

	/**
	 * Set visibility of the Flight Path Marker.
	 */
	void
	set_flight_path_marker_visible (bool visible);

	/**
	 * Return current speed.
	 */
	Knots
	speed() const;

	/**
	 * Set speed shown on speed ladder.
	 */
	void
	set_speed (Knots);

	/**
	 * Toggle visibility of the speed scale.
	 */
	void
	set_speed_visible (bool visible);

	/**
	 * Set speed tendency value.
	 */
	void
	set_speed_tendency (Knots);

	/**
	 * Set speed tendency arrow visibility.
	 */
	void
	set_speed_tendency_visible (bool visible);

	/**
	 * Current altitude value.
	 */
	Feet
	altitude() const;

	/**
	 * Set altitude value.
	 */
	void
	set_altitude (Feet);

	/**
	 * Toggle visibility of the altitude scale.
	 */
	void
	set_altitude_visible (bool visible);

	/**
	 * Set altitude tendency value.
	 */
	void
	set_altitude_tendency (Feet);

	/**
	 * Set altitude tendency arrow visibility.
	 */
	void
	set_altitude_tendency_visible (bool visible);

	/**
	 * Set radar altitude.
	 */
	void
	set_altitude_agl (Feet);

	/**
	 * Set radar altitude visibility.
	 */
	void
	set_altitude_agl_visible (bool visible);

	/**
	 * Return landing altitude.
	 */
	Feet
	landing_altitude() const;

	/**
	 * Set landing altitude.
	 */
	void
	set_landing_altitude (Feet);

	/**
	 * Set landing altitude visibility.
	 */
	void
	set_landing_altitude_visible (bool visible);

	/**
	 * Return transition altitude.
	 */
	Feet
	transition_altitude() const;

	/**
	 * Set transition altitude.
	 */
	void
	set_transition_altitude (Feet);

	/**
	 * Set transition altitude visibility.
	 */
	void
	set_transition_altitude_visible (bool visible);

	/**
	 * Return current climb rate.
	 */
	FeetPerMinute
	climb_rate() const;

	/**
	 * Set climb rate.
	 */
	void
	set_climb_rate (FeetPerMinute feet_per_minute);

	/**
	 * Set climb rate visibility.
	 */
	void
	set_climb_rate_visible (bool visible);

	/**
	 * Return speed bug value or 0.0f if not found.
	 */
	Knots
	speed_bug (QString name) const;

	/**
	 * Add new speed bug.
	 */
	void
	add_speed_bug (QString name, Knots speed);

	/**
	 * Remove a speed bug.
	 * Pass QString::null to remove all speed bugs.
	 */
	void
	remove_speed_bug (QString name);

	/**
	 * Return altitude bug value of 0.0f if not found.
	 */
	Feet
	altitude_bug (QString name) const;

	/**
	 * Add new altitude bug.
	 */
	void
	add_altitude_bug (QString name, Feet altitude);

	/**
	 * Remove an altitude bug.
	 * Pass QString::null to remove all altitude bugs.
	 */
	void
	remove_altitude_bug (QString name);

	/**
	 * Return mach number.
	 */
	float
	mach() const;

	/**
	 * Set mach number indicator.
	 */
	void
	set_mach (float value);

	/**
	 * Set mach number indicator visibility.
	 */
	void
	set_mach_visible (bool visible);

	/**
	 * Return current pressure indicator value.
	 */
	Pressure
	pressure() const;

	/**
	 * Set pressure indicator.
	 */
	void
	set_pressure (Pressure pressure);

	/**
	 * Set pressure unit to be hPa instead of inHg.
	 */
	void
	set_pressure_display_hpa (bool hpa);

	/**
	 * Show or hide pressure indicator.
	 */
	void
	set_pressure_visible (bool visible);

	/**
	 * Enable/disable standard pressure.
	 */
	void
	set_standard_pressure (bool standard);

	/**
	 * Get minimum speed indicator setting.
	 */
	Knots
	minimum_speed() const;

	/**
	 * Set minimum speed indicator on the speed ladder.
	 */
	void
	set_minimum_speed (Knots);

	/**
	 * Set minimum speed indicator visibility.
	 */
	void
	set_minimum_speed_visible (bool visible);

	/**
	 * Get warning speed indicator setting.
	 */
	Knots
	warning_speed() const;

	/**
	 * Set warning speed indicator on the speed ladder.
	 */
	void
	set_warning_speed (Knots);

	/**
	 * Set warning speed indicator visibility.
	 */
	void
	set_warning_speed_visible (bool visible);

	/**
	 * Get maximum speed indicator setting.
	 */
	Knots
	maximum_speed() const;

	/**
	 * Set maximum speed indicator on the speed ladder.
	 */
	void
	set_maximum_speed (Knots);

	/**
	 * Set maximum speed indicator visibility.
	 */
	void
	set_maximum_speed_visible (bool visible);

	/**
	 * Return autopilot altitude.
	 */
	Feet
	ap_altitude() const;

	/**
	 * Set autopilot altitude.
	 */
	void
	set_ap_altitude (Feet);

	/**
	 * Set AP altitude setting visibility.
	 */
	void
	set_ap_altitude_visible (bool visible);

	/**
	 * Return autopilot climb rate setting.
	 */
	FeetPerMinute
	ap_climb_rate() const;

	/**
	 * Set autopilot climb rate setting.
	 */
	void
	set_ap_climb_rate (FeetPerMinute);

	/**
	 * Set AP climb rate visibility.
	 */
	void
	set_ap_climb_rate_visible (bool visible);

	/**
	 * Return autothrottle setting.
	 */
	Knots
	at_speed() const;

	/**
	 * Set autothrottle speed.
	 */
	void
	set_at_speed (Knots);

	/**
	 * Set AT speed visibility.
	 */
	void
	set_at_speed_visible (bool visible);

	/**
	 * Return flight director alpha.
	 */
	Angle
	flight_director_pitch() const;

	/**
	 * Set flight director pitch.
	 */
	void
	set_flight_director_pitch (Angle pitch);

	/**
	 * Set flight director pitch visibility.
	 */
	void
	set_flight_director_pitch_visible (bool visible);

	/**
	 * Return flight director roll.
	 */
	Angle
	flight_director_roll() const;

	/**
	 * Set flight director roll.
	 */
	void
	set_flight_director_roll (Angle roll);

	/**
	 * Set flight director roll visibility.
	 */
	void
	set_flight_director_roll_visible (bool visible);

	/**
	 * Return control stick indicator pitch.
	 */
	Angle
	control_stick_pitch() const;

	/**
	 * Set control stick indicator pitch.
	 */
	void
	set_control_stick_pitch (Angle pitch);

	/**
	 * Return control stick indicator roll.
	 */
	Angle
	control_stick_roll() const;

	/**
	 * Set control stick indicator roll.
	 */
	void
	set_control_stick_roll (Angle roll);

	/**
	 * Set visibility of the control stick indicator.
	 */
	void
	set_control_stick_visible (bool visible);

	/**
	 * Set visibility of approach reference info (localizer/glideslope needles,
	 * localizer ID/bearing, DME, etc).
	 */
	void
	set_approach_reference_visible (bool visible);

	/**
	 * Return vertical deviation value.
	 */
	Angle
	vertical_deviation() const;

	/**
	 * Set vertical deviation.
	 */
	void
	set_vertical_deviation (Angle deviation);

	/**
	 * Set navigation vertical needle visibility.
	 */
	void
	set_vertical_deviation_visible (bool visible);

	/**
	 * Return navigation heading needle value.
	 */
	Angle
	lateral_deviation() const;

	/**
	 * Set localizer deviation needle.
	 */
	void
	set_lateral_deviation (Angle value);

	/**
	 * Set navigation heading needle visibility.
	 */
	void
	set_lateral_deviation_visible (bool visible);

	/**
	 * Set runway visibility (aligns with lateral deviation needle).
	 */
	void
	set_runway_visible (bool visible);

	/**
	 * Return nav hint.
	 */
	QString
	approach_hint() const;

	/**
	 * Set navigation hint, a text shown on the top left corner of the ADI.
	 * Usually something like "ILS" or "VOR".
	 */
	void
	set_approach_hint (QString hint);

	/**
	 * Return DME distance.
	 */
	Length
	dme_distance() const;

	/**
	 * Set DME distance.
	 */
	void
	set_dme_distance (Length);

	/**
	 * Set DME info visibility.
	 */
	void
	set_dme_distance_visible (bool visible);

	/**
	 * Return localizer ID.
	 */
	QString
	localizer_id() const;

	/**
	 * Set localizer ID.
	 */
	void
	set_localizer_id (QString const& loc_id);

	/**
	 * Return localizer mag bearing.
	 */
	Angle
	localizer_magnetic_bearing() const;

	/**
	 * Set localizer magnetic bearing.
	 */
	void
	set_localizer_magnetic_bearing (Angle mag_bearing);

	/**
	 * Set visibility of localizer ID and its bearing.
	 */
	void
	set_localizer_info_visible (bool visible);

	/**
	 * Return control hint.
	 */
	QString
	control_hint() const;

	/**
	 * Set control hint - the text displayed right above roll scale.
	 */
	void
	set_control_hint (QString const&);

	/**
	 * Set visibility of the control hint.
	 */
	void
	set_control_hint_visible (bool visible);

	/**
	 * Set AP hints visibility (three boxes on the top).
	 */
	void
	set_ap_hints_visible (bool visible);

	/**
	 * Return AP speed hint text.
	 */
	QString
	ap_speed_hint() const;

	/**
	 * Set AP speed hint text.
	 */
	void
	set_ap_speed_hint (QString const&);

	/**
	 * Return additional AP speed hint text.
	 */
	QString
	ap_speed_small_hint() const;

	/**
	 * Set additional AP speed hint text.
	 */
	void
	set_ap_speed_small_hint (QString const&);

	/**
	 * Return AP lateral hint text.
	 */
	QString
	ap_lateral_hint() const;

	/**
	 * Set AP lateral hint text.
	 */
	void
	set_ap_lateral_hint (QString const&);

	/**
	 * Return additional AP lateral hint text.
	 */
	QString
	ap_lateral_small_hint() const;

	/**
	 * Set additional AP lateral hint text.
	 */
	void
	set_ap_lateral_small_hint (QString const&);

	/**
	 * Return AP altitude hint text.
	 */
	QString
	ap_vertical_hint() const;

	/**
	 * Set AP altitude hint text.
	 */
	void
	set_ap_vertical_hint (QString const&);

	/**
	 * Return additional AP altitude hint text.
	 */
	QString
	ap_vertical_small_hint() const;

	/**
	 * Set additional AP altitude hint text.
	 */
	void
	set_ap_vertical_small_hint (QString const&);

	/**
	 * Return field of view.
	 * Default is 120°. Usable maximum: 180°.
	 */
	Angle
	fov() const;

	/**
	 * Set field of view.
	 */
	void
	set_fov (Angle);

	/**
	 * Set input alert visibility.
	 */
	void
	set_input_alert_visible (bool visible);

  private slots:
	void
	blink_speed();

	void
	blink_baro();

  private:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	/*
	 * ADI
	 */

	void
	adi_post_resize();

	void
	adi_pre_paint();

	void
	adi_paint (QPainter&, TextPainter&);

	void
	adi_paint_horizon (QPainter&);

	void
	adi_paint_pitch (QPainter&, TextPainter&);

	void
	adi_paint_roll (QPainter&);

	void
	adi_paint_heading (QPainter&, TextPainter&);

	void
	adi_paint_flight_path_marker (QPainter&);

	/*
	 * Speed ladder
	 */

	void
	sl_post_resize();

	void
	sl_pre_paint();

	void
	sl_paint (QPainter&, TextPainter&);

	void
	sl_paint_black_box (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_ladder_scale (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_speed_limits (QPainter& painter, float x);

	void
	sl_paint_speed_tendency (QPainter& painter, float x);

	void
	sl_paint_bugs (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_mach_number (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_ap_setting (QPainter& painter, TextPainter&);

	float
	kt_to_px (Knots ft) const;

	/*
	 * Altitude ladder
	 */

	void
	al_post_resize();

	void
	al_pre_paint();

	void
	al_paint (QPainter&, TextPainter&);

	void
	al_paint_black_box (QPainter&, TextPainter&, float x);

	void
	al_paint_ladder_scale (QPainter&, TextPainter&, float x);

	void
	al_paint_altitude_tendency (QPainter&, float x);

	void
	al_paint_bugs (QPainter&, TextPainter&, float x);

	void
	al_paint_climb_rate (QPainter&, TextPainter&, float x);

	void
	al_paint_pressure (QPainter&, TextPainter&, float x);

	void
	al_paint_ap_setting (QPainter&, TextPainter&);

	float
	ft_to_px (Feet ft) const;

	float
	scale_cbr (FeetPerMinute climb_rate) const;

	/*
	 * Other
	 */

	void
	paint_center_cross (QPainter&, bool center_box, bool rest);

	void
	paint_flight_director (QPainter&);

	void
	paint_control_stick (QPainter&);

	void
	paint_altitude_agl (QPainter&, TextPainter&);

	void
	paint_baro_setting (QPainter&, TextPainter&);

	void
	paint_nav (QPainter&, TextPainter&);

	void
	paint_hints (QPainter&, TextPainter&);

	void
	paint_pitch_limit (QPainter&);

	void
	paint_input_alert (QPainter&, TextPainter&);

	void
	paint_dashed_zone (QPainter&, QColor const&, QRectF const& target);

	/**
	 * Render 'rotatable' value on speed/altitude black box.
	 *
	 * \param	painter
	 * 			QPainter to use.
	 * \param	text_painter
	 * 			TextPainter to use.
	 * \param	position
	 * 			Text position, [-0.5, 0.5].
	 * \param	next, curr, prev
	 * 			Texts to render. Special value "G" paints green dashed zone, "R" paints red dashed zone.
	 */
	void
	paint_rotating_value (QPainter& painter, TextPainter& text_painter,
						  QRectF const& rect, float position, float height_scale,
						  QString const& next, QString const& curr, QString const& prev);

	/**
	 * \param	two_zeros
	 * 			Two separate zeros, for positive and negative values.
	 * \param	zero_mark
	 * 			Draw red/green/blank mark instead of zero.
	 */
	void
	paint_rotating_digit (QPainter& painter, TextPainter& text_painter,
						  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
						  bool two_zeros, bool zero_mark, bool black_zero = false);

	/**
	 * Start or stop blinking warning timer on given condition.
	 */
	void
	update_blinker (QTimer* warning_timer, bool condition, bool* blink_state);

	float
	pitch_to_px (Angle degrees) const;

	float
	heading_to_px (Angle degrees) const;

	QPainterPath
	get_pitch_scale_clipping_path() const;

	QColor
	get_baro_color() const;

  private:
	QColor				_sky_color;
	QColor				_ground_color;
	QColor				_ladder_color;
	QColor				_ladder_border_color;
	QColor				_warning_color_1;
	QColor				_warning_color_2;
	QTransform			_center_transform;
	QTransform			_pitch_transform;
	QTransform			_roll_transform;
	QTransform			_heading_transform;
	QTransform			_horizon_transform;
	Angle				_fov							= 120_deg;
	bool				_input_alert_visible			= false;
	TextPainter::Cache	_text_painter_cache;
	QTimer*				_speed_blinking_warning			= nullptr;
	bool				_speed_blink					= false;
	QTimer*				_baro_blinking_warning			= nullptr;
	bool				_baro_blink						= false;

	float				_w;
	float				_h;
	float				_max_w_h;

	/*
	 * ADI
	 */

	QRectF				_adi_sky_rect;
	QRectF				_adi_gnd_rect;
	QPainterPath		_flight_path_marker_shape;
	QPainterPath		_flight_path_marker_clip;
	QPointF				_flight_path_marker_position;

	/*
	 * Speed ladder
	 */

	QTransform			_sl_transform;
	Knots				_sl_extent						= 124;
	int					_sl_line_every					= 10;
	int					_sl_number_every				= 20;
	Knots				_sl_min_shown;
	Knots				_sl_max_shown;
	int					_sl_rounded_speed;
	QRectF				_sl_ladder_rect;
	QPen				_sl_ladder_pen;
	QRectF				_sl_black_box_rect;
	QPen				_sl_black_box_pen;
	QPen				_sl_scale_pen;
	QPen				_sl_speed_bug_pen;
	float				_sl_margin;
	int					_sl_digits;

	/*
	 * Altitude ladder
	 */

	QTransform			_al_transform;
	int					_al_line_every					= 100;
	int					_al_number_every				= 200;
	int					_al_bold_every					= 500;
	Feet				_al_extent						= 825;
	Feet				_al_min_shown;
	Feet				_al_max_shown;
	int					_al_rounded_altitude;
	QRectF				_al_ladder_rect;
	QPen				_al_ladder_pen;
	QRectF				_al_black_box_rect;
	QPen				_al_black_box_pen;
	QPen				_al_scale_pen_1;
	QPen				_al_scale_pen_2; // Bold one, each 500 ft
	QPen				_al_negative_altitude_pen;
	QPen				_al_altitude_bug_pen;
	QPen				_al_ldg_alt_pen;
	QRectF				_al_b_digits_box;
	QRectF				_al_s_digits_box;
	float				_al_margin;

	/*
	 * Parameters
	 */

	Angle				_pitch							= 0_deg;
	Angle				_pitch_limit					= 0_deg;
	bool				_pitch_visible					= false;
	bool				_pitch_limit_visible			= false;
	Angle				_roll							= 0_deg;
	Angle				_roll_limit						= 0_deg;
	bool				_roll_visible					= false;
	Angle				_heading						= 0_deg;
	bool				_heading_visible				= false;
	bool				_heading_numbers_visible		= false;
	float				_slip_skid						= 0.f;
	float				_slip_skid_limit				= 0.f;
	bool				_slip_skid_visible				= false;
	Angle				_flight_path_alpha				= 0_deg;
	Angle				_flight_path_beta				= 0_deg;
	bool				_flight_path_visible			= false;
	Knots				_speed							= 0.f;
	bool				_speed_visible					= false;
	Knots				_speed_tendency					= 0.f;
	bool				_speed_tendency_visible			= false;
	Feet				_altitude						= 0.f;
	bool				_altitude_visible				= false;
	Feet				_altitude_tendency				= 0.f;
	bool				_altitude_tendency_visible		= false;
	Feet				_altitude_agl					= 0.f;
	bool				_altitude_agl_visible			= false;
	Feet				_landing_altitude				= 0.f;
	bool				_landing_altitude_visible		= false;
	Feet				_transition_altitude			= 0.f;
	bool				_transition_altitude_visible	= false;
	FeetPerMinute		_climb_rate						= 0.f;
	bool				_climb_rate_visible				= false;
	float				_mach							= 0.f;
	bool				_mach_visible					= false;
	Pressure			_pressure						= 0_inhg;
	bool				_pressure_display_hpa			= false;
	bool				_pressure_visible				= false;
	bool				_standard_pressure				= false;
	Knots				_minimum_speed					= 0.f;
	bool				_minimum_speed_visible			= false;
	Knots				_warning_speed					= 0.f;
	bool				_warning_speed_visible			= false;
	Knots				_maximum_speed					= 0.f;
	bool				_maximum_speed_visible			= false;
	Feet				_ap_altitude					= 0.f;
	bool				_ap_altitude_visible			= false;
	FeetPerMinute		_ap_climb_rate					= 0.f;
	bool				_ap_climb_rate_visible			= false;
	Knots				_at_speed						= 0.f;
	bool				_at_speed_visible				= false;
	Angle				_flight_director_pitch			= 0_deg;
	bool				_flight_director_pitch_visible	= false;
	Angle				_flight_director_roll			= 0_deg;
	bool				_flight_director_roll_visible	= false;
	Angle				_control_stick_pitch			= 0_deg;
	Angle				_control_stick_roll				= 0_deg;
	bool				_control_stick_visible			= false;
	bool				_approach_reference_visible		= false;
	Angle				_vertical_deviation_deg			= 0_deg;
	bool				_vertical_deviation_visible		= false;
	Angle				_lateral_deviation_deg			= 0_deg;
	bool				_lateral_deviation_visible		= false;
	bool				_runway_visible					= false;
	QString				_approach_hint;
	Length				_dme_distance					= 0_nm;
	bool				_dme_distance_visible			= false;
	QString				_localizer_id;
	Angle				_localizer_magnetic_bearing		= 0_deg;
	bool				_localizer_info_visible			= false;
	QString				_control_hint;
	bool				_control_hint_visible			= false;
	bool				_ap_hints_visible				= false;
	QString				_ap_speed_hint;
	QString				_ap_speed_small_hint;
	QString				_ap_lateral_hint;
	QString				_ap_lateral_small_hint;
	QString				_ap_vertical_hint;
	QString				_ap_vertical_small_hint;
	SpeedBugs			_speed_bugs;
	AltitudeBugs		_altitude_bugs;
};


inline void
EFISWidget::set_speed_ladder_line_every (int knots)
{
	_sl_line_every = std::max (1, knots);
	update();
}


inline void
EFISWidget::set_speed_ladder_number_every (int knots)
{
	_sl_number_every = std::max (1, knots);
	update();
}


inline void
EFISWidget::set_speed_ladder_extent (int knots)
{
	_sl_extent = std::max (1, knots);
	update();
}


inline void
EFISWidget::set_altitude_ladder_line_every (int feet)
{
	_al_line_every = std::max (1, feet);
	update();
}


inline void
EFISWidget::set_altitude_ladder_number_every (int feet)
{
	_al_number_every = std::max (1, feet);
	update();
}


inline void
EFISWidget::set_altitude_ladder_bold_every (int feet)
{
	_al_bold_every = std::max (1, feet);
	update();
}


inline void
EFISWidget::set_altitude_ladder_extent (int feet)
{
	_al_extent = std::max (1, feet);
	update();
}


inline Angle
EFISWidget::pitch() const
{
	return _pitch;
}


inline void
EFISWidget::set_pitch (Angle degrees)
{
	_pitch = degrees;
	update();
}


inline void
EFISWidget::set_pitch_visible (bool visible)
{
	_pitch_visible = visible;
	update();
}


inline Angle
EFISWidget::pitch_limit() const
{
	return _pitch_limit;
}


inline void
EFISWidget::set_pitch_limit (Angle pitch_limit)
{
	_pitch_limit = pitch_limit;
	update();
}


inline void
EFISWidget::set_pitch_limit_visible (bool visible)
{
	_pitch_limit_visible = visible;
	update();
}


inline Angle
EFISWidget::roll() const
{
	return _roll;
}


inline Angle
EFISWidget::roll_limit() const
{
	return _roll_limit;
}


inline void
EFISWidget::set_roll (Angle degrees)
{
	_roll = degrees;
	update();
}


inline void
EFISWidget::set_roll_limit (Angle limit)
{
	_roll_limit = limit;
	update();
}


inline void
EFISWidget::set_roll_visible (bool visible)
{
	_roll_visible = visible;
	update();
}


inline Angle
EFISWidget::heading() const
{
	return _heading;
}


inline void
EFISWidget::set_heading (Angle degrees)
{
	_heading = degrees;
	update();
}


inline void
EFISWidget::set_heading_visible (bool visible)
{
	_heading_visible = visible;
	update();
}


inline void
EFISWidget::set_heading_numbers_visible (bool visible)
{
	_heading_numbers_visible = visible;
	update();
}


inline float
EFISWidget::slip_skid() const
{
	return _slip_skid;
}


inline float
EFISWidget::slip_skid_limit() const
{
	return _slip_skid_limit;
}


inline void
EFISWidget::set_slip_skid (float value)
{
	_slip_skid = value;
	update();
}


inline void
EFISWidget::set_slip_skid_limit (float limit)
{
	_slip_skid_limit = limit;
	update();
}


inline void
EFISWidget::set_slip_skid_visible (bool visible)
{
	_slip_skid_visible = visible;
	update();
}


inline Angle
EFISWidget::flight_path_alpha() const
{
	return _flight_path_alpha;
}


inline void
EFISWidget::set_flight_path_alpha (Angle pitch)
{
	_flight_path_alpha = pitch;
	update();
}


inline Angle
EFISWidget::flight_path_beta() const
{
	return _flight_path_beta;
}


inline void
EFISWidget::set_flight_path_beta (Angle heading)
{
	_flight_path_beta = heading;
	update();
}


inline void
EFISWidget::set_flight_path_marker_visible (bool visible)
{
	_flight_path_visible = visible;
	update();
}


inline Knots
EFISWidget::speed() const
{
	return _speed;
}


inline void
EFISWidget::set_speed (Knots speed)
{
	_speed = speed;
	update();
}


inline void
EFISWidget::set_speed_visible (bool visible)
{
	_speed_visible = visible;
	update();
}


inline void
EFISWidget::set_speed_tendency (Knots kt)
{
	_speed_tendency = kt;
	update();
}


inline void
EFISWidget::set_speed_tendency_visible (bool visible)
{
	_speed_tendency_visible = visible;
	update();
}


inline Feet
EFISWidget::altitude() const
{
	return _altitude;
}


inline void
EFISWidget::set_altitude (Feet altitude)
{
	_altitude = altitude;
	update();
}


inline void
EFISWidget::set_altitude_visible (bool visible)
{
	_altitude_visible = visible;
	update();
}


inline void
EFISWidget::set_altitude_tendency (Feet ft)
{
	_altitude_tendency = ft;
	update();
}


inline void
EFISWidget::set_altitude_tendency_visible (bool visible)
{
	_altitude_tendency_visible = visible;
	update();
}


inline void
EFISWidget::set_altitude_agl (Feet altitude)
{
	_altitude_agl = altitude;
	update();
}


inline void
EFISWidget::set_altitude_agl_visible (bool visible)
{
	_altitude_agl_visible = visible;
	update();
}


inline Feet
EFISWidget::landing_altitude() const
{
	return _landing_altitude;
}


inline void
EFISWidget::set_landing_altitude (Feet feet)
{
	_landing_altitude = feet;
	update();
}


inline void
EFISWidget::set_landing_altitude_visible (bool visible)
{
	_landing_altitude_visible = visible;
	update();
}


inline Feet
EFISWidget::transition_altitude() const
{
	return _transition_altitude;
}


inline void
EFISWidget::set_transition_altitude (Feet transition_altitude)
{
	_transition_altitude = transition_altitude;
	update();
}


inline void
EFISWidget::set_transition_altitude_visible (bool visible)
{
	_transition_altitude_visible = visible;
	update();
}


inline FeetPerMinute
EFISWidget::climb_rate() const
{
	return _climb_rate;
}


inline void
EFISWidget::set_climb_rate (FeetPerMinute feet_per_minute)
{
	_climb_rate = feet_per_minute;
	update();
}


inline void
EFISWidget::set_climb_rate_visible (bool visible)
{
	_climb_rate_visible = visible;
	update();
}


inline Knots
EFISWidget::speed_bug (QString name) const
{
	auto it = _speed_bugs.find (name);
	if (it != _speed_bugs.end())
		return it->second;
	return 0.f;
}


inline void
EFISWidget::add_speed_bug (QString name, Knots speed)
{
	_speed_bugs[name] = speed;
	update();
}


inline void
EFISWidget::remove_speed_bug (QString name)
{
	if (name.isNull())
		_speed_bugs.clear();
	else
		_speed_bugs.erase (name);
	update();
}


inline Feet
EFISWidget::altitude_bug (QString name) const
{
	auto it = _altitude_bugs.find (name);
	if (it != _altitude_bugs.end())
		return it->second;
	return 0.f;
}


inline void
EFISWidget::add_altitude_bug (QString name, Feet altitude)
{
	_altitude_bugs[name] = altitude;
	update();
}


inline void
EFISWidget::remove_altitude_bug (QString name)
{
	if (name.isNull())
		_altitude_bugs.clear();
	else
		_altitude_bugs.erase (name);
	update();
}


inline float
EFISWidget::mach() const
{
	return _mach;
}


inline void
EFISWidget::set_mach (float value)
{
	_mach = value;
	update();
}


inline void
EFISWidget::set_mach_visible (bool visible)
{
	_mach_visible = visible;
	update();
}


inline Pressure
EFISWidget::pressure() const
{
	return _pressure;
}


inline void
EFISWidget::set_pressure (Pressure pressure)
{
	_pressure = pressure;
	update();
}


inline void
EFISWidget::set_pressure_display_hpa (bool hpa)
{
	_pressure_display_hpa = hpa;
	update();
}


inline void
EFISWidget::set_pressure_visible (bool visible)
{
	_pressure_visible = visible;
	update();
}


inline void
EFISWidget::set_standard_pressure (bool standard)
{
	_standard_pressure = standard;
	update();
}


inline Knots
EFISWidget::minimum_speed() const
{
	return _minimum_speed;
}


inline void
EFISWidget::set_minimum_speed (Knots minimum_speed)
{
	_minimum_speed = minimum_speed;
	update();
}


inline void
EFISWidget::set_minimum_speed_visible (bool visible)
{
	_minimum_speed_visible = visible;
	update();
}


inline Knots
EFISWidget::warning_speed() const
{
	return _warning_speed;
}


inline void
EFISWidget::set_warning_speed (Knots warning_speed)
{
	_warning_speed = warning_speed;
	update();
}


inline void
EFISWidget::set_warning_speed_visible (bool visible)
{
	_warning_speed_visible = visible;
	update();
}


inline Knots
EFISWidget::maximum_speed() const
{
	return _maximum_speed;
}


inline void
EFISWidget::set_maximum_speed (Knots maximum_speed)
{
	_maximum_speed = maximum_speed;
	update();
}


inline void
EFISWidget::set_maximum_speed_visible (bool visible)
{
	_maximum_speed_visible = visible;
	update();
}


inline Feet
EFISWidget::ap_altitude() const
{
	return _ap_altitude;
}


inline void
EFISWidget::set_ap_altitude (Feet feet)
{
	_ap_altitude = feet;
	update();
}


inline void
EFISWidget::set_ap_altitude_visible (bool visible)
{
	_ap_altitude_visible = visible;
	update();
}


inline FeetPerMinute
EFISWidget::ap_climb_rate() const
{
	return _ap_climb_rate;
}


inline void
EFISWidget::set_ap_climb_rate (FeetPerMinute fpm)
{
	_ap_climb_rate = fpm;
	update();
}


inline void
EFISWidget::set_ap_climb_rate_visible (bool visible)
{
	_ap_climb_rate_visible = visible;
	update();
}


inline Knots
EFISWidget::at_speed() const
{
	return _at_speed;
}


inline void
EFISWidget::set_at_speed (Knots knots)
{
	_at_speed = knots;
	update();
}


inline void
EFISWidget::set_at_speed_visible (bool visible)
{
	_at_speed_visible = visible;
	update();
}


inline Angle
EFISWidget::flight_director_pitch() const
{
	return _flight_director_pitch;
}


inline void
EFISWidget::set_flight_director_pitch (Angle pitch)
{
	_flight_director_pitch = pitch;
	update();
}


inline void
EFISWidget::set_flight_director_pitch_visible (bool visible)
{
	_flight_director_pitch_visible = visible;
	update();
}


inline Angle
EFISWidget::flight_director_roll() const
{
	return _flight_director_roll;
}


inline void
EFISWidget::set_flight_director_roll (Angle roll)
{
	_flight_director_roll = roll;
	update();
}


inline void
EFISWidget::set_flight_director_roll_visible (bool visible)
{
	_flight_director_roll_visible = visible;
	update();
}


inline Angle
EFISWidget::control_stick_pitch() const
{
	return _control_stick_pitch;
}


inline void
EFISWidget::set_control_stick_pitch (Angle pitch)
{
	_control_stick_pitch = pitch;
	update();
}


inline Angle
EFISWidget::control_stick_roll() const
{
	return _control_stick_roll;
}


inline void
EFISWidget::set_control_stick_roll (Angle roll)
{
	_control_stick_roll = roll;
	update();
}


inline void
EFISWidget::set_control_stick_visible (bool visible)
{
	_control_stick_visible = visible;
	update();
}


inline void
EFISWidget::set_approach_reference_visible (bool visible)
{
	_approach_reference_visible = visible;
	update();
}


inline Angle
EFISWidget::vertical_deviation() const
{
	return _vertical_deviation_deg;
}


inline void
EFISWidget::set_vertical_deviation (Angle deviation)
{
	_vertical_deviation_deg = deviation;
	update();
}


inline void
EFISWidget::set_vertical_deviation_visible (bool visible)
{
	_vertical_deviation_visible = visible;
	update();
}


inline Angle
EFISWidget::lateral_deviation() const
{
	return _lateral_deviation_deg;
}


inline void
EFISWidget::set_lateral_deviation (Angle deviation)
{
	_lateral_deviation_deg = deviation;
	update();
}


inline void
EFISWidget::set_lateral_deviation_visible (bool visible)
{
	_lateral_deviation_visible = visible;
	update();
}


inline void
EFISWidget::set_runway_visible (bool visible)
{
	_runway_visible = visible;
	update();
}


inline QString
EFISWidget::approach_hint() const
{
	return _approach_hint;
}


inline void
EFISWidget::set_approach_hint (QString hint)
{
	_approach_hint = hint;
	update();
}


inline Length
EFISWidget::dme_distance() const
{
	return _dme_distance;
}


inline void
EFISWidget::set_dme_distance (Length distance)
{
	_dme_distance = distance;
	update();
}


inline void
EFISWidget::set_dme_distance_visible (bool visible)
{
	_dme_distance_visible = visible;
	update();
}


inline QString
EFISWidget::localizer_id() const
{
	return _localizer_id;
}


inline void
EFISWidget::set_localizer_id (QString const& loc_id)
{
	_localizer_id = loc_id;
	update();
}


inline Angle
EFISWidget::localizer_magnetic_bearing() const
{
	return _localizer_magnetic_bearing;
}


inline void
EFISWidget::set_localizer_magnetic_bearing (Angle mag_bearing)
{
	_localizer_magnetic_bearing = mag_bearing;
	update();
}


inline void
EFISWidget::set_localizer_info_visible (bool visible)
{
	_localizer_info_visible = visible;
	update();
}


inline QString
EFISWidget::control_hint() const
{
	return _control_hint;
}


inline void
EFISWidget::set_control_hint (QString const& hint)
{
	_control_hint = hint;
	update();
}


inline void
EFISWidget::set_control_hint_visible (bool visible)
{
	_control_hint_visible = visible;
	update();
}


inline void
EFISWidget::set_ap_hints_visible (bool visible)
{
	_ap_hints_visible = visible;
	update();
}


inline QString
EFISWidget::ap_speed_hint() const
{
	return _ap_speed_hint;
}


inline void
EFISWidget::set_ap_speed_hint (QString const& hint)
{
	_ap_speed_hint = hint;
	update();
}


inline QString
EFISWidget::ap_speed_small_hint() const
{
	return _ap_speed_small_hint;
}


inline void
EFISWidget::set_ap_speed_small_hint (QString const& hint)
{
	_ap_speed_small_hint = hint;
	update();
}


inline QString
EFISWidget::ap_lateral_hint() const
{
	return _ap_lateral_hint;
}


inline void
EFISWidget::set_ap_lateral_hint (QString const& hint)
{
	_ap_lateral_hint = hint;
	update();
}


inline QString
EFISWidget::ap_lateral_small_hint() const
{
	return _ap_lateral_small_hint;
}


inline void
EFISWidget::set_ap_lateral_small_hint (QString const& hint)
{
	_ap_lateral_small_hint = hint;
	update();
}


inline QString
EFISWidget::ap_vertical_hint() const
{
	return _ap_vertical_hint;
}


inline void
EFISWidget::set_ap_vertical_hint (QString const& hint)
{
	_ap_vertical_hint = hint;
	update();
}


inline QString
EFISWidget::ap_vertical_small_hint() const
{
	return _ap_vertical_small_hint;
}


inline void
EFISWidget::set_ap_vertical_small_hint (QString const& hint)
{
	_ap_vertical_small_hint = hint;
	update();
}


inline Angle
EFISWidget::fov() const
{
	return _fov;
}


inline void
EFISWidget::set_fov (Angle degrees)
{
	_fov = degrees;
	update();
}


inline void
EFISWidget::set_input_alert_visible (bool visible)
{
	_input_alert_visible = visible;
}


inline float
EFISWidget::kt_to_px (Knots kt) const
{
	return -0.5f * _sl_ladder_rect.height() * (kt - _speed) / (0.5f * _sl_extent);
}


inline float
EFISWidget::ft_to_px (Feet ft) const
{
	return -0.5f * _al_ladder_rect.height() * (ft - _altitude) / (0.5f * _al_extent);
}


inline void
EFISWidget::blink_speed()
{
	_speed_blink = !_speed_blink;
}


inline void
EFISWidget::blink_baro()
{
	_baro_blink = !_baro_blink;
}


inline float
EFISWidget::pitch_to_px (Angle degrees) const
{
	float const correction = 0.775f;
	return -degrees / (_fov * correction) * wh();
}


inline float
EFISWidget::heading_to_px (Angle degrees) const
{
	return pitch_to_px (-degrees);
}


inline QColor
EFISWidget::get_baro_color() const
{
	if (_baro_blinking_warning->isActive())
		return _warning_color_2;
	return _navigation_color;
}

#endif

