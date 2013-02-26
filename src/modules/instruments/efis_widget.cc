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

// Standard:
#include <cstddef>
#include <utility>
#include <cmath>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>

// Local:
#include "efis_widget.h"


EFISWidget::EFISWidget (QWidget* parent):
	InstrumentWidget (parent, 0.8f, 1.f, 1.f)
{
	setAttribute (Qt::WA_NoBackground);
	_sky_color.setHsv (213, 245, 255);
	_ground_color.setHsv (30, 255, 122);
	_ladder_color = QColor (64, 51, 108, 0x80);
	_ladder_border_color = _ladder_color.darker (120);
	_warning_color_1 = QColor (255, 150, 0);
	_warning_color_2 = QColor (255, 200, 50);

	_speed_blinking_warning = new QTimer (this);
	_speed_blinking_warning->setInterval (200);
	QObject::connect (_speed_blinking_warning, SIGNAL (timeout()), this, SLOT (blink_speed()));

	_baro_blinking_warning = new QTimer (this);
	_baro_blinking_warning->setInterval (200);
	QObject::connect (_baro_blinking_warning, SIGNAL (timeout()), this, SLOT (blink_baro()));
}


void
EFISWidget::resizeEvent (QResizeEvent* resize_event)
{
	InstrumentWidget::resizeEvent (resize_event);

	_w = width();
	_h = height();
	_max_w_h = std::max (_w, _h);
	_q = 0.1f * wh();

	_center_transform.reset();
	_center_transform.translate (0.5f * _w, 0.5f * _h);

	adi_post_resize();
	sl_post_resize();
	al_post_resize();
}


void
EFISWidget::paintEvent (QPaintEvent*)
{
	_current_datetime = QDateTime::currentDateTime();

	update_blinker (_speed_blinking_warning,
					_speed_visible &&
					((_warning_speed_visible && _speed < _warning_speed) ||
					 (_minimum_speed_visible && _speed < _minimum_speed) ||
					 (_maximum_speed_visible && _speed > _maximum_speed)),
				    &_speed_blink);

	update_blinker (_baro_blinking_warning,
					_altitude_visible && _transition_altitude_visible &&
					((_transition_altitude > _altitude && _standard_pressure) ||
					 (_transition_altitude <= _altitude && !_standard_pressure)),
					&_baro_blink);

	QPainter painter (this);
	TextPainter text_painter (painter, &_text_painter_cache);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	if (_input_alert_visible)
		paint_input_alert (painter, text_painter);
	else
	{
		adi_paint (painter, text_painter);

		paint_center_cross (painter, false, true);
		paint_flight_director (painter);
		paint_control_stick (painter);
		paint_center_cross (painter, true, false);
		paint_altitude_agl (painter, text_painter);
		paint_baro_setting (painter, text_painter);
		paint_nav (painter, text_painter);
		paint_hints (painter, text_painter);
		paint_pitch_limit (painter);

		sl_paint (painter, text_painter);
		al_paint (painter, text_painter);
	}
}


void
EFISWidget::adi_post_resize()
{
	float const w_max = 2.f * _max_w_h;
	float const h_max = 10.f * _max_w_h;
	_adi_sky_rect = QRectF (-w_max, -h_max, 2.f * w_max, h_max + 1.f);
	_adi_gnd_rect = QRectF (-w_max, 0.f, 2.f * w_max, h_max);

	// Flight path marker:
	{
		float x = 0.013f * wh();
		float w = pen_width (3.f);
		float r = 0.5f * w;
		_flight_path_marker_clip = QPainterPath();
		_flight_path_marker_clip.setFillRule (Qt::WindingFill);
		_flight_path_marker_clip.addEllipse (QRectF (-x - 0.5f * w, -x - 0.5f * w, 2.f * x + w, 2.f * x + w));
		_flight_path_marker_clip.addRoundedRect (QRectF (-4.f * x - 0.5f * w, -0.5f * w, +3.f * x + w, w), r, r);
		_flight_path_marker_clip.addRoundedRect (QRectF (+1.f * x - 0.5f * w, -0.5f * w, +3.f * x + w, w), r, r);
		_flight_path_marker_clip.addRoundedRect (QRectF (-0.5f * w, -2.f * x - 0.5f * w, w, x + w), r, r);

		_flight_path_marker_shape = QPainterPath();
		_flight_path_marker_shape.addEllipse (QRectF (-x, -x, 2.f * x, 2.f * x));
		_flight_path_marker_shape.moveTo (QPointF (+x, 0.f));
		_flight_path_marker_shape.lineTo (QPointF (+4.f * x, 0.f));
		_flight_path_marker_shape.moveTo (QPointF (-x, 0.f));
		_flight_path_marker_shape.lineTo (QPointF (-4.f * x, 0.f));
		_flight_path_marker_shape.moveTo (QPointF (0.f, -x));
		_flight_path_marker_shape.lineTo (QPointF (0.f, -2.f * x));
	}
}


void
EFISWidget::adi_pre_paint()
{
	Angle p = floored_mod (_pitch + 180_deg, 360_deg) - 180_deg;
	Angle r = floored_mod (_roll + 180_deg, 360_deg) - 180_deg;
	Angle hdg = floored_mod (_heading, 360_deg);

	// Mirroring, eg. -180° pitch is the same
	// as 0° pitch with roll inverted:
	if (p < -90_deg)
	{
		p = -180_deg - p;
		r = +180_deg - r;
	}
	else if (p > 90_deg)
	{
		p = +180_deg - p;
		r = +180_deg - r;
	}

	_pitch = p;
	_roll = r;
	_heading = hdg;

	_pitch_transform.reset();
	_pitch_transform.translate (0.f, -pitch_to_px (p));

	_roll_transform.reset();
	_roll_transform.rotate (-r.deg());

	_heading_transform.reset();
	_heading_transform.translate (-heading_to_px (hdg), 0.f);

	// Total transform of horizon (heading is not really necessary here):
	_horizon_transform = _pitch_transform * _roll_transform * _center_transform;
	// Without the following, Qt did something weird sometimes, like aligning drawn points to display pixels (?).
	_horizon_transform.shear (0.0001f, 0.f);

	_flight_path_marker_position = QPointF (-heading_to_px (_flight_path_beta), -pitch_to_px (_flight_path_alpha));
}


void
EFISWidget::adi_paint (QPainter& painter, TextPainter& text_painter)
{
	adi_pre_paint();

	adi_paint_horizon (painter);
	adi_paint_flight_path_marker (painter);
	adi_paint_pitch (painter, text_painter);
	adi_paint_roll (painter);
	adi_paint_heading (painter, text_painter);
}


void
EFISWidget::adi_paint_horizon (QPainter& painter)
{
	if (_pitch_visible && _roll_visible)
	{
		painter.setClipping (false);
		painter.setTransform (_horizon_transform);
		painter.fillRect (_adi_sky_rect, _sky_color);
		painter.fillRect (_adi_gnd_rect, _ground_color);
	}
	else
	{
		painter.setClipping (false);
		painter.resetTransform();
		painter.setPen (Qt::NoPen);
		painter.setBrush (Qt::black);
		painter.drawRect (rect());
	}
}


