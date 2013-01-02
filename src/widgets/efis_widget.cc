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


EFISWidget::AltitudeLadder::AltitudeLadder (EFISWidget& efis, QPainter& painter):
	_efis (efis),
	_painter (painter),
	_text_painter (_painter, &_efis._text_painter_cache),
	_altitude (bound (_efis._altitude, -9999.f, +99999.f)),
	_climb_rate (bound (_efis._climb_rate, -9999.f, +9999.f)),
	_pressure (bound (_efis._pressure, 0.f, 99.99f)),
	_extent (_efis._altitude_ladder_extent),
	_sgn (_altitude < 0.f ? -1.f : 1.f),
	_min_shown (_altitude - _extent / 2.f),
	_max_shown (_altitude + _extent / 2.f),
	_rounded_altitude (static_cast<int> (_altitude + _sgn * 10.f) / 20 * 20),
	_ladder_rect (-0.0675f * _efis.wh(), -0.375 * _efis.wh(), 0.135 * _efis.wh(), 0.75f * _efis.wh()),
	_ladder_pen (_efis._ladder_border_color, _efis.pen_width (0.75f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin),
	_black_box_pen (_efis.get_pen (QColor (255, 255, 255), 1.2f)),
	_scale_pen_1 (_efis.get_pen (QColor (255, 255, 255), 1.f)),
	_scale_pen_2 (_efis.get_pen (QColor (255, 255, 255), 3.f)),
	_altitude_bug_pen (_efis.get_pen (QColor (0, 255, 0), 1.5f)),
	_ldg_alt_pen (_efis.get_pen (QColor (255, 220, 0), 1.5f))
{ }


void
EFISWidget::AltitudeLadder::paint()
{
	float const x = _ladder_rect.width() / 4.0f;

	_painter.save();

	_painter.setPen (_ladder_pen);
	_painter.setBrush (_efis._ladder_color);
	_painter.drawRect (_ladder_rect);

	paint_black_box (x, true);
	paint_ladder_scale (x);
	paint_climb_rate (x);
	paint_bugs (x);
	paint_altitude_tendency (x);
	paint_black_box (x);
	paint_pressure (x);
	paint_ap_setting (x);

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_black_box (float x, bool only_compute_black_box_rect)
{
	QFont b_font = _efis._font_20_bold;
	float const b_digit_width = _efis._font_20_digit_width;
	float const b_digit_height = _efis._font_20_digit_height;

	QFont s_font = _efis._font_16_bold;
	float const s_digit_width = _efis._font_16_digit_width;
	float const s_digit_height = _efis._font_16_digit_height;

	int const b_digits = 2;
	int const s_digits = 3;
	float const margin = 0.2f * b_digit_width;

	QRectF b_digits_box (0.f, 0.f, b_digits * b_digit_width, 2.f * b_digit_height - 2.f * margin);
	QRectF s_digits_box (0.f, 0.f, s_digits * s_digit_width, 2.f * b_digit_height - 2.f * margin);
	_black_box_rect = QRectF (0.f, -0.5f * b_digits_box.height() - margin,
							  b_digits_box.width() + s_digits_box.width() + 2.f * margin, b_digits_box.height() + 2.f * margin);

	if (only_compute_black_box_rect)
		return;
	if (!_efis._altitude_visible)
		return;
	b_digits_box.translate (margin, -0.5f * b_digits_box.height());
	s_digits_box.translate (margin + b_digits_box.width(), -0.5f * s_digits_box.height());

	_painter.save();
	_painter.translate (-0.75f * x, 0.f);

	_painter.setPen (_black_box_pen);
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawPolygon (QPolygonF()
		<< QPointF (-0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _black_box_rect.topLeft()
		<< _black_box_rect.topRight()
		<< _black_box_rect.bottomRight()
		<< _black_box_rect.bottomLeft()
		<< QPointF (0.f, +0.5f * x));

	bool show_zero_mark = -10000.f < _rounded_altitude && _rounded_altitude < 10000.f;

	QRectF box_10000 = QRectF (b_digits_box.topLeft(), QSizeF (b_digit_width, b_digits_box.height()));
	QRectF box_01000 = box_10000.translated (b_digit_width, 0.f);
	QRectF box_00100 = QRectF (s_digits_box.topLeft(), QSizeF (s_digit_width, b_digits_box.height()));
	QRectF box_00011 = box_00100.translated (s_digit_width, 0.f).adjusted (0.f, 0.f, s_digit_width, 0.f);

	// 11100 part:
	_painter.setFont (b_font);
	_efis.paint_rotating_digit (_painter, _text_painter, box_10000, _altitude, 10000, 1.4f * s_digit_height / b_digit_height, 0.0005f, 5.f, true, show_zero_mark);
	_efis.paint_rotating_digit (_painter, _text_painter, box_01000, _altitude, 1000, 1.4f * s_digit_height / b_digit_height, 0.005f, 5.f, false, false);
	_painter.setFont (s_font);
	_efis.paint_rotating_digit (_painter, _text_painter, box_00100, _altitude, 100, 1.4f, 0.05f, 5.f, false, false);

	// 00011 part:
	float pos_00011 = (_rounded_altitude - _altitude) / 20.f;
	_efis.paint_rotating_value (_painter, _text_painter, box_00011, pos_00011, 0.7f,
								QString::number (static_cast<int> (std::abs (std::fmod (_rounded_altitude / 10.f + 2.f, 10.f)))) + "0",
								QString::number (static_cast<int> (std::abs (std::fmod (_rounded_altitude / 10.f + 0.f, 10.f)))) + "0",
								QString::number (static_cast<int> (std::abs (std::fmod (_rounded_altitude / 10.f - 2.f, 10.f)))) + "0");

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_ladder_scale (float x)
{
	if (!_efis._altitude_visible)
		return;

	int const line_every = _efis._altitude_ladder_line_every;
	int const num_every = _efis._altitude_ladder_number_every;;
	int const bold_every = _efis._altitude_ladder_bold_every;;

	QFont b_ladder_font = _efis._font_13_bold;
	float const b_ladder_digit_width = _efis._font_13_digit_width;
	float const b_ladder_digit_height = _efis._font_13_digit_height;

	QFont s_ladder_font = _efis._font_10_bold;
	float const s_ladder_digit_width = _efis._font_10_digit_width;
	float const s_ladder_digit_height = _efis._font_10_digit_height;

	// Special clipping that leaves some margin around black indicator:
	QPainterPath clip_path_m;
	clip_path_m.addRect (_black_box_rect.translated (-x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_ladder_rect);
	clip_path -= clip_path_m;

	_painter.save();
	_painter.setClipPath (clip_path);
	_painter.translate (-2.f * x, 0.f);

	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int ft = (static_cast<int> (_min_shown) / line_every) * line_every - line_every;
		 ft <= _max_shown + line_every;
		 ft += line_every)
	{
		if (ft >  100000.f)
			continue;

		float posy = ft_to_px (ft);

		_painter.setPen (ft % bold_every == 0 ? _scale_pen_2 : _scale_pen_1);
		_painter.drawLine (QPointF (0.f, posy), QPointF (0.8f * x, posy));

		if (ft % num_every == 0)
		{
			QRectF big_text_box (1.1f * x, -0.5f * b_ladder_digit_height + posy,
								 2.f * b_ladder_digit_width, b_ladder_digit_height);
			if (std::abs (ft) / 1000 > 0)
			{
				QString big_text = QString::number (ft / 1000);
				_painter.setFont (b_ladder_font);
				_text_painter.drawText (big_text_box, Qt::AlignVCenter | Qt::AlignRight, big_text);
			}

			QString small_text = QString ("%1").arg (QString::number (std::abs (ft % 1000)), 3, '0');
			if (ft == 0)
				small_text = "0";
			_painter.setFont (s_ladder_font);
			QRectF small_text_box (1.1f * x + 2.1f * b_ladder_digit_width, -0.5f * s_ladder_digit_height + posy,
								   3.f * s_ladder_digit_width, s_ladder_digit_height);
			_text_painter.drawText (small_text_box, Qt::AlignVCenter | Qt::AlignRight, small_text);
			// Minus sign?
			if (ft < 0)
			{
				if (ft > -1000)
					_text_painter.drawText (small_text_box.adjusted (-s_ladder_digit_width, 0.f, 0.f, 0.f),
											Qt::AlignVCenter | Qt::AlignLeft, MINUS_SIGN);
			}
		}
	}

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_altitude_tendency (float x)
{
	if (!_efis._altitude_tendency_visible || !_efis._altitude_visible)
		return;

	QPen pen (_efis.get_pen (_efis._navigation_color, 1.25f));
	pen.setCapStyle (Qt::RoundCap);
	pen.setJoinStyle (Qt::RoundJoin);

	_painter.save();

	_painter.setPen (pen);
	_painter.translate (-1.2f * x, 0.f);
	if (_efis._altitude_tendency < _altitude)
		_painter.scale (1.f, -1.f);
	float length = std::min<float> (_ladder_rect.height() / 2.f, 1.f * std::abs (ft_to_px (std::max (0.f, _efis._altitude_tendency)))) - 0.5f * x;

	if (length > 0.2f * x)
	{
		_painter.setClipRect (QRectF (_ladder_rect.topLeft(), QPointF (_ladder_rect.right(), 0.f)));
		_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -length));
		_painter.translate (0.f, -length);
		_painter.drawPolygon (QPolygonF()
			<< QPointF (0.f, -0.5f * x)
			<< QPointF (-0.2f * x, 0.f)
			<< QPointF (+0.2f * x, 0.f));
		_painter.restore();
	}
}


void
EFISWidget::AltitudeLadder::paint_bugs (float x)
{
	if (_efis._altitude_visible)
	{
		QFont altitude_bug_font = _efis._font_10_bold;
		float const altitude_bug_digit_height = _efis._font_10_digit_height;

		_painter.save();
		_painter.setFont (altitude_bug_font);

		for (auto& bug: _efis._altitude_bugs)
		{
			if (bug.second > _min_shown && bug.second < _max_shown)
			{
				float posy = ft_to_px (bug.second);
				QRectF text_rect (-4.5f * x, posy - 0.5f * altitude_bug_digit_height,
								  +2.f * x, altitude_bug_digit_height);
				_painter.setClipRect (_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));

				_painter.setPen (_altitude_bug_pen);
				_painter.drawLine (QPointF (-1.5f * x, posy), QPointF (-2.25f * x, posy));

				_painter.setClipping (false);
				_text_painter.drawText (text_rect, Qt::AlignVCenter | Qt::AlignRight, bug.first);
			}
		}

		// Landing altitude bug:
		if (_efis._landing_altitude_visible && _efis._landing_altitude > _min_shown && _efis._landing_altitude < _max_shown)
		{
			float posy = ft_to_px (_efis._landing_altitude);
			QRectF text_rect (-4.5f * x, posy - 0.5f * altitude_bug_digit_height,
							  +2.f * x, altitude_bug_digit_height);
			_painter.setClipRect (_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));

			_painter.setPen (_ldg_alt_pen);
			_painter.drawLine (QPointF (-0.5f * x, posy), QPointF (-2.25f * x, posy));

			_painter.setClipping (false);
			_text_painter.drawText (text_rect, Qt::AlignVCenter | Qt::AlignRight, "LDG");
		}

		// AP bug:
		if (_efis._ap_altitude_visible)
		{
			float posy = bound (ft_to_px (_efis._ap_altitude),
								static_cast<float> (-_ladder_rect.height() / 2), static_cast<float> (_ladder_rect.height() / 2));
			QPolygonF bug_shape = QPolygonF()
				<< QPointF (0.f, 0.f)
				<< QPointF (-0.5f * x, -0.5f * x)
				<< QPointF (-0.5f * x, _black_box_rect.top())
				<< QPointF (+1.3f * x, _black_box_rect.top())
				<< QPointF (+1.3f * x, _black_box_rect.bottom())
				<< QPointF (-0.5f * x, _black_box_rect.bottom())
				<< QPointF (-0.5f * x, +0.5f * x);
			_painter.setClipRect (_ladder_rect.translated (-x, 0.f));
			_painter.translate (-2.f * x, posy);
			_painter.setBrush (Qt::NoBrush);
			_painter.setPen (_efis._autopilot_pen_1);
			_painter.drawPolygon (bug_shape);
			_painter.setPen (_efis._autopilot_pen_2);
			_painter.drawPolygon (bug_shape);
		}

		_painter.restore();
	}

	// Climb rate bug:
	if (_efis._ap_climb_rate_visible && _efis._climb_rate_visible)
	{
		_painter.save();
		_painter.translate (4.15f * x, 0.f);
		float posy = -8.f * x * scale_cbr (_efis._ap_climb_rate);
		for (auto pen: { _efis._autopilot_pen_1, _efis._autopilot_pen_2 })
		{
			_painter.setPen (pen);
			for (auto y: { posy - 0.2f * x, posy + 0.2f * x })
				_painter.drawLine (QPointF (-0.25 * x, y), QPointF (0.2f * x, y));
		}
		_painter.restore();
	}
}


void
EFISWidget::AltitudeLadder::paint_climb_rate (float x)
{
	if (!_efis._climb_rate_visible)
		return;

	QPen bold_white_pen = _efis.get_pen (QColor (255, 255, 255), 1.25f);
	QPen thin_white_pen = _efis.get_pen (QColor (255, 255, 255), 0.50f);

	_painter.save();

	float const y = x * 4.f;

	_painter.translate (4.f * x, 0.f);

	_painter.setPen (_ladder_pen);
	_painter.setBrush (_efis._ladder_color);
	_painter.drawPolygon (QPolygonF()
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

	float const line_w = 0.2f * x;

	_painter.setFont (_efis._font_10_bold);
	_painter.setPen (bold_white_pen);
	_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.5f * x, 0.f));
	for (float kfpm: { -6.f, -2.f, -1.f, +1.f, +2.f, +6.f })
	{
		float posy = -2.f * y * scale_cbr (kfpm * 1000.f);
		QRectF num_rect (-1.55f * x, posy - x, 1.3f * x, 2.f * x);
		_painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
		_text_painter.drawText (num_rect, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (kfpm))));
	}
	_painter.setPen (thin_white_pen);
	for (float kfpm: { -4.f, -1.5f, -0.5f, +0.5f, +1.5f, +4.f })
	{
		float posy = -2.f * y * scale_cbr (kfpm * 1000.f);
		_painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
	}
	_painter.setClipRect (QRectF (0.15f * x, -2.75f * y - x, (1.66f - 0.15f) * x, 5.5f * y + 2.f * x));
	QPen indicator_pen = bold_white_pen;
	indicator_pen.setCapStyle (Qt::FlatCap);
	_painter.setPen (indicator_pen);
	_painter.drawLine (QPointF (3.f * x, 0.f), QPointF (line_w, -2.f * y * scale_cbr (_climb_rate)));

	// Numeric indicators:

	int abs_climb_rate = static_cast<int> (std::abs (_climb_rate)) / 10 * 10;
	if (abs_climb_rate >= 100)
	{
		float const fh = _efis._font_13_digit_height;
		float const sgn = _climb_rate > 0.f ? 1.f : -1.f;
		_painter.setClipping (false);
		_painter.setFont (_efis._font_13_bold);
		_painter.translate (-1.05f * x, sgn * -2.35f * y);
		_text_painter.drawText (QRectF (0.f, -0.5f * fh, 4.f * fh, fh),
								Qt::AlignVCenter | Qt::AlignLeft, QString::number (abs_climb_rate));
	}

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_pressure (float x)
{
	if (!_efis._pressure_visible)
		return;

	_painter.save();
	_painter.translate (0.f, 0.75f * x);

	QFont font_a = _efis._font_16_bold;
	QFont font_b = _efis._font_10_bold;

	QString in_str = "IN";
	QString pressure_str = QString ("%1").arg (_pressure, 0, 'f', 2) + " ";

	QRectF nn_rect (0.f, _ladder_rect.bottom(), QFontMetrics (font_a).width (pressure_str), 1.2f * _efis._font_16_digit_height);
	QRectF zz_rect (0.f, nn_rect.top(), QFontMetrics (font_b).width (in_str), nn_rect.height());
	nn_rect.moveLeft (-0.5f * (zz_rect.width() + nn_rect.width()));
	// Correct position of zz_rect to get correct baseline position:
	zz_rect.translate (0.f, QFontMetrics (font_b).descent() - QFontMetrics (font_a).descent());
	zz_rect.moveLeft (nn_rect.right());

	_painter.setPen (QPen (_efis._navigation_color, _efis.pen_width()));
	_painter.setFont (font_a);
	_text_painter.drawText (nn_rect, Qt::AlignBottom | Qt::AlignRight, pressure_str, true);
	_painter.setFont (font_b);
	_text_painter.drawText (zz_rect, Qt::AlignBottom | Qt::AlignLeft, in_str);

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_ap_setting (float)
{
	if (!_efis._ap_altitude_visible)
		return;

	QFont b_font = _efis._font_20_bold;
	float const b_digit_width = _efis._font_20_digit_width;
	float const b_digit_height = _efis._font_20_digit_height;

	QFont s_font = _efis._font_16_bold;
	float const s_digit_width = _efis._font_16_digit_width;

	int const b_digits = 2;
	int const s_digits = 3;
	float const margin = 0.2f * b_digit_width;

	QRectF b_digits_box (0.f, 0.f, b_digits * b_digit_width + margin, 1.3f * b_digit_height);
	QRectF s_digits_box (0.f, 0.f, s_digits * s_digit_width + margin, 1.3f * b_digit_height);
	QRectF box_rect (_ladder_rect.left(), _ladder_rect.top() - 1.4f * b_digits_box.height(),
					 b_digits_box.width() + s_digits_box.width(), b_digits_box.height());
	b_digits_box.translate (box_rect.left(), box_rect.top());
	s_digits_box.translate (b_digits_box.right(), b_digits_box.top() + 0.f * s_digits_box.height());

	_painter.save();

	_painter.setPen (_efis.get_pen (QColor (0, 0, 0), 0.5f));
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawRect (box_rect);

	_painter.setPen (_efis.get_pen (_efis._autopilot_color, 1.f));
	_painter.setFont (b_font);

	// 11000 part of the altitude setting:
	QRectF box_11000 = b_digits_box.adjusted (margin, margin, 0.f, -margin);
	QString minus_sign_s = _efis._ap_altitude < 0.f ? MINUS_SIGN : "";
	_painter.drawText (box_11000, Qt::AlignVCenter | Qt::AlignRight,
					   minus_sign_s + QString::number (std::abs (static_cast<int> (_efis._ap_altitude / 1000))));

	_painter.setFont (s_font);

	// 00111 part of the altitude setting:
	QRectF box_00111 = s_digits_box.adjusted (0.f, margin, -margin, -margin);
	_painter.drawText (box_00111, Qt::AlignVCenter | Qt::AlignLeft,
					   QString ("%1").arg (static_cast<int> (std::abs (_efis._ap_altitude)) % 1000, 3, 'f', 0, '0'));

	_painter.restore();
}


float
EFISWidget::AltitudeLadder::scale_cbr (FeetPerMinute climb_rate) const
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


EFISWidget::SpeedLadder::SpeedLadder (EFISWidget& efis, QPainter& painter):
	_efis (efis),
	_painter (painter),
	_text_painter (_painter, &_efis._text_painter_cache),
	_speed (bound (_efis._speed, 0.f, 9999.99f)),
	_mach (bound (_efis._mach, 0.f, 9.99f)),
	_minimum_speed (bound (_efis._minimum_speed, 0.f, 9999.99f)),
	_warning_speed (bound (_efis._warning_speed, 0.f, 9999.99f)),
	_maximum_speed (bound (_efis._maximum_speed, 0.f, 9999.99f)),
	_extent (_efis._speed_ladder_extent),
	_min_shown (_speed - _extent / 2.f),
	_max_shown (_speed + _extent / 2.f),
	_rounded_speed (static_cast<int> (_speed + 0.5f)),
	_ladder_rect (-0.0675f * _efis.wh(), -0.375 * _efis.wh(), 0.135 * _efis.wh(), 0.75f * _efis.wh()),
	_ladder_pen (_efis.get_pen (_efis._ladder_border_color, 0.75f)),
	_black_box_pen (_efis.get_pen (QColor (255, 255, 255), 1.2f)),
	_scale_pen (_efis.get_pen (QColor (255, 255, 255), 1.f)),
	_speed_bug_pen (_efis.get_pen (QColor (0, 255, 0), 1.5f))
{ }


void
EFISWidget::SpeedLadder::paint()
{
	float const x = _ladder_rect.width() / 4.0f;

	_painter.save();

	_painter.setPen (_ladder_pen);
	_painter.setBrush (_efis._ladder_color);
	_painter.drawRect (_ladder_rect);

	paint_black_box (x, true);
	paint_ladder_scale (x);
	paint_speed_limits (x);
	paint_bugs (x);
	paint_speed_tendency (x);
	paint_black_box (x);
	paint_mach_number (x);
	paint_ap_setting (x);

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_black_box (float x, bool only_compute_black_box_rect)
{
	QFont actual_speed_font = _efis._font_20_bold;
	float const digit_width = _efis._font_20_digit_width;
	float const digit_height = _efis._font_20_digit_height;

	int digits = 3;
	if (_speed >= 1000.0f - 0.5f)
		digits = 4;
	float const margin = 0.2f * digit_width;

	_black_box_rect = QRectF (-digits * digit_width - 2.f * margin, -digit_height,
							  +digits * digit_width + 2.f * margin, 2.f * digit_height);

	if (only_compute_black_box_rect)
		return;
	if (!_efis._speed_visible)
		return;

	_painter.save();
	_painter.translate (+0.75f * x, 0.f);

	QPen border_pen = _black_box_pen;
	if (_efis._blinking_warning->isActive())
	{
		border_pen.setColor (_efis._blink || (_speed < _minimum_speed)
								? _efis._warning_color_1
								: Qt::black);
	}

	_painter.setPen (border_pen);
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawPolygon (QPolygonF()
		<< QPointF (+0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _black_box_rect.topRight()
		<< _black_box_rect.topLeft()
		<< _black_box_rect.bottomLeft()
		<< _black_box_rect.bottomRight()
		<< QPointF (0.f, +0.5f * x));

	QRectF box_1000 = _black_box_rect.adjusted (margin, margin, -margin, -margin);
	QRectF box_0100 =
		digits == 3
			? box_1000
			: box_1000.adjusted (digit_width, 0.f, 0.f, 0.f);
	QRectF box_0010 = box_0100.adjusted (digit_width, 0.f, 0.f, 0.f);
	QRectF box_0001 = box_0010.adjusted (digit_width, 0.f, 0.f, 0.f);

	_painter.setPen (_black_box_pen);
	_painter.setFont (actual_speed_font);
	if (digits == 4)
		_efis.paint_rotating_digit (_painter, _text_painter, box_1000, _speed, 1000, 1.4f, 0.0005f, 0.5f, false, false);
	_efis.paint_rotating_digit (_painter, _text_painter, box_0100, _speed, 100, 1.4f, 0.005f, 0.5f, false, false);
	_efis.paint_rotating_digit (_painter, _text_painter, box_0010, _speed, 10, 1.4f, 0.05f, 0.5f, false, false);
	float pos_0001 = _rounded_speed - _speed;
	_efis.paint_rotating_value (_painter, _text_painter, box_0001, pos_0001, 0.7f,
								QString::number (static_cast<int> (std::abs (std::fmod (1.f * _rounded_speed + 1.f, 10.f)))),
								QString::number (static_cast<int> (std::abs (std::fmod (1.f * _rounded_speed, 10.f)))),
								_speed > 0.5f
									? QString::number (static_cast<int> (floored_mod (1.f * _rounded_speed - 1.f, 10.f)))
									: " ");

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_ladder_scale (float x)
{
	if (!_efis._speed_visible)
		return;

	QFont ladder_font = _efis._font_13_bold;
	float const ladder_digit_width = _efis._font_13_digit_width;
	float const ladder_digit_height = _efis._font_13_digit_height;

	_painter.setFont (ladder_font);

	int const line_every = _efis._speed_ladder_line_every;
	int const num_every = _efis._speed_ladder_number_every;

	if (_min_shown < 0.f)
		_min_shown = 0.f;

	// Special clipping that leaves some margin around black indicator:
	QPainterPath clip_path_m;
	clip_path_m.addRect (_black_box_rect.translated (x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_ladder_rect);
	clip_path -= clip_path_m;

	_painter.save();
	_painter.setClipPath (clip_path);
	_painter.translate (2.f * x, 0.f);

	_painter.setPen (_scale_pen);
	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int kt = (static_cast<int> (_min_shown) / line_every) * line_every - line_every;
		 kt <= _max_shown + line_every;
		 kt += line_every)
	{
		if (kt < 0)
			continue;
		float posy = kt_to_px (kt);
		_painter.drawLine (QPointF (-0.8f * x, posy), QPointF (0.f, posy));

		if (kt % num_every == 0)
			_text_painter.drawText (QRectF (-4.f * ladder_digit_width - 1.25f * x, -0.5f * ladder_digit_height + posy,
											+4.f * ladder_digit_width, ladder_digit_height),
									Qt::AlignVCenter | Qt::AlignRight, QString::number (kt));
	}

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_speed_limits (float x)
{
	if (!_efis._speed_visible)
		return;

	QPointF ydif (0.f, _efis.pen_width (0.25f));
	QPen pen_b (QColor (0, 0, 0), _efis.pen_width (10.f), Qt::SolidLine, Qt::FlatCap);
	QPen pen_r (QColor (255, 0, 0), _efis.pen_width (10.f), Qt::DotLine, Qt::FlatCap);
	QPen pen_y (QColor (255, 170, 0), _efis.pen_width (1.2f), Qt::SolidLine, Qt::FlatCap);
	pen_r.setDashPattern (QVector<qreal> (2, 0.5f));

	float tr_right = 0.45f * x;
	float p1w = 0.45f * _efis.pen_width (1.2f);

	_painter.save();
	_painter.translate (tr_right, 0.f);
	_painter.setClipRect (_ladder_rect.adjusted (0.f, -ydif.y(), 0.f, ydif.y()));

	float max_posy = kt_to_px (_maximum_speed);
	float wrn_posy = kt_to_px (_warning_speed);
	float min_posy = kt_to_px (_minimum_speed);
	QPointF zero_point (_ladder_rect.right(), std::min<float> (_ladder_rect.bottom() + ydif.y(), 1.f * kt_to_px (0.f)));

	if (_efis._maximum_speed_visible && _maximum_speed < _max_shown)
	{
		_painter.setPen (pen_b);
		_painter.drawLine (QPointF (_ladder_rect.right(), max_posy), _ladder_rect.topRight() - ydif);
		_painter.setPen (pen_r);
		_painter.drawLine (QPointF (_ladder_rect.right(), max_posy), _ladder_rect.topRight() - ydif);
	}

	if (_efis._warning_speed_visible && _warning_speed > _min_shown)
	{
		_painter.setPen (pen_y);
		_painter.drawPolyline (QPolygonF()
			<< QPointF (_ladder_rect.right() - tr_right, wrn_posy)
			<< QPointF (_ladder_rect.right() - p1w, wrn_posy)
			<< zero_point - QPointF (p1w, 0.f));
	}

	if (_efis._minimum_speed_visible && _minimum_speed > _min_shown)
	{
		_painter.setPen (pen_b);
		_painter.drawLine (QPointF (_ladder_rect.right(), min_posy), zero_point);
		_painter.setPen (pen_r);
		_painter.drawLine (QPointF (_ladder_rect.right(), min_posy), zero_point);
	}

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_speed_tendency (float x)
{
	if (!_efis._speed_tendency_visible || !_efis._speed_visible)
		return;

	QPen pen (_efis.get_pen (_efis._navigation_color, 1.25f));
	pen.setCapStyle (Qt::RoundCap);
	pen.setJoinStyle (Qt::RoundJoin);

	_painter.save();

	_painter.setPen (pen);
	_painter.translate (1.2f * x, 0.f);
	if (_efis._speed_tendency < _speed)
		_painter.scale (1.f, -1.f);
	float length = std::min<float> (_ladder_rect.height() / 2.f, 1.f * std::abs (kt_to_px (std::max (0.f, _efis._speed_tendency)))) - 0.5f * x;

	if (length > 0.2f * x)
	{
		_painter.setClipRect (QRectF (_ladder_rect.topLeft(), QPointF (_ladder_rect.right(), 0.f)));
		_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -length));
		_painter.translate (0.f, -length);
		_painter.drawPolygon (QPolygonF()
			<< QPointF (0.f, -0.5f * x)
			<< QPointF (-0.2f * x, 0.f)
			<< QPointF (+0.2f * x, 0.f));
	}

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_bugs (float x)
{
	if (!_efis._speed_visible)
		return;

	QFont speed_bug_font = _efis._font_10_bold;
	float const speed_bug_digit_height = _efis._font_10_digit_height;

	_painter.save();
	_painter.setFont (speed_bug_font);

	for (auto& bug: _efis._speed_bugs)
	{
		if (bug.second > _min_shown && bug.second < _max_shown)
		{
			float posy = kt_to_px (bug.second);
			_painter.setPen (_speed_bug_pen);
			_painter.setClipRect (_ladder_rect.translated (x, 0.f));
			_painter.drawLine (QPointF (1.5f * x, posy), QPointF (2.25f * x, posy));
			_painter.setClipping (false);
			_text_painter.drawText (QRectF (2.5f * x, posy - 0.5f * speed_bug_digit_height,
											2.f * x, speed_bug_digit_height),
									Qt::AlignVCenter | Qt::AlignLeft, bug.first);
		}
	}

	// AT bug:
	if (_efis._at_speed_visible)
	{
		float posy = bound (kt_to_px (_efis._at_speed),
							static_cast<float> (-_ladder_rect.height() / 2.f), static_cast<float> (_ladder_rect.height() / 2.f));
		QPolygonF bug_shape = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.5f * x, -0.5f * x)
			<< QPointF (2.f * x, -0.5f * x)
			<< QPointF (2.f * x, +0.5f * x)
			<< QPointF (+0.5f * x, +0.5f * x);
		_painter.setClipRect (_ladder_rect.translated (2.5f * x, 0.f));
		_painter.translate (1.25f * x, posy);
		_painter.setBrush (Qt::NoBrush);
		_painter.setPen (_efis._autopilot_pen_1);
		_painter.drawPolygon (bug_shape);
		_painter.setPen (_efis._autopilot_pen_2);
		_painter.drawPolygon (bug_shape);
	}

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_mach_number (float x)
{
	if (!_efis._mach_visible)
		return;

	_painter.save();
	_painter.translate (0.f, 0.75f * x);

	QFont font_a = _efis._font_16_bold;
	QFont font_b = _efis._font_10_bold;

	QString m_str = "M";
	QString mach_str = " " + QString ("%1").arg (_mach, 0, 'f', 3);

	QRectF nn_rect (0.f, _ladder_rect.bottom(), QFontMetrics (font_a).width (mach_str), 1.2f * _efis._font_16_digit_height);
	QRectF zz_rect (0.f, nn_rect.top(), QFontMetrics (font_b).width (m_str), nn_rect.height());
	zz_rect.moveLeft (-0.5f * (zz_rect.width() + nn_rect.width()));
	// Correct position of zz_rect to get correct baseline position:
	zz_rect.translate (0.f, QFontMetrics (font_b).descent() - QFontMetrics (font_a).descent());
	nn_rect.moveLeft (zz_rect.right());

	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.f));
	_painter.setFont (font_a);
	_text_painter.drawText (nn_rect, Qt::AlignBottom | Qt::AlignLeft, mach_str, true);
	_painter.setFont (font_b);
	_text_painter.drawText (zz_rect, Qt::AlignBottom | Qt::AlignRight, m_str);

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_ap_setting (float)
{
	if (!_efis._at_speed_visible)
		return;

	QFont actual_speed_font = _efis._font_20_bold;
	float const digit_width = _efis._font_20_digit_width;
	float const digit_height = _efis._font_20_digit_height;

	int digits = 4;
	float const margin = 0.2f * digit_width;

	QRectF digits_box (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	QRectF box_rect (_ladder_rect.right() - digits_box.width(), _ladder_rect.top() - 1.4f * digits_box.height(),
					 digits_box.width(), digits_box.height());

	_painter.save();

	_painter.setPen (_efis.get_pen (QColor (0, 0, 0), 0.5f));
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawRect (box_rect);

	_painter.setPen (_efis.get_pen (_efis._autopilot_color, 1.f));
	_painter.setFont (actual_speed_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	_painter.drawText (box, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (_efis._at_speed))));

	_painter.restore();
}


EFISWidget::AttitudeDirectorIndicator::AttitudeDirectorIndicator (EFISWidget& efis, QPainter& painter):
	_efis (efis),
	_painter (painter),
	_text_painter (_painter, &_efis._text_painter_cache),
	_pitch (_efis._pitch),
	_roll (_efis._roll),
	_heading (_efis._heading)
{
	float p = floored_mod (_pitch + 180.0, 360.0) - 180.0;
	float r = floored_mod (_roll + 180.0, 360.0) - 180.0;
	float hdg = floored_mod (_heading, 360.0);

	// Mirroring, eg. -180° pitch is the same
	// as 0° pitch with roll inverted:
	if (p < -90.f)
	{
		p = -180.f - p;
		r = +180.f - r;
	}
	else if (p > 90.f)
	{
		p = +180.f - p;
		r = +180.f - r;
	}

	_pitch_transform.reset();
	_pitch_transform.translate (0.f, -_efis.pitch_to_px (p));

	_roll_transform.reset();
	_roll_transform.rotate (-r);

	_heading_transform.reset();
	_heading_transform.translate (-_efis.heading_to_px (hdg), 0.f);

	// Total transform of horizon (heading is not really necessary here):
	_horizon_transform = _pitch_transform * _roll_transform;
}


void
EFISWidget::AttitudeDirectorIndicator::paint()
{
	paint_horizon();
	paint_flight_path_marker();
	paint_pitch();
	paint_roll();
	paint_heading();
}


void
EFISWidget::AttitudeDirectorIndicator::paint_horizon()
{
	_painter.save();

	if (_efis._pitch_visible && _efis._roll_visible)
	{
		_painter.setTransform (_horizon_transform * _efis._center_transform);

		float const max = std::max (_efis.width(), _efis.height());
		float const w_max = 2.f * max;
		float const h_max = 10.f * max;
		// Sky and ground:
		_painter.fillRect (-w_max, -h_max, 2.f * w_max, h_max + 1.f, QBrush (_efis._sky_color, Qt::SolidPattern));
		_painter.fillRect (-w_max, 0.f, 2.f * w_max, h_max, QBrush (_efis._ground_color, Qt::SolidPattern));

	}
	else
	{
		_painter.resetTransform();
		_painter.setPen (Qt::NoPen);
		_painter.setBrush (QBrush (QColor (0, 0, 0)));
		_painter.drawRect (_efis.rect());
	}

	_painter.restore();
}


// TODO refactor
void
EFISWidget::AttitudeDirectorIndicator::paint_pitch()
{
	if (!_efis._pitch_visible)
		return;

	float const w = _efis.wh() * 2.f / 9.f;
	float const z = 0.5f * w;
	float const fpxs = _efis._font_10_bold.pixelSize();

	// TODO cache pens, name them correctly

	_painter.save();

	// Clip rectangle before and after rotation:
	_painter.setClipPath (get_pitch_scale_clipping_path() - _flight_path_marker);
	_painter.setTransform (_roll_transform * _efis._center_transform);
	_painter.setClipRect (QRectF (-w, -0.9f * w, 2.f * w, 2.2f * w), Qt::IntersectClip);
	_painter.setTransform (_horizon_transform * _efis._center_transform);
	_painter.setFont (_efis._font_10_bold);

	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.f));
	// 10° lines, exclude +/-90°:
	for (int deg = -90; deg <= 90; deg += 10)
	{
		if (deg == 0)
			continue;
		float d = _efis.pitch_to_px (deg);
		_painter.drawLine (QPointF (-z, d), QPointF (z, d));
		// Degs number:
		int abs_deg = std::abs (deg);
		QString deg_t = QString::number (abs_deg > 90 ? 180 - abs_deg : abs_deg);
		// Text:
		QRectF lbox (-z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		QRectF rbox (+z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		_text_painter.drawText (lbox, Qt::AlignVCenter | Qt::AlignRight, deg_t);
		_text_painter.drawText (rbox, Qt::AlignVCenter | Qt::AlignLeft, deg_t);
	}
	// 5° lines:
	for (int deg = -90; deg <= 90; deg += 5)
	{
		if (deg % 10 == 0)
			continue;
		float d = _efis.pitch_to_px (deg);
		_painter.drawLine (QPointF (-z / 2.f, d), QPointF (z / 2.f, d));
	}
	// 2.5° lines:
	for (int deg = -900; deg <= 900; deg += 25)
	{
		if (deg % 50 == 0)
			continue;
		float d = _efis.pitch_to_px (deg / 10.f);
		_painter.drawLine (QPointF (-z / 4.f, d), QPointF (z / 4.f, d));
	}

	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.75f));
	// -90°, 90° lines:
	for (float deg: { -90.f, 90.f })
	{
		float d = _efis.pitch_to_px (deg);
		_painter.drawLine (QPointF (-z, d), QPointF (z, d));
	}

	_painter.restore();
}


void
EFISWidget::AttitudeDirectorIndicator::paint_roll()
{
	if (!_efis._roll_visible)
		return;

	float const w = _efis.wh() * 3.f / 9.f;
	bool const bank_angle_warning = _efis._roll_limit > 0.f && std::abs (_efis._roll) > _efis._roll_limit;
	bool const slip_skid_warning = _efis._slip_skid_limit > 0.f && std::abs (_efis._slip_skid) > _efis._slip_skid_limit;

	_painter.save();

	QPen pen = _efis.get_pen (QColor (255, 255, 255), 1.f);
	_painter.setPen (pen);
	_painter.setBrush (QBrush (QColor (255, 255, 255)));

	QPen warning_pen = pen;
	warning_pen.setColor (_efis._warning_color_2);

	_painter.setTransform (_efis._center_transform);
	_painter.setClipRect (QRectF (-w, -w, 2.f * w, 2.25f * w));
	for (float deg: { -60.f, -45.f, -30.f, -20.f, -10.f, 0.f, +10.f, +20.f, +30.f, +45.f, +60.f })
	{
		_painter.setTransform (_efis._center_transform);
		_painter.rotate (1.f * deg);
		_painter.translate (0.f, -0.795f * w);

		if (deg == 0.f)
		{
			// Triangle:
			QPointF p0 (0.f, 0.f);
			QPointF px (0.025f * w, 0.f);
			QPointF py (0.f, 0.05f * w);
			_painter.drawPolygon (QPolygonF() << p0 << p0 - px - py << p0 + px - py);
		}
		else
		{
			float length = -0.05f * w;
			if (std::abs (std::fmod (deg, 60.f)) < 1.f)
				length *= 1.6f;
			else if (std::abs (std::fmod (deg, 30.f)) < 1.f)
				length *= 2.2f;
			_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, length));
		}
	}

	float const bold_width = _efis.pen_width (3.f);
	QPointF a (0, 0.01f * w); // Miter
	QPointF b (-0.062f * w, 0.1f * w);
	QPointF c (+0.062f * w, 0.1f * w);
	QPointF x0 (0.002f * w, 0.f);
	QPointF y0 (0.f, 0.005f * w);
	QPointF y1 (0.f, 1.f * bold_width);

	_painter.setTransform (_roll_transform * _efis._center_transform);
	_painter.translate (0.f, -0.79f * w);

	QPolygonF bank_angle_polygon = QPolygonF() << b << a << c << b;

	if (bank_angle_warning)
	{
		_painter.setPen (warning_pen);
		_painter.setBrush (warning_pen.color());
		_painter.drawPolygon (bank_angle_polygon);
	}
	else
	{
		_painter.setPen (pen);
		_painter.drawPolyline (bank_angle_polygon);
	}

	if (_efis._slip_skid_visible)
	{
		QPolygonF slip_skid_polygon = QPolygonF()
			<< b - x0 + y0
			<< b - x0 + y1
			<< c + x0 + y1
			<< c + x0 + y0
			<< b - x0 + y0;

		_painter.translate (-bound (_efis._slip_skid, -4.f, +4.f) * 0.08f * w, 0.f);

		if (bank_angle_warning || slip_skid_warning)
			_painter.setPen (warning_pen);
		else
			_painter.setPen (pen);

		if (slip_skid_warning)
		{
			_painter.setBrush (warning_pen.color());
			_painter.drawPolygon (slip_skid_polygon);
		}
		else
			_painter.drawPolyline (slip_skid_polygon);
	}

	_painter.restore();
}


