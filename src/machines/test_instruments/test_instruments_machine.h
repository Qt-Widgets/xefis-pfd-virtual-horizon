/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_INSTRUMENTS_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_INSTRUMENTS_MACHINE_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/core/machine.h>
#include <xefis/core/xefis.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/navigation/navaid_storage.h>
#include <xefis/support/system/work_performer.h>
#include <xefis/utility/tracker.h>

// Local:
#include "test_loop.h"


class TestInstrumentsMachine: public xf::Machine
{
  public:
	// Ctor
	TestInstrumentsMachine (xf::Xefis*);

  private:
	xf::Logger								_logger;
	std::unique_ptr<xf::NavaidStorage>		_navaid_storage;
	std::unique_ptr<xf::Airframe>			_airframe;
	std::unique_ptr<xf::WorkPerformer>		_work_performer;
	std::optional<xf::Registrant<TestLoop>>	_test_loop;
};

#endif