void
EFISWidget::adi_paint_pitch (QPainter& painter, TextPainter& text_painter)
{
	if (!_pitch_visible)
		return;

	float const w = wh() * 0.22222f; // 0.(2) == 2/9
	float const z = 0.5f * w;
	float const fpxs = _font_10_bold.pixelSize();

	// Clip rectangle before and after rotation:
	painter.setTransform (_center_transform);
	painter.setClipPath (get_pitch_scale_clipping_path());
	painter.setTransform (_roll_transform * _center_transform);
	painter.setClipRect (QRectF (-w, -0.9f * w, 2.f * w, 2.2f * w), Qt::IntersectClip);
	painter.setTransform (_horizon_transform);
	painter.setFont (_font_10_bold);

	// Pitch scale is clipped to small rectangle, so narrow it even more:
	float clipped_pitch_factor = 0.45f;
	Range<Angle> deg_range (_pitch - clipped_pitch_factor * 0.5f * _fov,
							_pitch + clipped_pitch_factor * 0.5f * _fov);

	painter.setPen (get_pen (Qt::white, 1.f));
	// 10° lines, exclude +/-90°:
	for (int deg = -90; deg <= 90; deg += 10)
	{
		if (!deg_range.includes (1_deg * deg) || deg == 0)
			continue;
		float d = pitch_to_px (1_deg * deg);
		painter.drawLine (QPointF (-z, d), QPointF (z, d));
		// Degs number:
		int abs_deg = std::abs (deg);
		QString deg_t = QString::number (abs_deg > 90 ? 180 - abs_deg : abs_deg);
		// Text:
		QRectF lbox (-z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		QRectF rbox (+z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		text_painter.drawText (lbox, Qt::AlignVCenter | Qt::AlignRight, deg_t);
		text_painter.drawText (rbox, Qt::AlignVCenter | Qt::AlignLeft, deg_t);
	}
	// 5° lines:
	for (int deg = -90; deg <= 90; deg += 5)
	{
		if (!deg_range.includes (1_deg * deg) || deg % 10 == 0)
			continue;
		float d = pitch_to_px (1_deg * deg);
		painter.drawLine (QPointF (-z / 2.f, d), QPointF (z / 2.f, d));
	}
	// 2.5° lines:
	for (int deg = -900; deg <= 900; deg += 25)
	{
		if (!deg_range.includes (1_deg * deg / 10) || deg % 50 == 0)
			continue;
		float d = pitch_to_px (1_deg * deg / 10.f);
		painter.drawLine (QPointF (-z / 4.f, d), QPointF (z / 4.f, d));
	}

	painter.setPen (get_pen (Qt::white, 1.75f));
	// -90°, 90° lines:
	if (deg_range.includes (-90_deg) || deg_range.includes (+90_deg))
	{
		for (float deg: { -90.f, 90.f })
		{
			float d = pitch_to_px (1_deg * deg);
			painter.drawLine (QPointF (-z, d), QPointF (z, d));
		}
	}
}


void
EFISWidget::adi_paint_roll (QPainter& painter)
{
	float const w = wh() * 3.f / 9.f;
	bool const bank_angle_warning = _roll_limit > 0_deg && std::abs (_roll.deg()) > _roll_limit.deg();
	bool const slip_skid_warning = _slip_skid_limit > 0.f && std::abs (_slip_skid) > _slip_skid_limit;

	QPen pen = get_pen (Qt::white, 1.f);
	painter.setPen (pen);
	painter.setBrush (QBrush (Qt::white));

	QPen warning_pen = pen;
	warning_pen.setColor (_warning_color_2);

	painter.setTransform (_center_transform);
	painter.setClipRect (QRectF (-w, -w, 2.f * w, 2.25f * w));
	for (float deg: { -60.f, -45.f, -30.f, -20.f, -10.f, 0.f, +10.f, +20.f, +30.f, +45.f, +60.f })
	{
		painter.setTransform (_center_transform);
		painter.rotate (1.f * deg);
		painter.translate (0.f, -0.795f * w);

		if (deg == 0.f)
		{
			// Triangle:
			QPointF p0 (0.f, 0.f);
			QPointF px (0.025f * w, 0.f);
			QPointF py (0.f, 0.05f * w);
			painter.drawPolygon (QPolygonF() << p0 << p0 - px - py << p0 + px - py);
		}
		else
		{
			float length = -0.05f * w;
			if (std::abs (std::fmod (deg, 60.f)) < 1.f)
				length *= 1.6f;
			else if (std::abs (std::fmod (deg, 30.f)) < 1.f)
				length *= 2.2f;
			painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, length));
		}
	}

	if (!_roll_visible)
		return;

	float const bold_width = pen_width (3.f);
	QPointF a (0, 0.01f * w); // Miter
	QPointF b (-0.062f * w, 0.1f * w);
	QPointF c (+0.062f * w, 0.1f * w);
	QPointF x0 (0.002f * w, 0.f);
	QPointF y0 (0.f, 0.005f * w);
	QPointF y1 (0.f, 1.f * bold_width);

	painter.setTransform (_roll_transform * _center_transform);
	painter.translate (0.f, -0.79f * w);

	QPolygonF bank_angle_polygon = QPolygonF() << b << a << c << b;

	if (bank_angle_warning)
	{
		painter.setPen (warning_pen);
		painter.setBrush (warning_pen.color());
		painter.drawPolygon (bank_angle_polygon);
	}
	else
	{
		painter.setPen (pen);
		painter.drawPolyline (bank_angle_polygon);
	}

	if (_slip_skid_visible)
	{
		QPolygonF slip_skid_polygon = QPolygonF()
			<< b - x0 + y0
			<< b - x0 + y1
			<< c + x0 + y1
			<< c + x0 + y0
			<< b - x0 + y0;

		painter.translate (-limit (_slip_skid, -4.f, +4.f) * 0.08f * w, 0.f);

		if (bank_angle_warning || slip_skid_warning)
			painter.setPen (warning_pen);
		else
			painter.setPen (pen);

		if (slip_skid_warning)
		{
			painter.setBrush (warning_pen.color());
			painter.drawPolygon (slip_skid_polygon);
		}
		else
			painter.drawPolyline (slip_skid_polygon);
	}
}


void
EFISWidget::adi_paint_heading (QPainter& painter, TextPainter& text_painter)
{
	float const w = wh() * 2.25f / 9.f;
	float const fpxs = _font_10_bold.pixelSize();

	if (!_pitch_visible || !_roll_visible)
		return;

	// Clip rectangle before and after rotation:
	painter.setTransform (_center_transform);
	painter.setClipPath (get_pitch_scale_clipping_path());
	painter.setTransform (_roll_transform * _center_transform);
	painter.setClipRect (QRectF (-1.1f * w, -0.8f * w, 2.2f * w, 1.9f * w), Qt::IntersectClip);

	painter.setTransform (_horizon_transform);
	painter.setPen (get_pen (Qt::white, 1.25f));
	painter.drawLine (QPointF (-1.25 * w, 0.f), QPointF (1.25f * w, 0.f));

	QPen p = get_pen (Qt::white, 1.f);
	p.setCapStyle (Qt::FlatCap);
	painter.setPen (p);
	painter.setFont (_font_10_bold);

	if (!_heading_visible)
		return;

	float clipped_pitch_factor = 0.5f;
	Range<Angle> deg_range (_heading - clipped_pitch_factor * 0.5f * _fov,
							_heading + clipped_pitch_factor * 0.5f * _fov);

	painter.setTransform (_heading_transform * _horizon_transform);
	for (int deg = -180; deg < 540; deg += 10)
	{
		if (!deg_range.includes (1_deg * deg))
			continue;

		float d10 = heading_to_px (1_deg * deg);
		float d05 = heading_to_px (1_deg * deg + 5_deg);
		// 10° lines:
		painter.drawLine (QPointF (d10, -w / 18.f), QPointF (d10, 0.f));
		if (_heading_numbers_visible)
		{
			QString text = QString::number (floored_mod (1.f * deg, 360.f) / 10);
			if (text == "0")
				text = "N";
			else if (text == "9")
				text = "E";
			else if (text == "18")
				text = "S";
			else if (text == "27")
				text = "W";
			text_painter.drawText (QRectF (d10 - 2.f * fpxs, 0.05f * fpxs, 4.f * fpxs, fpxs),
								   Qt::AlignVCenter | Qt::AlignHCenter, text);
		}
		// 5° lines:
		painter.drawLine (QPointF (d05, -w / 36.f), QPointF (d05, 0.f));
	}
}


void
EFISWidget::adi_paint_flight_path_marker (QPainter& painter)
{
	if (!_flight_path_visible)
		return;

	painter.setTransform (_center_transform);
	painter.setClipRect (QRectF (-0.325f * wh(), -0.4f * wh(), 0.65f * wh(), 0.8f * wh()));
	painter.translate (_flight_path_marker_position);
	painter.setPen (get_pen (Qt::white, 1.25f));
	painter.drawPath (_flight_path_marker_shape);
}


void
EFISWidget::sl_post_resize()
{
	float const wh = this->wh();

	_speed = limit (_speed, 0.f, 9999.99f);
	_mach = limit (_mach, 0.f, 9.99f);
	_minimum_speed = limit (_minimum_speed, 0.f, 9999.99f);
	_warning_speed = limit (_warning_speed, 0.f, 9999.99f);
	_maximum_speed = limit (_maximum_speed, 0.f, 9999.99f);

	_sl_ladder_rect = QRectF (-0.0675f * wh, -0.375 * wh, 0.135 * wh, 0.75f * wh);
	_sl_ladder_pen = QPen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_sl_black_box_pen = get_pen (Qt::white, 1.2f);
	_sl_scale_pen = get_pen (Qt::white, 1.f);
	_sl_speed_bug_pen = get_pen (Qt::green, 1.5f);

	QFont actual_speed_font = _font_20_bold;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;
	_sl_margin = 0.25f * digit_width;
	_sl_digits = _speed >= 1000.0f - 0.5f ? 4 : 3;

	_sl_black_box_rect = QRectF (-_sl_digits * digit_width - 2.f * _sl_margin, -digit_height,
								 +_sl_digits * digit_width + 2.f * _sl_margin, 2.f * digit_height);

	_sl_transform = _center_transform;
	_sl_transform.translate (-0.4f * wh, 0.f);
}


void
EFISWidget::sl_pre_paint()
{
	_speed = limit<float> (_speed, _sl_minimum, _sl_maximum);
	_sl_min_shown = _speed - 0.5f * _sl_extent;
	_sl_max_shown = _speed + 0.5f * _sl_extent;
	_sl_min_shown = std::max<float> (_sl_min_shown, _sl_minimum);
	_sl_max_shown = std::min<float> (_sl_max_shown, _sl_maximum);
	if (_sl_min_shown < 0.f)
		_sl_min_shown = 0.f;
	_sl_rounded_speed = static_cast<int> (_speed + 0.5f);
}


void
EFISWidget::sl_paint (QPainter& painter, TextPainter& text_painter)
{
	sl_pre_paint();

	float const x = _sl_ladder_rect.width() / 4.0f;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.setPen (_sl_ladder_pen);
	painter.setBrush (_ladder_color);
	painter.drawRect (_sl_ladder_rect);

	sl_paint_ladder_scale (painter, text_painter, x);
	sl_paint_speed_limits (painter, x);
	sl_paint_bugs (painter, text_painter, x);
	sl_paint_speed_tendency (painter, x);
	sl_paint_black_box (painter, text_painter, x);
	sl_paint_mach_number (painter, text_painter, x);
	sl_paint_ap_setting (painter, text_painter);
}