// TODO refactor
void
EFISWidget::AttitudeDirectorIndicator::paint_heading()
{
	float const w = _efis.wh() * 2.25f / 9.f;
	float const fpxs = _efis._font_10_bold.pixelSize();

	if (!_efis._pitch_visible || !_efis._roll_visible)
		return;

	_painter.save();
	// Clip rectangle before and after rotation:
	_painter.setTransform (_efis._center_transform);
	_painter.setClipPath (get_pitch_scale_clipping_path());
	_painter.setTransform (_roll_transform * _efis._center_transform);
	_painter.setClipRect (QRectF (-1.1f * w, -0.8f * w, 2.2f * w, 1.9f * w), Qt::IntersectClip);
	_painter.setTransform (_horizon_transform * _efis._center_transform);
	_painter.setFont (_efis._font_10_bold);

	_painter.setTransform (_horizon_transform * _efis._center_transform);
	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.25f));
	_painter.drawLine (QPointF (-1.25 * w, 0.f), QPointF (1.25f * w, 0.f));
	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.f));

	if (!_efis._heading_visible)
	{
		_painter.restore();
		return;
	}

	_painter.setTransform (_heading_transform * _horizon_transform * _efis._center_transform);
	for (int deg = -360; deg < 450; deg += 10)
	{
		float d10 = _efis.heading_to_px (deg);
		float d05 = _efis.heading_to_px (deg + 5);
		// 10° lines:
		_painter.drawLine (QPointF (d10, -w / 18.f), QPointF (d10, 0.f));
		if (_efis._heading_numbers_visible)
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
			_text_painter.drawText (QRectF (d10 - 2.f * fpxs, 0.05f * fpxs, 4.f * fpxs, fpxs),
									Qt::AlignVCenter | Qt::AlignHCenter, text);
		}
		// 5° lines:
		_painter.drawLine (QPointF (d05, -w / 36.f), QPointF (d05, 0.f));
	}

	_painter.restore();
}


