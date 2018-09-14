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

// Standard:
#include <cstddef>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "vertical_trim.h"


VerticalTrim::VerticalTrim (std::unique_ptr<VerticalTrimIO> module_io, std::string_view const& instance):
	Instrument (std::move (module_io), instance)
{
	_inputs_observer.set_callback ([&]{
		mark_dirty();
	});
	_inputs_observer.observe (io.trim_value);
}


void
VerticalTrim::process (xf::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_time());
}


void
VerticalTrim::paint (xf::PaintRequest& paint_request) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);
	auto trim = io.trim_value.get_optional();

	if (trim)
		xf::clamp (*trim, -1.0, +1.0);

	auto ref = io.trim_reference.get_optional();
	auto ref_min = io.trim_reference_minimum.get_optional();
	auto ref_max = io.trim_reference_maximum.get_optional();

	double h = aids->font_2.digit_height;
	double v = aids->height() - h;
	bool within_reference = trim && ref_min && ref_max && (*ref_min <= *trim) && (*trim <= *ref_max);

	QFont nu_nd_font = aids->font_2.font;
	QFont label_font = aids->font_2.font;
	QFont value_font = aids->font_4.font;
	QFont reference_font = aids->font_2.font;
	QColor cyan = aids->kCyan;
	QTransform center_point_transform;
	center_point_transform.translate (0.65 * aids->width(), 0.5 * aids->height());

	// Scale line:
	QPointF nd (-h, 0.5 * (h - aids->height()));
	QPointF nu (-h, 0.5 * (aids->height() - h));
	QPolygonF line = QPolygonF()
		<< nd + QPointF (0.5 * h, 0.0)
		<< nd + QPointF (h, 0.0)
		<< nu + QPointF (h, 0.0)
		<< nu + QPointF (0.5 * h, 0.0);
	painter.setPen (aids->get_pen (Qt::white, 1.0));
	painter.setFont (nu_nd_font);
	painter.setTransform (center_point_transform);
	painter.drawPolyline (line);
	painter.drawLine (QPointF (-0.5 * h, 0.0), QPointF (+0.5 * h, 0.0));
	painter.fast_draw_text (nd - QPointF (0.25 * h, 0.0), Qt::AlignVCenter | Qt::AlignRight, "ND");
	painter.fast_draw_text (nu - QPointF (0.25 * h, 0.0), Qt::AlignVCenter | Qt::AlignRight, "NU");

	// Reference range:
	if (ref_min && ref_max)
	{
		painter.setPen (Qt::NoPen);
		painter.setBrush (Qt::green);
		painter.drawRect (QRectF (QPointF (aids->pen_width (0.5), -*ref_min * 0.5 * v),
									QPointF (aids->pen_width (5.0), -*ref_max * 0.5 * v)));
	}

	// Reference value:
	if (ref)
	{
		painter.setPen (aids->get_pen (aids->kAutopilotColor, 2.0));
		painter.paint (aids->default_shadow(), [&] {
			painter.drawLine (QPointF (aids->pen_width (0.5), -*ref * 0.5 * v),
							  QPointF (aids->pen_width (7.5), -*ref * 0.5 * v));
		});
	}

	// Cyan vertical text:
	painter.setFont (label_font);
	painter.setPen (cyan);
	painter.fast_draw_vertical_text (QPointF (1.5 * h, 0.0), Qt::AlignVCenter | Qt::AlignLeft, *io.label);

	// Pointer:
	if (trim)
	{
		QPolygonF triangle = QPolygonF()
			<< QPointF (-1.0 * h, -0.35 * h)
			<< QPointF (0.0, 0.0)
			<< QPointF (-1.0 * h, +0.35 * h);
		triangle << triangle[0];
		QColor color = within_reference ? Qt::green : Qt::white;
		painter.setPen (aids->get_pen (color, 1.0));
		painter.setBrush (color);
		painter.paint (aids->default_shadow(), [&] {
			painter.drawPolygon (triangle.translated (0.0, -*trim * 0.5 * v));
		});
	}

	// Numerical value:
	QString value_str = "   ";
	if (trim)
		value_str = stringify (-*trim);
	double x = 0.25 * h;
	QPointF text_hook = QPointF (-2.0 * h, 0.0);
	Qt::Alignment alignment = Qt::AlignVCenter | Qt::AlignRight;
	painter.setPen (aids->get_pen (within_reference ? Qt::green : Qt::white, 1.0));
	painter.setBrush (Qt::NoBrush);
	painter.setFont (value_font);
	QRectF box = painter.get_text_box (text_hook, alignment, value_str).adjusted (-x, 0.0, x, 0.0);
	painter.fast_draw_text (text_hook, alignment, value_str);
	painter.drawRect (box);

	// Numerical reference:
	if (ref)
	{
		QString ref_str = stringify (-*ref);
		painter.setPen (aids->get_pen (aids->kAutopilotColor, 1.0));
		painter.setFont (reference_font);
		painter.fast_draw_text (QPointF (box.center().x(), box.top()), Qt::AlignBottom | Qt::AlignHCenter, ref_str);
	}
}


QString
VerticalTrim::stringify (double value)
{
	QString result = QString::fromStdString ((boost::format ("%+03d") % (std::round (100.0 * value))).str());
	// Remove +/- sign when 00:
	if (result.mid (1, 2) == "00")
		result[0] = ' ';
	return result;
}