void
EFISWidget::sl_paint_black_box (QPainter& painter, TextPainter& text_painter, float x)
{
	if (!_speed_visible)
		return;

	QFont actual_speed_font = _font_20_bold;
	float const digit_width = _font_20_digit_width;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.translate (+0.75f * x, 0.f);

	QPen border_pen = get_pen (Qt::white, 1.2f);
	if (_speed_blinking_warning->isActive())
	{
		border_pen.setColor (_speed_blink || (_speed < _minimum_speed)
								? _warning_color_1
								: Qt::black);
	}

	painter.setPen (border_pen);
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawPolygon (QPolygonF()
		<< QPointF (+0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _sl_black_box_rect.topRight()
		<< _sl_black_box_rect.topLeft()
		<< _sl_black_box_rect.bottomLeft()
		<< _sl_black_box_rect.bottomRight()
		<< QPointF (0.f, +0.5f * x));

	QRectF box_1000 = _sl_black_box_rect.adjusted (_sl_margin, _sl_margin, -_sl_margin, -_sl_margin);
	QRectF box_0100 =
		_sl_digits == 3
			? box_1000
			: box_1000.adjusted (digit_width, 0.f, 0.f, 0.f);
	QRectF box_0010 = box_0100.adjusted (digit_width, 0.f, 0.f, 0.f);
	QRectF box_0001 = box_0010.adjusted (digit_width, 0.f, 0.f, 0.f);

	painter.setPen (QPen (Qt::white, 1.f));
	painter.setFont (actual_speed_font);
	if (_sl_digits == 4)
		paint_rotating_digit (painter, text_painter, box_1000, _speed, 1000, 1.25f, 0.0005f, 0.5f, false, true);
	paint_rotating_digit (painter, text_painter, box_0100, _speed, 100, 1.25f, 0.005f, 0.5f, false, true, true);
	paint_rotating_digit (painter, text_painter, box_0010, _speed, 10, 1.25f, 0.05f, 0.5f, false, false);
	float pos_0001 = _sl_rounded_speed - _speed;
	paint_rotating_value (painter, text_painter, box_0001, pos_0001, 0.7f,
						  QString::number (static_cast<int> (std::abs (std::fmod (1.f * _sl_rounded_speed + 1.f, 10.f)))),
						  QString::number (static_cast<int> (std::abs (std::fmod (1.f * _sl_rounded_speed, 10.f)))),
							 _speed > 0.5f
								? QString::number (static_cast<int> (floored_mod (1.f * _sl_rounded_speed - 1.f, 10.f)))
								: " ");
}


void
EFISWidget::sl_paint_ladder_scale (QPainter& painter, TextPainter& text_painter, float x)
{
	if (!_speed_visible)
		return;

	QFont ladder_font = _font_13_bold;
	float const ladder_digit_width = _font_13_digit_width;
	float const ladder_digit_height = _font_13_digit_height;

	painter.setFont (ladder_font);

	// Special clipping that leaves some margin around black indicator:
	QPainterPath clip_path_m;
	clip_path_m.addRect (_sl_black_box_rect.translated (x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_sl_ladder_rect);
	clip_path -= clip_path_m;

	painter.setTransform (_sl_transform);
	painter.setClipPath (clip_path, Qt::IntersectClip);
	painter.translate (2.f * x, 0.f);

	painter.setPen (_sl_scale_pen);
	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int kt = (static_cast<int> (_sl_min_shown) / _sl_line_every) * _sl_line_every - _sl_line_every;
		 kt <= _sl_max_shown + _sl_line_every;
		 kt += _sl_line_every)
	{
		if (kt < _sl_minimum || kt > _sl_maximum)
			continue;
		float posy = kt_to_px (kt);
		painter.drawLine (QPointF (-0.8f * x, posy), QPointF (0.f, posy));

		if ((kt - _sl_minimum) % _sl_number_every == 0)
			text_painter.drawText (QRectF (-4.f * ladder_digit_width - 1.25f * x, -0.5f * ladder_digit_height + posy,
										   +4.f * ladder_digit_width, ladder_digit_height),
								   Qt::AlignVCenter | Qt::AlignRight, QString::number (kt));
	}
}


void
EFISWidget::sl_paint_speed_limits (QPainter& painter, float x)
{
	if (!_speed_visible)
		return;

	QPointF ydif (0.f, pen_width (0.25f));
	QPen pen_b (QColor (0, 0, 0), pen_width (10.f), Qt::SolidLine, Qt::FlatCap);
	QPen pen_r (QColor (255, 0, 0), pen_width (10.f), Qt::DotLine, Qt::FlatCap);
	QPen pen_y (QColor (255, 170, 0), pen_width (1.2f), Qt::SolidLine, Qt::FlatCap);
	pen_r.setDashPattern (QVector<qreal> (2, 0.5f));

	float tr_right = 0.45f * x;
	float p1w = 0.45f * pen_width (1.2f);

	painter.setTransform (_sl_transform);
	painter.translate (tr_right, 0.f);
	painter.setClipRect (_sl_ladder_rect.adjusted (0.f, -ydif.y(), 0.f, ydif.y()));

	float max_posy = kt_to_px (_maximum_speed);
	float wrn_posy = kt_to_px (_warning_speed);
	float min_posy = kt_to_px (_minimum_speed);
	QPointF zero_point (_sl_ladder_rect.right(), _sl_ladder_rect.bottom() + ydif.y());

	if (_maximum_speed_visible && _maximum_speed < _sl_max_shown)
	{
		painter.setPen (pen_b);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), max_posy), _sl_ladder_rect.topRight() - ydif);
		painter.setPen (pen_r);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), max_posy), _sl_ladder_rect.topRight() - ydif);
	}

	if (_warning_speed_visible && _warning_speed > _sl_min_shown)
	{
		painter.setPen (pen_y);
		painter.drawPolyline (QPolygonF()
			<< QPointF (_sl_ladder_rect.right() - tr_right, wrn_posy)
			<< QPointF (_sl_ladder_rect.right() - p1w, wrn_posy)
			<< zero_point - QPointF (p1w, 0.f));
	}

	if (_minimum_speed_visible && _minimum_speed > _sl_min_shown)
	{
		painter.setPen (pen_b);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), min_posy), zero_point);
		painter.setPen (pen_r);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), min_posy), zero_point);
	}
}


void
EFISWidget::sl_paint_speed_tendency (QPainter& painter, float x)
{
	if (!_speed_tendency_visible || !_speed_visible)
		return;

	QPen pen (get_pen (_navigation_color, 1.25f));
	pen.setCapStyle (Qt::RoundCap);
	pen.setJoinStyle (Qt::RoundJoin);

	painter.setTransform (_sl_transform);
	painter.setPen (pen);
	painter.translate (1.2f * x, 0.f);
	if (_speed_tendency < _speed)
		painter.scale (1.f, -1.f);
	float length = std::min<float> (_sl_ladder_rect.height() / 2.f, 1.f * std::abs (kt_to_px (limit<float> (_speed_tendency, _sl_minimum, _sl_maximum)))) - 0.5f * x;

	if (length > 0.2f * x)
	{
		painter.setClipRect (QRectF (_sl_ladder_rect.topLeft(), QPointF (_sl_ladder_rect.right(), 0.f)));
		painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -length));
		painter.translate (0.f, -length);
		painter.drawPolygon (QPolygonF()
			<< QPointF (0.f, -0.5f * x)
			<< QPointF (-0.2f * x, 0.f)
			<< QPointF (+0.2f * x, 0.f));
	}
}


void
EFISWidget::sl_paint_bugs (QPainter& painter, TextPainter& text_painter, float x)
{
	if (!_speed_visible)
		return;

	QFont speed_bug_font = _font_10_bold;
	float const speed_bug_digit_height = _font_10_digit_height;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.setFont (speed_bug_font);

	for (auto& bug: _speed_bugs)
	{
		if (bug.second > _sl_min_shown && bug.second < _sl_max_shown)
		{
			float posy = kt_to_px (bug.second);
			painter.setPen (_sl_speed_bug_pen);
			painter.setClipRect (_sl_ladder_rect.translated (x, 0.f));
			painter.drawLine (QPointF (1.5f * x, posy), QPointF (2.25f * x, posy));
			painter.setClipping (false);
			text_painter.drawText (QRectF (2.5f * x, posy - 0.5f * speed_bug_digit_height,
										   2.f * x, speed_bug_digit_height),
								   Qt::AlignVCenter | Qt::AlignLeft, bug.first);
		}
	}

	// AT bug:
	if (_cmd_speed_visible)
	{
		float posy = limit (kt_to_px (limit<float> (_cmd_speed, _sl_minimum, _sl_maximum)),
							static_cast<float> (-_sl_ladder_rect.height() / 2.f), static_cast<float> (_sl_ladder_rect.height() / 2.f));
		// TODO extract bug_shape to sl_post_resize()
		QPolygonF bug_shape = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.5f * x, -0.5f * x)
			<< QPointF (2.f * x, -0.5f * x)
			<< QPointF (2.f * x, +0.5f * x)
			<< QPointF (+0.5f * x, +0.5f * x);
		painter.setClipRect (_sl_ladder_rect.translated (2.5f * x, 0.f));
		painter.translate (1.25f * x, posy);
		painter.setBrush (Qt::NoBrush);
		painter.setPen (_autopilot_pen_1);
		painter.drawPolygon (bug_shape);
		painter.setPen (_autopilot_pen_2);
		painter.drawPolygon (bug_shape);
	}
}


void
EFISWidget::sl_paint_mach_number (QPainter& painter, TextPainter& text_painter, float x)
{
	if (!_mach_visible)
		return;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.translate (0.f, 0.75f * x);

	QFont font_a = _font_16_bold;
	QFont font_b = _font_10_bold;

	QString m_str = "M";
	QString mach_str = " " + QString ("%1").arg (_mach, 0, 'f', 3);

	QRectF nn_rect (0.f, _sl_ladder_rect.bottom(), QFontMetricsF (font_a).width (mach_str), 1.2f * _font_16_digit_height);
	QRectF zz_rect (0.f, nn_rect.top(), QFontMetricsF (font_b).width (m_str), nn_rect.height());
	zz_rect.moveLeft (-0.5f * (zz_rect.width() + nn_rect.width()));
	// Correct position of zz_rect to get correct baseline position:
	zz_rect.translate (0.f, QFontMetricsF (font_b).descent() - QFontMetricsF (font_a).descent());
	nn_rect.moveLeft (zz_rect.right());

	painter.setPen (get_pen (Qt::white, 1.f));
	painter.setFont (font_a);
	text_painter.drawText (nn_rect, Qt::AlignBottom | Qt::AlignLeft, mach_str);
	painter.setFont (font_b);
	text_painter.drawText (zz_rect, Qt::AlignBottom | Qt::AlignRight, m_str);
}