void
EFISWidget::AttitudeDirectorIndicator::paint_flight_path_marker()
{
	if (!_efis._flight_path_visible)
		return;

	float x = 0.013f * _efis.wh();
	float w = _efis.pen_width (3.f);
	float r = 0.5f * w;

	QPointF marker_position (-_efis.heading_to_px (_efis._flight_path_beta), -_efis.pitch_to_px (_efis._flight_path_alpha));
	_painter.save();

	_flight_path_marker = QPainterPath();
	_flight_path_marker.setFillRule (Qt::WindingFill);
	_flight_path_marker.addEllipse (QRectF (-x - 0.5f * w, -x - 0.5f * w, 2.f * x + w, 2.f * x + w));
	_flight_path_marker.addRoundedRect (QRectF (-4.f * x - 0.5f * w, -0.5f * w, +3.f * x + w, w), r, r);
	_flight_path_marker.addRoundedRect (QRectF (+1.f * x - 0.5f * w, -0.5f * w, +3.f * x + w, w), r, r);
	_flight_path_marker.addRoundedRect (QRectF (-0.5f * w, -2.f * x - 0.5f * w, w, x + w), r, r);
	_flight_path_marker.translate (marker_position);

	auto draw_marker = [&]() -> void {
		_painter.drawEllipse (QRectF (-x, -x, 2.f * x, 2.f * x));
		_painter.drawLine (QPointF (+x, 0.f), QPointF (+4.f * x, 0.f));
		_painter.drawLine (QPointF (-x, 0.f), QPointF (-4.f * x, 0.f));
		_painter.drawLine (QPointF (0.f, -x), QPointF (0.f, -2.f * x));
	};

	_painter.setClipRect (QRectF (-0.325f * _efis.wh(), -0.4f * _efis.wh(), 0.65f * _efis.wh(), 0.8f * _efis.wh()));
	_painter.translate (marker_position);
	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.25f));
	draw_marker();

	_painter.restore();
}


