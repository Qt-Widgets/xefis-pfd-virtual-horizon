/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "power_indicator.h"


PowerIndicator::PowerIndicator (QDomElement const& config, QWidget* parent):
	Instrument (parent)
{
	_power_widget = new PowerWidget (this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_power_widget);

	bool found_properties_config = false;

	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "value", _value, true },
				{ "value-minimum", _range_minimum, true },
				{ "value-maximum", _range_maximum, true },
				{ "value-warning", _warning_value, false },
				{ "value-critical", _critical_value, false }
			});
			found_properties_config = true;
		}
	}

	if (!found_properties_config)
		throw Xefis::Exception ("module configuration missing");

	QTimer* t = new QTimer (this);
	t->setInterval (33);
	QObject::connect (t, SIGNAL (timeout()), this, SLOT (read()));
	t->start();
}


void
PowerIndicator::read()
{
	_power_widget->set_range (Range<double> { *_range_minimum, *_range_maximum });

	_power_widget->set_value (*_value);
	_power_widget->set_value_visible (_value.valid());

	if (!_warning_value.is_singular())
	{
		_power_widget->set_warning_value (*_warning_value);
		_power_widget->set_warning_visible (_warning_value.valid());
	}

	if (!_critical_value.is_singular())
	{
		_power_widget->set_critical_value (*_critical_value);
		_power_widget->set_critical_visible (_critical_value.valid());
	}
}