void
EFISWidget::sl_paint_ap_setting (QPainter& painter, TextPainter& text_painter)
{
	if (!_cmd_speed_visible)
		return;

	QFont actual_speed_font = _font_20_bold;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;

	int digits = 4;
	float const margin = 0.2f * digit_width;

	QRectF digits_box (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	QRectF box_rect (_sl_ladder_rect.right() - digits_box.width(), _sl_ladder_rect.top() - 1.4f * digits_box.height(),
					 digits_box.width(), digits_box.height());

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.setPen (get_pen (QColor (0, 0, 0), 0.5f));
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (box_rect);

	painter.setPen (get_pen (_autopilot_color, 1.f));
	painter.setFont (actual_speed_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	text_painter.drawText (box, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (_cmd_speed))));
}


void
EFISWidget::al_post_resize()
{
	float const wh = this->wh();

	_al_ladder_rect = QRectF (-0.0675f * wh, -0.375 * wh, 0.135 * wh, 0.75f * wh);
	_al_ladder_pen = QPen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_al_black_box_pen = get_pen (Qt::white, 1.2f);
	_al_scale_pen_1 = get_pen (Qt::white, 1.f);
	_al_scale_pen_2 = get_pen (Qt::white, 3.f);
	_al_altitude_bug_pen = get_pen (QColor (0, 255, 0), 1.5f);
	_al_ldg_alt_pen = get_pen (QColor (255, 220, 0), 1.5f);
	_al_ldg_alt_pen.setCapStyle (Qt::RoundCap);

	float const b_digit_width = _font_20_digit_width;
	float const b_digit_height = _font_20_digit_height;
	float const s_digit_width = _font_16_digit_width;
	int const b_digits = 2;
	int const s_digits = 3;
	_al_margin = 0.25f * b_digit_width;

	_al_b_digits_box = QRectF (0.f, 0.f, b_digits * b_digit_width, 2.f * b_digit_height - 2.f * _al_margin);
	_al_s_digits_box = QRectF (0.f, 0.f, s_digits * s_digit_width, 2.f * b_digit_height - 2.f * _al_margin);
	_al_black_box_rect = QRectF (0.f, -0.5f * _al_b_digits_box.height() - _al_margin,
								 _al_b_digits_box.width() + _al_s_digits_box.width() + 2.f * _al_margin, _al_b_digits_box.height() + 2.f * _al_margin);
	_al_b_digits_box.translate (_al_margin, -0.5f * _al_b_digits_box.height());
	_al_s_digits_box.translate (_al_margin + _al_b_digits_box.width(), -0.5f * _al_s_digits_box.height());
}


void
EFISWidget::al_pre_paint()
{
	_altitude = limit (_altitude, -99999.f, +99999.f);
	_climb_rate = limit (_climb_rate, -9999.f, +9999.f);

	float sgn = _altitude < 0.f ? -1.f : 1.f;
	_al_min_shown = _altitude - 0.5f * _al_extent;
	_al_max_shown = _altitude + 0.5f * _al_extent;
	_al_rounded_altitude = static_cast<int> (_altitude + sgn * 10.f) / 20 * 20;

	_al_transform = _center_transform;
	_al_transform.translate (+0.4f * wh(), 0.f);
}


void
EFISWidget::al_paint (QPainter& painter, TextPainter& text_painter)
{
	al_pre_paint();

	float const x = _al_ladder_rect.width() / 4.0f;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.setPen (_al_ladder_pen);
	painter.setBrush (_ladder_color);
	painter.drawRect (_al_ladder_rect);

	al_paint_ladder_scale (painter, text_painter, x);
	al_paint_climb_rate (painter, text_painter, x);
	al_paint_bugs (painter, text_painter, x);
	al_paint_altitude_tendency (painter, x);
	al_paint_black_box (painter, text_painter, x);
	al_paint_pressure (painter, text_painter, x);
	al_paint_ap_setting (painter, text_painter);
}


void
EFISWidget::al_paint_black_box (QPainter& painter, TextPainter& text_painter, float x)
{
	QFont b_font = _font_20_bold;
	float const b_digit_width = _font_20_digit_width;
	float const b_digit_height = _font_20_digit_height;

	QFont s_font = _font_16_bold;
	float const s_digit_width = _font_16_digit_width;
	float const s_digit_height = _font_16_digit_height;

	if (!_altitude_visible)
		return;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.translate (-0.75f * x, 0.f);

	painter.setPen (_al_black_box_pen);
	painter.setBrush (Qt::black);
	// TODO extract this polygon
	painter.drawPolygon (QPolygonF()
		<< QPointF (-0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _al_black_box_rect.topLeft()
		<< _al_black_box_rect.topRight()
		<< _al_black_box_rect.bottomRight()
		<< _al_black_box_rect.bottomLeft()
		<< QPointF (0.f, +0.5f * x));

	QRectF box_10000 = QRectF (_al_b_digits_box.topLeft(), QSizeF (b_digit_width, _al_b_digits_box.height()));
	QRectF box_01000 = box_10000.translated (b_digit_width, 0.f);
	QRectF box_00100 = QRectF (_al_s_digits_box.topLeft(), QSizeF (s_digit_width, _al_b_digits_box.height()));
	QRectF box_00011 = box_00100.translated (s_digit_width, 0.f).adjusted (0.f, 0.f, s_digit_width, 0.f);

	// 11100 part:
	painter.setFont (b_font);
	paint_rotating_digit (painter, text_painter, box_10000, _altitude, 10000, 1.25f * s_digit_height / b_digit_height, 0.0005f, 5.f, true, true);
	paint_rotating_digit (painter, text_painter, box_01000, _altitude, 1000, 1.25f * s_digit_height / b_digit_height, 0.005f, 5.f, false, false);
	painter.setFont (s_font);
	paint_rotating_digit (painter, text_painter, box_00100, _altitude, 100, 1.25f, 0.05f, 5.f, false, false);

	// 00011 part:
	float pos_00011 = (_al_rounded_altitude - _altitude) / 20.f;
	paint_rotating_value (painter, text_painter, box_00011, pos_00011, 0.7f,
						  QString::number (static_cast<int> (std::abs (std::fmod (_al_rounded_altitude / 10.f + 2.f, 10.f)))) + "0",
						  QString::number (static_cast<int> (std::abs (std::fmod (_al_rounded_altitude / 10.f + 0.f, 10.f)))) + "0",
						  QString::number (static_cast<int> (std::abs (std::fmod (_al_rounded_altitude / 10.f - 2.f, 10.f)))) + "0");
}


void
EFISWidget::al_paint_ladder_scale (QPainter& painter, TextPainter& text_painter, float x)
{
	if (!_altitude_visible)
		return;

	QFont b_ladder_font = _font_13_bold;
	float const b_ladder_digit_width = _font_13_digit_width;
	float const b_ladder_digit_height = _font_13_digit_height;

	QFont s_ladder_font = _font_10_bold;
	float const s_ladder_digit_width = _font_10_digit_width;
	float const s_ladder_digit_height = _font_10_digit_height;

	// Special clipping that leaves some margin around black indicator:
	// TODO extract this path
	QPainterPath clip_path_m;
	clip_path_m.addRect (_al_black_box_rect.translated (-x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_al_ladder_rect);
	clip_path -= clip_path_m;

	painter.setTransform (_al_transform);
	painter.setClipPath (clip_path, Qt::IntersectClip);
	painter.translate (-2.f * x, 0.f);

	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int ft = (static_cast<int> (_al_min_shown) / _al_line_every) * _al_line_every - _al_line_every;
		 ft <= _al_max_shown + _al_line_every;
		 ft += _al_line_every)
	{
		if (ft >  100000.f)
			continue;

		float posy = ft_to_px (ft);

		painter.setPen (ft % _al_bold_every == 0 ? _al_scale_pen_2 : _al_scale_pen_1);
		painter.drawLine (QPointF (0.f, posy), QPointF (0.8f * x, posy));

		// TODO extract painting text to separate loop
		if (ft % _al_number_every == 0)
		{
			QRectF big_text_box (1.1f * x, -0.5f * b_ladder_digit_height + posy,
								 2.f * b_ladder_digit_width, b_ladder_digit_height);
			if (std::abs (ft) / 1000 > 0)
			{
				QString big_text = QString::number (ft / 1000);
				painter.setFont (b_ladder_font);
				text_painter.drawText (big_text_box, Qt::AlignVCenter | Qt::AlignRight, big_text);
			}

			QString small_text = QString ("%1").arg (QString::number (std::abs (ft % 1000)), 3, '0');
			if (ft == 0)
				small_text = "0";
			painter.setFont (s_ladder_font);
			QRectF small_text_box (1.1f * x + 2.1f * b_ladder_digit_width, -0.4f * s_ladder_digit_height + posy,
								   3.f * s_ladder_digit_width, s_ladder_digit_height);
			text_painter.drawText (small_text_box, Qt::AlignVCenter | Qt::AlignRight, small_text);
			// Minus sign?
			if (ft < 0)
			{
				if (ft > -1000)
					text_painter.drawText (small_text_box.adjusted (-s_ladder_digit_width, 0.f, 0.f, 0.f),
										   Qt::AlignVCenter | Qt::AlignLeft, MINUS_SIGN);
			}
		}
	}
}


void
EFISWidget::al_paint_altitude_tendency (QPainter& painter, float x)
{
	if (!_altitude_tendency_visible || !_altitude_visible)
		return;

	QPen pen (get_pen (_navigation_color, 1.25f));
	pen.setCapStyle (Qt::RoundCap);
	pen.setJoinStyle (Qt::RoundJoin);

	painter.setTransform (_al_transform);
	painter.translate (-1.2f * x, 0.f);
	painter.setPen (pen);
	if (_altitude_tendency < _altitude)
		painter.scale (1.f, -1.f);
	float length = std::min<float> (_al_ladder_rect.height() / 2.f, 1.f * std::abs (ft_to_px (_altitude_tendency))) - 0.5f * x;

	if (length > 0.2f * x)
	{
		painter.setClipRect (QRectF (_al_ladder_rect.topLeft(), QPointF (_al_ladder_rect.right(), 0.f)));
		painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -length));
		painter.translate (0.f, -length);
		painter.drawPolygon (QPolygonF()
			<< QPointF (0.f, -0.5f * x)
			<< QPointF (-0.2f * x, 0.f)
			<< QPointF (+0.2f * x, 0.f));
	}
}