QPainterPath
EFISWidget::AttitudeDirectorIndicator::get_pitch_scale_clipping_path() const
{
	float const w = _efis.wh() * 2.f / 9.f;

	QPainterPath clip_path;
	clip_path.setFillRule (Qt::WindingFill);
	clip_path.addEllipse (QRectF (-1.15f * w, -1.175f * w, 2.30f * w, 2.35f * w));
	clip_path.addRect (QRectF (-1.15f * w, 0.f, 2.30f * w, 1.375f * w));

	return clip_path - _flight_path_marker;
}


EFISWidget::EFISWidget (QWidget* parent):
	InstrumentWidget (parent, 0.8f, 1.f)
{
	setAttribute (Qt::WA_NoBackground);
	_sky_color.setHsv (213, 217, 255);
	_ground_color.setHsv (30, 235, 122);
	_ladder_color = QColor (64, 51, 108, 0x80);
	_ladder_border_color = _ladder_color.darker (125);
	_warning_color_1 = QColor (255, 150, 0);
	_warning_color_2 = QColor (255, 200, 50);

	_blinking_warning = new QTimer (this);
	_blinking_warning->setInterval (200);
	QObject::connect (_blinking_warning, SIGNAL (timeout()), this, SLOT (blink()));
}


void
EFISWidget::paintEvent (QPaintEvent* paint_event)
{
	if (_speed_visible &&
		((_warning_speed_visible && _speed < _warning_speed) ||
		 (_minimum_speed_visible && _speed < _minimum_speed) ||
		 (_maximum_speed_visible && _speed > _maximum_speed)))
	{
		if (!_blinking_warning->isActive())
		{
			_blinking_warning->start();
			_blink = true;
		}
	}
	else if (_blinking_warning->isActive())
		_blinking_warning->stop();

	float const w = width();
	float const h = height();

	_center_transform.reset();
	_center_transform.translate (w / 2.f, h / 2.f);

	// Draw on buffer:
	QPixmap buffer (w, h);
	QPainter painter (&buffer);
	TextPainter text_painter (painter);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);
	painter.setTransform (_center_transform);

	if (_input_alert_visible)
		paint_input_alert (painter);
	else
	{
		painter.save();
		AttitudeDirectorIndicator adi (*this, painter);
		adi.paint();
		painter.restore();

		paint_center_cross (painter);
		paint_flight_director (painter);
		paint_altitude_agl (painter);
		paint_nav (painter, text_painter);

		painter.save();
		SpeedLadder sl (*this, painter);
		painter.translate (-0.4f * wh(), 0.f);
		sl.paint();
		painter.restore();

		painter.save();
		AltitudeLadder al (*this, painter);
		painter.translate (+0.4f * wh(), 0.f);
		al.paint();
		painter.restore();
	}

	// Copy buffer to screen:
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), buffer, paint_event->rect());
}