void
EFISWidget::al_paint_bugs (QPainter& painter, TextPainter& text_painter, float x)
{
	if (_altitude_visible)
	{
		QFont altitude_bug_font = _font_10_bold;
		float const altitude_bug_digit_height = _font_10_digit_height;

		painter.setClipping (false);
		painter.setTransform (_al_transform);
		painter.setFont (altitude_bug_font);

		for (auto& bug: _altitude_bugs)
		{
			if (bug.second > _al_min_shown && bug.second < _al_max_shown)
			{
				float posy = ft_to_px (bug.second);
				QRectF text_rect (-4.5f * x, posy - 0.5f * altitude_bug_digit_height,
								  +2.f * x, altitude_bug_digit_height);
				painter.setClipRect (_al_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));

				painter.setPen (_al_altitude_bug_pen);
				painter.drawLine (QPointF (-1.5f * x, posy), QPointF (-2.25f * x, posy));

				painter.setClipping (false);
				text_painter.drawText (text_rect, Qt::AlignVCenter | Qt::AlignRight, bug.first);
			}
		}

		// Altitude warning:
		if (_altitude_warnings_visible)
		{
			Length base = -99999_ft;
			if (_landing_altitude_visible)
				base = std::max (base, 1_ft * _landing_altitude);
			if (_altitude_agl_visible)
				base = std::max (base, 1_ft * _altitude_agl);

			QPointF p1 (-2.05f * x, ft_to_px ((base + 500_ft).ft()));
			QPointF p2 (-2.05f * x, ft_to_px ((base + 1000_ft).ft()));

			if (_landing_altitude_visible)
				base = std::min (base, 1_ft * _landing_altitude);

			QPointF p0 (-2.05f * x, ft_to_px (base.ft()));

			QPen w = _al_ldg_alt_pen;
			w.setColor (Qt::white);
			w.setCapStyle (Qt::SquareCap);

			painter.setClipRect (_al_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));
			painter.setPen (_al_ldg_alt_pen);
			painter.drawLine (p0, p1);
			painter.setPen (w);
			painter.drawPolyline (QPolygonF() << p1 << p2 << p2 + QPointF (0.25f * x, 0.f));
		}

		// Landing altitude bug:
		if (_landing_altitude_visible && _landing_altitude > _al_min_shown && _landing_altitude < _al_max_shown)
		{
			painter.setClipRect (_al_ladder_rect);
			float posy = ft_to_px (_landing_altitude);

			painter.setPen (_al_ldg_alt_pen);
			painter.drawLine (QPointF (+2.25f * x, posy), QPointF (-2.25f * x, posy));
			for (int i = -8; i <= 4; ++i)
			{
				QPointF p (0.4f * i * x + 0.125f * x, posy + 0.1f * x);
				painter.drawLine (p, p + QPointF (x, x));
			}

			painter.setClipping (false);
			QPointF tp (_al_ladder_rect.bottomRight() + QPointF (0.25f * x, -x));
			text_painter.drawText (tp, Qt::AlignLeft | Qt::AlignVCenter, "LDG");
			text_painter.drawText (tp + QPointF (0.f, 1.5f * _font_10_digit_height), Qt::AlignLeft | Qt::AlignVCenter, "ALT");
		}

		// AP bug:
		if (_cmd_altitude_visible)
		{
			float cmd_altitude = limit (_cmd_altitude, -99999.f, +99999.f);
			float posy = limit (ft_to_px (cmd_altitude),
								static_cast<float> (-_al_ladder_rect.height() / 2), static_cast<float> (_al_ladder_rect.height() / 2));
			QPolygonF bug_shape = QPolygonF()
				<< QPointF (0.f, 0.f)
				<< QPointF (-0.5f * x, -0.5f * x)
				<< QPointF (-0.5f * x, _al_black_box_rect.top())
				<< QPointF (+1.3f * x, _al_black_box_rect.top())
				<< QPointF (+1.3f * x, _al_black_box_rect.bottom())
				<< QPointF (-0.5f * x, _al_black_box_rect.bottom())
				<< QPointF (-0.5f * x, +0.5f * x);
			painter.setClipRect (_al_ladder_rect.translated (-x, 0.f));
			painter.translate (-2.f * x, posy);
			painter.setBrush (Qt::NoBrush);
			painter.setPen (_autopilot_pen_1);
			painter.drawPolygon (bug_shape);
			painter.setPen (_autopilot_pen_2);
			painter.drawPolygon (bug_shape);
		}

		// Baro bug:
		if (_transition_altitude_visible)
		{
			if (_transition_altitude > _al_min_shown && _transition_altitude < _al_max_shown)
			{
				if (!(_baro_blinking_warning->isActive() && !_baro_blink))
				{
					float posy = ft_to_px (_transition_altitude);
					painter.setTransform (_al_transform);
					painter.setClipRect (_al_ladder_rect.adjusted (-2.5f * x, 0.f, 0.f, 0.f));
					QPen pen = get_pen (get_baro_color(), 1.25f);
					pen.setMiterLimit (0.35f);
					painter.setPen (pen);
					painter.setBrush (Qt::NoBrush);
					QPointF a (_al_ladder_rect.left(), posy);
					QPointF b (_al_ladder_rect.left() - 0.65f * x, posy - 0.65f * x);
					QPointF c (_al_ladder_rect.left() - 0.65f * x, posy + 0.65f * x);
					QPolygonF poly = QPolygonF() << a << b << c;
					painter.drawLine (a, QPointF (_al_ladder_rect.right(), posy));
					painter.drawPolygon (poly);
				}
			}
		}
	}

	// Climb rate bug:
	if (_cmd_climb_rate_visible && _climb_rate_visible)
	{
		painter.setClipping (false);
		painter.setTransform (_al_transform);
		painter.translate (4.15f * x, 0.f);
		float posy = -8.f * x * scale_cbr (_cmd_climb_rate);
		for (auto pen: { _autopilot_pen_1, _autopilot_pen_2 })
		{
			painter.setPen (pen);
			for (auto y: { posy - 0.2f * x, posy + 0.2f * x })
				painter.drawLine (QPointF (-0.25 * x, y), QPointF (0.2f * x, y));
		}
	}
}


void
EFISWidget::al_paint_climb_rate (QPainter& painter, TextPainter& text_painter, float x)
{
	QPen bold_white_pen = get_pen (Qt::white, 1.25f);
	QPen thin_white_pen = get_pen (Qt::white, 0.50f);

	float const y = x * 4.f;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.translate (4.f * x, 0.f);

	painter.setPen (_al_ladder_pen);
	painter.setBrush (_ladder_color);
	painter.drawPolygon (QPolygonF()
		<< QPointF (0.0f, -0.6 * y)
		<< QPointF (-x, -0.6 * y - x)
		<< QPointF (-x, -1.9f * y - x)
		<< QPointF (+0.3f * x, -1.9f * y - x)
		<< QPointF (1.66f * x, -y - x)
		<< QPointF (1.66f * x, +y + x)
		<< QPointF (+0.3f * x, +1.9f * y + x)
		<< QPointF (-x, +1.9f * y + x)
		<< QPointF (-x, +0.6 * y + x)
		<< QPointF (0.0f, +0.6 * y));

	if (!_climb_rate_visible)
		return;

	float const line_w = 0.2f * x;

	painter.setFont (_font_10_bold);
	painter.setPen (bold_white_pen);
	painter.drawLine (QPointF (0.f, 0.f), QPointF (0.5f * x, 0.f));
	for (float kfpm: { -6.f, -2.f, -1.f, +1.f, +2.f, +6.f })
	{
		float posy = -2.f * y * scale_cbr (kfpm * 1000.f);
		QRectF num_rect (-1.55f * x, posy - x, 1.3f * x, 2.f * x);
		painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
		text_painter.drawText (num_rect, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (kfpm))));
	}
	painter.setPen (thin_white_pen);
	for (float kfpm: { -4.f, -1.5f, -0.5f, +0.5f, +1.5f, +4.f })
	{
		float posy = -2.f * y * scale_cbr (kfpm * 1000.f);
		painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
	}
	painter.setClipRect (QRectF (0.15f * x, -2.75f * y - x, (1.66f - 0.15f) * x, 5.5f * y + 2.f * x));
	QPen indicator_pen = bold_white_pen;
	indicator_pen.setCapStyle (Qt::FlatCap);
	painter.setPen (indicator_pen);
	painter.drawLine (QPointF (3.f * x, 0.f), QPointF (line_w, -2.f * y * scale_cbr (_climb_rate)));

	// Numeric indicators:

	int abs_climb_rate = static_cast<int> (std::abs (_climb_rate)) / 10 * 10;
	if (abs_climb_rate >= 100)
	{
		float const fh = _font_13_digit_height;
		float const sgn = _climb_rate > 0.f ? 1.f : -1.f;
		painter.setClipping (false);
		painter.setFont (_font_13_bold);
		painter.translate (-1.05f * x, sgn * -2.35f * y);
		text_painter.drawText (QRectF (0.f, -0.5f * fh, 4.f * fh, fh),
							   Qt::AlignVCenter | Qt::AlignLeft, QString::number (abs_climb_rate));
	}
}


void
EFISWidget::al_paint_pressure (QPainter& painter, TextPainter& text_painter, float x)
{
	if (!_pressure_visible)
		return;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.translate (0.f, 0.75f * x);

	QFont font_a = _standard_pressure ? _font_13_bold : _font_16_bold;
	QFont font_b = _font_10_bold;
	QFontMetricsF metrics_a (font_a);
	QFontMetricsF metrics_b (font_b);

	QString unit_str = _pressure_display_hpa? "HPA" : "IN";
	int precision = _pressure_display_hpa ? 0 : 2;
	QString pressure_str = QString ("%1").arg (_pressure_display_hpa? _pressure.hpa() : _pressure.inhg(), 0, 'f', precision) + " ";

	QRectF nn_rect (0.f, _al_ladder_rect.bottom(), metrics_a.width (pressure_str), 1.2f * _font_16_digit_height);
	QRectF zz_rect (0.f, nn_rect.top(), metrics_b.width (unit_str), nn_rect.height());
	nn_rect.moveLeft (-0.5f * (zz_rect.width() + nn_rect.width()));
	// Correct position of zz_rect to get correct baseline position:
	zz_rect.translate (0.f, metrics_b.descent() - metrics_a.descent());
	zz_rect.moveLeft (nn_rect.right());

	painter.setPen (QPen (_navigation_color, pen_width()));
	if (_standard_pressure)
	{
		painter.setFont (_font_16_bold);
		text_painter.drawText (QPointF (0.5f * (nn_rect.left() + zz_rect.right()), nn_rect.bottom()), Qt::AlignHCenter | Qt::AlignBottom, "STD");
		painter.translate (0.f, 0.9f * metrics_a.height());
		painter.setPen (QPen (Qt::white, 1.f));
	}
	painter.setFont (font_a);
	text_painter.drawText (nn_rect, Qt::AlignBottom | Qt::AlignRight, pressure_str);
	painter.setFont (font_b);
	text_painter.drawText (zz_rect, Qt::AlignBottom | Qt::AlignLeft, unit_str);
}


void
EFISWidget::al_paint_ap_setting (QPainter& painter, TextPainter& text_painter)
{
	if (!_cmd_altitude_visible)
		return;

	float cmd_altitude = limit (_cmd_altitude, -99999.f, +99999.f);

	QFont b_font = _font_20_bold;
	float const b_digit_width = _font_20_digit_width;
	float const b_digit_height = _font_20_digit_height;

	QFont s_font = _font_16_bold;
	float const s_digit_width = _font_16_digit_width;

	int const b_digits = 2;
	int const s_digits = 3;
	float const margin = 0.2f * b_digit_width;

	QRectF b_digits_box (0.f, 0.f, b_digits * b_digit_width + margin, 1.3f * b_digit_height);
	QRectF s_digits_box (0.f, 0.f, s_digits * s_digit_width + margin, 1.3f * b_digit_height);
	QRectF box_rect (_al_ladder_rect.left(), _al_ladder_rect.top() - 1.4f * b_digits_box.height(),
					 b_digits_box.width() + s_digits_box.width(), b_digits_box.height());
	b_digits_box.translate (box_rect.left(), box_rect.top());
	s_digits_box.translate (b_digits_box.right(), b_digits_box.top() + 0.f * s_digits_box.height());

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.setPen (get_pen (QColor (0, 0, 0), 0.5f));
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (box_rect);

	painter.setPen (get_pen (_autopilot_color, 1.f));
	painter.setFont (b_font);

	// 11000 part of the altitude setting:
	QRectF box_11000 = b_digits_box.adjusted (margin, margin, 0.f, -margin);
	QString minus_sign_s = cmd_altitude < 0.f ? MINUS_SIGN : "";
	text_painter.drawText (box_11000, Qt::AlignVCenter | Qt::AlignRight,
						   minus_sign_s + QString::number (std::abs (static_cast<int> (cmd_altitude / 1000))));

	painter.setFont (s_font);

	// 00111 part of the altitude setting:
	QRectF box_00111 = s_digits_box.adjusted (0.f, margin, -margin, -margin);
	text_painter.drawText (box_00111, Qt::AlignVCenter | Qt::AlignLeft,
						   QString ("%1").arg (static_cast<int> (std::abs (cmd_altitude)) % 1000, 3, 'f', 0, '0'));
}


float
EFISWidget::scale_cbr (FeetPerMinute climb_rate) const
{
	FeetPerMinute cbr = std::abs (climb_rate);

	if (cbr < 1000.f)
		cbr = cbr / 1000.f * 0.46f;
	else if (cbr < 2000)
		cbr = 0.46f + 0.32f * (cbr - 1000.f) / 1000.f;
	else if (cbr < 6000)
		cbr = 0.78f + 0.22f * (cbr - 2000.f) / 4000.f;
	else
		cbr = 1.f;

	if (climb_rate < 0.f)
		cbr *= -1.f;

	return cbr;
}


void
EFISWidget::paint_center_cross (QPainter& painter, bool center_box, bool rest)
{
	float const w = wh() * 3.f / 9.f;

	QPointF x (0.025f * w, 0.f);
	QPointF y (0.f, 0.025f * w);
	QPolygonF a = QPolygonF()
		<< -x - y
		<<  x - y
		<<  x + y
		<< -x + y;
	QPolygonF b = QPolygonF()
		<< -27.f * x - y
		<< -11.f * x - y
		<< -11.f * x + 4.f * y
		<< -13.f * x + 4.f * y
		<< -13.f * x + y
		<< -27.f * x + y;

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	if (rest)
	{
		painter.setBrush (QBrush (QColor (0, 0, 0)));
		painter.setPen (Qt::NoPen);
		painter.drawPolygon (a);
		painter.setPen (get_pen (Qt::white, 1.5f));
		painter.drawPolygon (b);
		painter.scale (-1.f, 1.f);
		painter.drawPolygon (b);
	}

	if (center_box)
	{
		painter.setPen (get_pen (Qt::white, 1.5f));
		painter.setBrush (Qt::NoBrush);
		painter.drawPolygon (a);
	}
}


void
EFISWidget::paint_flight_director (QPainter& painter)
{
	float const w = wh() * 1.4f / 9.f;
	Angle range = _fov / 4.f;

	Angle pitch = std::cos (_roll.rad()) * (_flight_director_pitch - _pitch);
	pitch = limit (pitch, -range, +range);

	Angle roll = _flight_director_roll - _roll;
	if (std::abs (roll.deg()) > 180.0)
		roll = roll - sgn (roll.deg()) * 360_deg;
	roll = limit (roll, -range, +range);

	float ypos = pitch_to_px (pitch);
	float xpos = heading_to_px (roll) / 2.f;

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	for (auto pen: { get_pen (_autopilot_pen_1.color(), 2.5f),
					 get_pen (_autopilot_pen_2.color(), 1.66f) })
	{
		painter.setPen (pen);
		if (_flight_director_pitch_visible && _pitch_visible)
			painter.drawLine (QPointF (-w, ypos), QPointF (+w, ypos));
		if (_flight_director_roll_visible && _roll_visible)
			painter.drawLine (QPointF (xpos, -w), QPointF (xpos, +w));
	}
}


void
EFISWidget::paint_control_stick (QPainter& painter)
{
	if (!_control_stick_visible)
		return;

	float const w = wh() * 0.2f / 9.f;
	Angle range = _fov / 4.f;

	Angle pitch = limit (_control_stick_pitch, -range, +range);
	Angle roll = limit (_control_stick_roll, -range, +range);

	float ypos = pitch_to_px (pitch);
	float xpos = heading_to_px (roll) / 2.f;

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	for (auto pen: { get_pen (_navigation_color.darker (300), 2.5f),
					 get_pen (_navigation_color, 1.5f) })
	{
		painter.setPen (pen);
		if (_flight_director_roll_visible || _flight_director_pitch_visible)
		{
			// X cross, if flight director visible:
			painter.drawLine (QPointF (xpos - w, ypos - w), QPointF (xpos + w, ypos + w));
			painter.drawLine (QPointF (xpos - w, ypos + w), QPointF (xpos + w, ypos - w));
		}
		else
		{
			// + cross, if alone:
			painter.drawLine (QPointF (xpos, ypos - w), QPointF (xpos, ypos + w));
			painter.drawLine (QPointF (xpos - w, ypos), QPointF (xpos + w, ypos));
		}
	}
}