void
EFISWidget::paint_center_cross (QPainter& painter)
{
	float const w = wh() * 3.f / 9.f;

	QPen white_pen = get_pen (QColor (255, 255, 255), 1.5f);

	painter.save();

	painter.setTransform (_center_transform);
	painter.setPen (white_pen);
	painter.setBrush (QBrush (QColor (0, 0, 0)));

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

	painter.drawPolygon (a);
	painter.drawPolygon (b);
	painter.scale (-1.f, 1.f);
	painter.drawPolygon (b);

	painter.restore();
}


void
EFISWidget::paint_flight_director (QPainter& painter)
{
	float const w = wh() * 1.4f / 9.f;
	Degrees range = _fov / 4.f;

	Degrees pitch = bound (_flight_director_pitch - _pitch, -range, +range);
	Degrees roll = bound (_flight_director_roll - _roll, -range, +range);

	float ypos = pitch_to_px (pitch);
	float xpos = heading_to_px (roll) / 2.f;

	painter.save();
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

	painter.restore();
}


void
EFISWidget::paint_altitude_agl (QPainter& painter)
{
	if (!_altitude_agl_visible)
		return;

	float aagl = bound (_altitude_agl, -9999.f, +99999.f);
	QFont radar_altimeter_font = _font_20_bold;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;

	int digits = 4;
	if (_altitude_agl > 9999)
		digits = 5;
	float const margin = 0.2f * digit_width;

	QRectF box_rect (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	box_rect.translate (-box_rect.width() / 2.f, 0.35f * wh());

	painter.save();

	painter.setPen (get_pen (QColor (0, 0, 0), 1.f));
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (box_rect);

	painter.setPen (get_pen (QColor (255, 255, 255), 1.f));
	painter.setFont (radar_altimeter_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	painter.drawText (box, Qt::AlignVCenter | Qt::AlignHCenter, QString ("%1").arg (std::round (aagl)));

	painter.restore();
}


void
EFISWidget::paint_nav (QPainter& painter, TextPainter& text_painter)
{
	if (_dme_distance_visible)
	{
		QString dme_val = QString ("DME %1").arg (_dme_distance, 0, 'f', 1);
		QFont font = _font_10_bold;
		font.setBold (false);
		QFontMetrics font_metrics (font);
		QRectF rect (-0.24f * wh(), -0.36f * wh(), font_metrics.width (dme_val), font_metrics.height());
		painter.save();
		painter.setPen (QColor (255, 255, 255));
		painter.setFont (font);
		text_painter.drawText (rect, Qt::AlignLeft | Qt::AlignVCenter, dme_val);
		painter.restore();
	}

	if (_navigation_hint != "")
	{
		QFont font = _font_16_bold;
		font.setBold (false);
		QFontMetrics font_metrics (font);
		QRectF rect (-0.24f * wh(), -0.32f * wh(), font_metrics.width (_navigation_hint), font_metrics.height());
		painter.save();
		painter.setPen (QColor (255, 255, 255));
		painter.setFont (font);
		text_painter.drawText (rect, Qt::AlignLeft | Qt::AlignVCenter, _navigation_hint);
		painter.restore();
	}

	if (_navigation_needles_visible)
	{
		painter.save();

		QPen ladder_pen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
		QPen white_pen = get_pen (QColor (255, 255, 255), 1.8f);

		auto paint_ladder = [&](bool needle_visible, float track_deviation) -> void
		{
			track_deviation = bound (track_deviation, -1.f, 1.f);

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
				diamond.translate (track_deviation * 0.15f * wh(), 0.f);
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

		painter.save();
		painter.translate (0.f, 0.452f * wh());
		paint_ladder (_navigation_hd_needle_visible, _navigation_hd_needle);
		painter.restore();

		painter.save();
		painter.translate (0.28f * wh(), 0.f);
		painter.rotate (-90);
		paint_ladder (_navigation_gs_needle_visible, _navigation_gs_needle);
		painter.restore();

		painter.restore();
	}

	if (_navigation_runway_visible)
	{
		float w = 0.10f * wh();
		float h = 0.05f * wh();
		float p = 1.3f;
		float offset = bound (_navigation_hd_needle, -1.f, +1.f);

		painter.save();
		painter.translate (0.f, 0.28f * wh());

		QPointF tps[] = { QPointF (-w, 0.f), QPointF (0.f, 0.f), QPointF (+w, 0.f) };
		QPointF bps[] = { QPointF (-w * p, h), QPointF (0.f, h), QPointF (+w * p, h) };

		for (QPointF& point: tps)
			point += QPointF (2.5f * w * offset, 0);
		for (QPointF& point: bps)
			point += QPointF (2.5f * p * w * offset, 0);

		QRectF clip (-2.5f * w, -0.2f * h, 5.f * w, 1.4f * h);
		painter.setClipRect (clip);

		QPolygonF runway = QPolygonF() << tps[0] << tps[2] << bps[2] << bps[0];

		painter.setBrush (Qt::NoBrush);
		for (auto pen: { QPen (_navigation_color.darker (400), pen_width (2.f), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin),
						 QPen (_navigation_color, pen_width (1.33f), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin) })
		{
			painter.setPen (pen);
			painter.drawPolygon (runway);
			painter.drawLine (tps[1], bps[1]);
		}

		painter.restore();
	}
}


void
EFISWidget::paint_input_alert (QPainter& painter)
{
	painter.save();

	QFont font = _font;
	font.setPixelSize (font_size (30.f));
	font.setBold (true);

	QString alert = "NO INPUT";

	QFontMetrics font_metrics (font);
	int width = font_metrics.width (alert);

	QPen pen = get_pen (QColor (255, 255, 255), 2.f);

	painter.resetTransform();
	painter.setPen (Qt::NoPen);
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (rect());

	painter.setTransform (_center_transform);
	painter.setPen (pen);
	painter.setBrush (QBrush (QColor (0xdd, 0, 0)));
	painter.setFont (font);

	QRectF rect (-0.6f * width, -0.5f * font_metrics.height(), 1.2f * width, 1.2f * font_metrics.height());

	painter.drawRect (rect);
	painter.drawText (rect, Qt::AlignVCenter | Qt::AlignHCenter, alert);

	painter.restore();
}


void
EFISWidget::paint_dashed_zone (QPainter& painter, QColor const& color, QRectF const& target)
{
	QFontMetrics metrics = painter.font();
	float w = 0.7f * metrics.width ("0");
	float h = 0.65f * metrics.height();
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
	QFontMetrics font_metrics = font;
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
		else
			text_painter.drawText (x.first, Qt::AlignVCenter | Qt::AlignLeft, x.second);
	}

	painter.restore();
}


void
EFISWidget::paint_rotating_digit (QPainter& painter, TextPainter& text_painter,
								  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
								  bool two_zeros, bool zero_mark)
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

	QString sa = zero_mark && a == 0 ? (xa >= 0.f ? "G" : "R") : QString::number (a);
	QString sb = zero_mark && b == 0 ? (xb >= 0.f ? "G" : "R") : QString::number (b);
	QString sc = zero_mark && c == 0 ? (xc >= 0.f ? "G" : "R") : QString::number (c);

	if (std::abs (dtr) < delta && (two_zeros || std::abs (value) >= round_target / 2))
		pos = floored_mod (-dtr * (0.5f / delta), 1.f) - 0.5f;

	paint_rotating_value (painter, text_painter, box, pos, height_scale, sa, sb, sc);
}