void
EFISWidget::paint_altitude_agl (QPainter& painter, TextPainter& text_painter)
{
	if (!_altitude_agl_visible)
		return;

	float aagl = limit (_altitude_agl, -9999.f, +99999.f);
	QFont radar_altimeter_font = _font_20_bold;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;
	float const v = 0.03f * _q;

	int digits = 4;
	if (_altitude_agl > 9999)
		digits = 5;
	float const margin = 0.2f * digit_width;

	QRectF box_rect (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	box_rect.translate (-box_rect.width() / 2.f, 0.35f * wh());
	box_rect.adjust (-v, -v, +v, +v);

	painter.setClipping (false);
	painter.setTransform (_center_transform);
	if (is_newly_set (_altitude_agl_ts))
		painter.setPen (get_pen (Qt::white, 1.25f));
	else
		painter.setPen (Qt::NoPen);
	painter.setBrush (QBrush (Qt::black));
	painter.drawRect (box_rect);

	painter.setPen (get_pen (Qt::white, 1.f));
	painter.setFont (radar_altimeter_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	text_painter.drawText (box, Qt::AlignVCenter | Qt::AlignHCenter, QString ("%1").arg (std::round (aagl)));
}


void
EFISWidget::paint_baro_setting (QPainter& painter, TextPainter& text_painter)
{
	if (!_transition_altitude_visible)
		return;

	float x = 0.18f * wh();

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	QFont font_a = _font_10_bold;
	QFont font_b = _font_16_bold;
	QFontMetricsF metrics_a (font_a);
	QFontMetricsF metrics_b (font_b);

	QString baro_str = "BARO";
	QString alt_str = QString ("%1").arg (_transition_altitude, 0, 'f', 0);

	QRectF baro_rect (x, 1.8f * x, metrics_a.width (baro_str), metrics_a.height());
	QRectF alt_rect (0.f, 0.f, metrics_b.width (alt_str), metrics_b.height());
	alt_rect.moveTopRight (baro_rect.bottomRight());

	QPen baro_pen = get_pen (get_baro_color(), 1.f);

	if (!(_baro_blinking_warning->isActive() && !_baro_blink))
	{
		painter.setPen (baro_pen);
		painter.setFont (font_a);
		text_painter.drawText (baro_rect, Qt::AlignVCenter | Qt::AlignRight, baro_str);
		painter.setFont (font_b);
		text_painter.drawText (alt_rect, Qt::AlignVCenter | Qt::AlignRight, alt_str);
	}

	if (is_newly_set (_transition_altitude_ts))
	{
		float v = 0.06f * _q;
		QRectF frame = alt_rect.united (baro_rect).adjusted (-2.f * v, -0.75f * v, +2.f * v, 0.f);
		painter.setPen (baro_pen);
		painter.setBrush (Qt::NoBrush);
		painter.drawRect (frame);
	}
}


void
EFISWidget::paint_nav (QPainter& painter, TextPainter& text_painter)
{
	painter.setClipping (false);
	painter.setTransform (_center_transform);

	if (_approach_reference_visible)
	{
		if (_localizer_info_visible)
		{
			QString loc_str = QString ("%1/%2°").arg (_localizer_id).arg (std::round (floored_mod (_localizer_magnetic_bearing.deg(), 360.0)));
			QFont font = _font_10_bold;
			font.setBold (false);

			painter.setPen (Qt::white);
			painter.setFont (font);
			text_painter.drawText (QPointF (-0.24f * wh(), -0.3925f * wh()), Qt::AlignTop | Qt::AlignLeft, loc_str);
		}

		if (_approach_hint != "")
		{
			QFont font = _font_16_bold;
			font.setBold (false);

			painter.setPen (Qt::white);
			painter.setFont (font);
			text_painter.drawText (QPointF (-0.24f * wh(), -0.32f * wh()), Qt::AlignTop | Qt::AlignLeft, _approach_hint);
		}

		QString dme_val = QString ("DME %1").arg (_dme_distance.nm(), 0, 'f', 1);
		if (!_dme_distance_visible)
			dme_val = "DME –––";
		QFont font = _font_10_bold;
		font.setBold (false);

		painter.setPen (Qt::white);
		painter.setFont (font);
		text_painter.drawText (QPointF (-0.24f * wh(), -0.36f * wh()), Qt::AlignTop | Qt::AlignLeft, dme_val);

		QPen ladder_pen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
		QPen white_pen = get_pen (Qt::white, 1.8f);

		auto paint_ladder = [&](bool needle_visible, Angle track_deviation) -> void
		{
			track_deviation = limit (track_deviation, -2_deg, +2_deg);

			QRectF rect (0.f, 0.f, 0.385f * wh(), 0.055f * wh());
			rect.translate (-rect.width() / 2.f, -rect.height() / 2.f);

			QRectF elli (0.f, 0.f, 0.015f * wh(), 0.015f * wh());
			elli.translate (-elli.width() / 2.f, -elli.height() / 2.f);

			painter.setPen (ladder_pen);
			painter.setBrush (_ladder_color);
			painter.drawRect (rect);

			if (needle_visible)
			{
				float w = 0.012f * wh();
				QPolygonF diamond = QPolygonF()
					<< QPointF (0.f, -w)
					<< QPointF (+1.6f * w, 0.f)
					<< QPointF (0.f, +w)
					<< QPointF (-1.6f * w, 0.f);
				diamond.translate (track_deviation.deg() * 0.075f * wh(), 0.f);
				for (auto pen: { _autopilot_pen_1, _autopilot_pen_2 })
				{
					painter.setPen (pen);
					painter.setBrush (pen.color());
					painter.drawPolygon (diamond);
					painter.drawPolygon (diamond);
				}
			}

			painter.setPen (white_pen);
			painter.setBrush (Qt::NoBrush);
			for (float x: { -1.f, -0.5f, +0.5f, +1.f })
				painter.drawEllipse (elli.translated (0.15f * wh() * x, 0.f));

			painter.drawLine (QPointF (0.f, -rect.height() / 3.f), QPointF (0.f, +rect.height() / 3.f));
		};

		painter.setTransform (_center_transform);
		painter.translate (0.f, 0.452f * wh());
		paint_ladder (_lateral_deviation_visible, _lateral_deviation_deg);

		painter.setTransform (_center_transform);
		painter.translate (0.28f * wh(), 0.f);
		painter.rotate (-90);
		paint_ladder (_vertical_deviation_visible, _vertical_deviation_deg);
	}

	if (_runway_visible)
	{
		float w = 0.10f * wh();
		float h = 0.05f * wh();
		float p = 1.3f;
		float offset = 0.5f * limit (_lateral_deviation_deg, -2_deg, +2_deg).deg();
		float ypos = -pitch_to_px (limit (_pitch + _runway_position, 0_deg, 25_deg));

		painter.setTransform (_center_transform);
		painter.translate (0.f, ypos);

		QPointF tps[] = { QPointF (-w, 0.f), QPointF (0.f, 0.f), QPointF (+w, 0.f) };
		QPointF bps[] = { QPointF (-w * p, h), QPointF (0.f, h), QPointF (+w * p, h) };

		for (QPointF& point: tps)
			point += QPointF (2.5f * w * offset, 0);
		for (QPointF& point: bps)
			point += QPointF (2.5f * p * w * offset, 0);

		painter.setClipRect (QRectF (-2.5f * w, -0.2f * h, 5.f * w, 1.4f * h));

		QPolygonF runway = QPolygonF() << tps[0] << tps[2] << bps[2] << bps[0];

		painter.setBrush (Qt::NoBrush);
		for (auto pen: { QPen (_navigation_color.darker (400), pen_width (2.f), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin),
						 QPen (_navigation_color, pen_width (1.33f), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin) })
		{
			painter.setPen (pen);
			painter.drawPolygon (runway);
			painter.drawLine (tps[1], bps[1]);
		}
	}
}


void
EFISWidget::paint_hints (QPainter& painter, TextPainter& text_painter)
{
	float const q = 0.1f * wh();

	if (_control_hint_visible)
	{
		painter.setClipping (false);
		painter.setTransform (_center_transform);
		painter.setFont (_font_20_bold);
		painter.setPen (get_pen (_navigation_color, 1.0));
		QPointF text_hook = QPointF (0.f, -3.1f * q);
		text_painter.drawText (text_hook, Qt::AlignVCenter | Qt::AlignHCenter, _control_hint);

		if (is_newly_set (_control_hint_ts))
		{
			float v = 0.055f * _q;
			QRectF frame (text_hook, QSizeF (2.f * _q, _font_20_digit_height));
			centrify (frame);
			frame.adjust (0.f, -v, 0.f, +v);
			painter.drawRect (frame);
		}
	}

	if (_fma_visible)
	{
		QRectF rect (0.f, 0.f, 6.3f * q, 0.65f * q);
		centrify (rect);

		float x16 = rect.left() + 1.f / 6.f * rect.width();
		float x26 = rect.left() + 2.f / 6.f * rect.width();
		float x36 = rect.left() + 3.f / 6.f * rect.width();
		float x46 = rect.left() + 4.f / 6.f * rect.width();
		float x56 = rect.left() + 5.f / 6.f * rect.width();
		float y13 = rect.top() + 8.5f / 30.f * rect.height();
		float y23 = rect.top() + 23.5f / 30.f * rect.height();

		QPointF b1 = QPointF (x16, y13);
		QPointF b2 = QPointF (x36, y13);
		QPointF b3 = QPointF (x56, y13);

		QPointF s1 = QPointF (x16, y23);
		QPointF s2 = QPointF (x36, y23);
		QPointF s3 = QPointF (x56, y23);

		auto paint_big_rect = [&](QPointF point) -> void
		{
			float v = 0.03f * _q;
			QRectF frame (point, QSizeF (1.9f * _q, _font_13_digit_height));
			centrify (frame);
			frame.adjust (0.f, -v, 0.f, +v);
			painter.drawRect (frame);
		};

		auto paint_small_rect = [&](QPointF point) -> void
		{
			float v = 0.025f * _q;
			QRectF frame (point, QSizeF (1.9f * _q, _font_10_digit_height));
			centrify (frame);
			frame.adjust (0.f, -v, 0.f, +v);
			painter.drawRect (frame);
		};

		painter.setClipping (false);
		painter.setTransform (_center_transform);
		painter.translate (0.f, -4.575f * q);
		painter.setPen (QPen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
		painter.setBrush (_ladder_color);
		painter.drawRect (rect);
		painter.setPen (get_pen (Qt::white, 1.2f));
		painter.drawLine (QPointF (x26, rect.top()), QPointF (x26, rect.bottom()));
		painter.drawLine (QPointF (x46, rect.top()), QPointF (x46, rect.bottom()));
		painter.setPen (get_pen (_navigation_color, 1.0f));
		painter.setBrush (Qt::NoBrush);

		painter.setFont (_font_13_bold);
		text_painter.drawText (b1, Qt::AlignVCenter | Qt::AlignHCenter, _fma_speed_hint);
		text_painter.drawText (b2, Qt::AlignVCenter | Qt::AlignHCenter, _fma_lateral_hint);
		text_painter.drawText (b3, Qt::AlignVCenter | Qt::AlignHCenter, _fma_vertical_hint);

		painter.setFont (_font_10_bold);
		text_painter.drawText (s1, Qt::AlignVCenter | Qt::AlignHCenter, _fma_speed_small_hint);
		text_painter.drawText (s2, Qt::AlignVCenter | Qt::AlignHCenter, _fma_lateral_small_hint);
		text_painter.drawText (s3, Qt::AlignVCenter | Qt::AlignHCenter, _fma_vertical_small_hint);

		if (_fma_speed_hint != "" && is_newly_set (_fma_speed_ts))
			paint_big_rect (b1);
		if (_fma_lateral_hint != "" && is_newly_set (_fma_lateral_ts))
			paint_big_rect (b2);
		if (_fma_vertical_hint != "" && is_newly_set (_fma_vertical_ts))
			paint_big_rect (b3);

		if (_fma_speed_small_hint != "" && is_newly_set (_fma_speed_small_ts))
			paint_small_rect (s1);
		if (_fma_lateral_small_hint != "" && is_newly_set (_fma_lateral_small_ts))
			paint_small_rect (s2);
		if (_fma_vertical_small_hint != "" && is_newly_set (_fma_vertical_small_ts))
			paint_small_rect (s3);
	}
}


void
EFISWidget::paint_pitch_limit (QPainter& painter)
{
	if (!_pitch_limit_visible || !_pitch_visible)
		return;

	painter.setClipping (false);
	painter.setTransform (_center_transform);
	painter.translate (0.f, pitch_to_px (limit (_pitch_limit - _pitch, -20_deg, +20_deg)));

	float const w = wh() * 3.f / 9.f;

	QPointF x (0.025f * w, 0.f);
	QPointF y (0.f, 0.025f * w);

	auto paint = [&](QColor color, float pen_width_scale) -> void
	{
		painter.setPen (get_pen (color, pen_width_scale * 2.f));
		painter.drawPolyline (QPolygonF()
			<< -11.f * x + y
			<< -11.f * x - y
			<< -17.f * x - y
		);
		QPen pen = get_pen (color, pen_width_scale * 1.5f);
		pen.setCapStyle (Qt::FlatCap);
		painter.setPen (pen);
		painter.drawLine (-12.5f * x - y, -14.f * x - 3.65f * y);
		painter.drawLine (-14.f * x - y, -15.5f * x - 3.65f * y);
		painter.drawLine (-15.5f * x - y, -17.f * x - 3.65f * y);
	};

	paint (_warning_color_2.darker (160), 1.0f);
	paint (_warning_color_2, 0.65f);
	painter.scale (-1.f, 1.f);
	paint (_warning_color_2.darker (160), 1.0f);
	paint (_warning_color_2, 0.65f);
}


void
EFISWidget::paint_input_alert (QPainter& painter, TextPainter& text_painter)
{
	QFont font = _font;
	font.setPixelSize (font_size (30.f));
	font.setBold (true);

	QString alert = "NO INPUT";

	QFontMetricsF font_metrics (font);
	int width = font_metrics.width (alert);

	QPen pen = get_pen (Qt::white, 2.f);

	painter.setClipping (false);

	painter.setTransform (_center_transform);
	painter.setPen (Qt::NoPen);
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (rect());

	painter.setTransform (_center_transform);
	painter.setPen (pen);
	painter.setBrush (QBrush (QColor (0xdd, 0, 0)));
	painter.setFont (font);

	QRectF rect (-0.6f * width, -0.5f * font_metrics.height(), 1.2f * width, 1.2f * font_metrics.height());

	painter.drawRect (rect);
	text_painter.drawText (rect, Qt::AlignVCenter | Qt::AlignHCenter, alert);
}


void
EFISWidget::paint_dashed_zone (QPainter& painter, QColor const& color, QRectF const& target)
{
	QFontMetricsF metrics (painter.font());
	float w = 0.7f * metrics.width ("0");
	float h = 0.55f * metrics.height();
	QPointF center = target.center();
	QRectF box (center - QPointF (w / 2.f, h / 1.9f), QSizeF (w, h));
	QPen pen = get_pen (color, 1.2f);
	QPointF difx (box.width() / 2.5f, 0.f);
	QPointF dify (0.f, box.height() / 2.5f);
	pen.setCapStyle (Qt::RoundCap);
	painter.save();
	painter.setPen (pen);
	painter.drawLine (box.topLeft(), box.bottomRight());
	painter.drawLine (box.topLeft() + difx, box.bottomRight() - dify);
	painter.drawLine (box.topLeft() + dify, box.bottomRight() - difx);
	painter.drawLine (box.topLeft() + 2.f * difx, box.bottomRight() - 2.f * dify);
	painter.drawLine (box.topLeft() + 2.f * dify, box.bottomRight() - 2.f * difx);
	painter.restore();
}


void
EFISWidget::paint_rotating_value (QPainter& painter, TextPainter& text_painter,
								  QRectF const& rect, float position, float height_scale,
								  QString const& next, QString const& curr, QString const& prev)
{
	QColor red (255, 0, 0);
	QColor green (0, 255, 0);

	QFont font = painter.font();
	QFontMetricsF font_metrics (font);
	float height = height_scale * font_metrics.height();

	QRectF box_next = rect.translated (0.f, -height);
	QRectF box_prev = rect.translated (0.f, +height);

	painter.save();
	painter.setClipRect (rect);
	painter.translate (0.f, -height * position);

	for (std::pair<QRectF, QString> x: { std::make_pair (box_next, next),
										 std::make_pair (rect, curr),
										 std::make_pair (box_prev, prev) })
	{
		if (x.second == "G" || x.second == "R")
			paint_dashed_zone (painter, x.second == "G" ? green : red, x.first);
		else if (x.second == "-")
			; // Paint nothing.
		else
			text_painter.drawText (x.first, Qt::AlignVCenter | Qt::AlignLeft, x.second);
	}

	painter.restore();
}


void
EFISWidget::paint_rotating_digit (QPainter& painter, TextPainter& text_painter,
								  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
								  bool two_zeros, bool zero_mark, bool black_zero)
{
	auto round_to = [] (float value, int to) -> float
	{
		float sgn = value >= 0.f ? +1.f : -1.f;
		return static_cast<int> (value + sgn * to / 2.f) / to * to;
	};

	float rounded = round_to (value + phase, round_target);
	float dtr = (value + phase - rounded) / round_target;
	float pos = 0.f;
	float epsilon = 0.000001f;
	float xa = std::fmod ((value + phase) / round_target + 1.f - epsilon, 10.f);
	float xb = std::fmod ((value + phase) / round_target + 0.f - epsilon, 10.f);
	float xc = std::fmod ((value + phase) / round_target - 1.f - epsilon, 10.f);

	int a = static_cast<int> (std::abs (xa));
	int b = static_cast<int> (std::abs (xb));
	int c = static_cast<int> (std::abs (xc));

	QString sa = zero_mark && a == 0 ? (black_zero ? "-" : (xa >= 0.f ? "G" : "R")) : QString::number (a);
	QString sb = zero_mark && b == 0 ? (black_zero ? "-" : (xb >= 0.f ? "G" : "R")) : QString::number (b);
	QString sc = zero_mark && c == 0 ? (black_zero ? "-" : (xc >= 0.f ? "G" : "R")) : QString::number (c);

	if (std::abs (dtr) < delta && (two_zeros || std::abs (value) >= round_target / 2))
		pos = floored_mod (-dtr * (0.5f / delta), 1.f) - 0.5f;

	paint_rotating_value (painter, text_painter, box, pos, height_scale, sa, sb, sc);
}


QPainterPath
EFISWidget::get_pitch_scale_clipping_path() const
{
	float const w = wh() * 2.f / 9.f;

	QPainterPath clip_path; // TODO cache this
	clip_path.setFillRule (Qt::WindingFill);
	clip_path.addEllipse (QRectF (-1.15f * w, -1.175f * w, 2.30f * w, 2.35f * w));
	clip_path.addRect (QRectF (-1.15f * w, 0.f, 2.30f * w, 1.375f * w));

	if (_flight_path_visible)
		return clip_path - _flight_path_marker_clip.translated (_flight_path_marker_position);
	else
		return clip_path;
}


void
EFISWidget::update_blinker (QTimer* warning_timer, bool condition, bool* blink_state)
{
	if (condition)
	{
		if (!warning_timer->isActive())
		{
			warning_timer->start();
			*blink_state = true;
		}
	}
	else if (warning_timer->isActive())
		warning_timer->stop();
}

