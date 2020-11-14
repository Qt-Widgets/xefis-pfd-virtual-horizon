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
#include <algorithm>
#include <functional>

// Qt:
#include <QtCore/QDir>
#include <QtCore/QTextStream>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/qt/qdom.h>
#include <neutrino/qt/qdom_iterator.h>
#include <neutrino/stdexcept.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/blob.h>
#include <xefis/utility/string.h>

// Local:
#include "state.h"


void
StateIO::register_socket (std::string const& unique_identifier, xf::BasicModuleOut& socket)
{
	if (auto f = _registered_sockets.find (unique_identifier); f != _registered_sockets.end())
		throw xf::Exception ("StateIO::register_socket: unique_identifier '" + unique_identifier + "' is not unique");

	_registered_sockets.emplace (unique_identifier, socket);
}


State::State (std::unique_ptr<StateIO> module_io, xf::Logger const& logger, std::string_view const& instance):
	Module (std::move (module_io), instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance))
{
	load_state();
}


State::~State()
{
	save_state();
	_save_future.wait();
}


void
State::process (xf::Cycle const&)
{
	for (auto& rp: io._registered_sockets)
	{
		if (rp.second.changed())
		{
			if (_save_future.valid())
			{
				if (_save_future.wait_for (std::chrono::seconds (0)) == std::future_status::ready)
				{
					try {
						_save_future.get();
					}
					catch (xf::Exception const& e)
					{
						_logger << "Encountered error when saving state: " << e.message() << std::endl;
					}

					save_state();
				}
				// TODO else enqueue again
			}
			else
				save_state();
		}
	}
}


void
State::load_state()
{
	try {
		QDir cwd;
		QString file_name = cwd.absolutePath() + '/' + QString::fromStdString (*io.file_name);
		QString content = do_load_state (file_name);

		QDomDocument doc;

		if (!doc.setContent (content, true))
			throw xf::BadConfiguration ("state file XML parse error" + file_name);

		if (doc.documentElement() == "xefis-mod-systems-state")
		{
			for (QDomElement const& e: xf::iterate_sub_elements (doc.documentElement()))
			{
				if (e == "state-variable")
				{
					auto id = e.attribute ("id").toStdString();

					if (auto rp = io._registered_sockets.find (id); rp != io._registered_sockets.end())
					{
						try {
							rp->second.socket.from_blob (xf::parse_hex_string (e.attribute ("value")));
						}
						catch (xf::Exception const& e)
						{
							_logger << "Failed to load setting '" << id << "': " << e.message() << std::endl;
						}
					}
					else
						_logger << "Ignoring not configured setting '" << id << "'" << std::endl;
				}
				else
					_logger << "Unknown element <" << e.tagName().toStdString() << ">";
			}
		}
	}
	catch (xf::Exception const& e)
	{
		_logger << "Error when loading state: " << e.message() << std::endl;
	}
}


void
State::save_state()
{
	try {
		QDomDocument doc;
		QDomElement root = doc.createElement ("xefis-mod-systems-state");
		doc.appendChild (root);

		for (auto const& rp: io._registered_sockets)
		{
			QDomElement rp_element = doc.createElement ("state-variable");
			rp_element.setAttribute ("id", QString::fromStdString (rp.first));
			rp_element.setAttribute ("value", QString::fromStdString (xf::to_hex_string (rp.second.socket.to_blob())));
			root.appendChild (rp_element);
		}

		// Wait for previous std::async to finish:
		if (_save_future.valid())
			_save_future.wait();

		_save_future = std::async (std::launch::async, do_save_state, doc.toString(), QString::fromStdString (*io.file_name));
	}
	catch (xf::Exception const& e)
	{
		_logger << "Error when saving state: " << e.message() << std::endl;
	}
	catch (std::exception const& e)
	{
		_logger << "System error: " << e.what() << std::endl;
	}
}


QString
State::do_load_state (QString file_name)
{
	QFile file (file_name);

	if (!file.exists())
		throw xf::BadConfiguration ("file '" + file.fileName() + "' not found");

	if (!file.open (QFile::ReadOnly))
		throw xf::IOError ("couldn't open '" + file.fileName() + "' for read: " + file.errorString());

	return file.readAll();
}


void
State::do_save_state (QString content, QString file_name)
{
	QDir cwd;
	QString target_file_name = cwd.absolutePath() + '/' + file_name;
	QString temp_file_name = target_file_name + '~';

	QFile file (temp_file_name);

	if (!file.open (QFile::WriteOnly))
		throw xf::IOError ("couldn't open '" + file.fileName() + "' for save: " + file.errorString());

	QTextStream ts (&file);
	ts << content;
	file.flush();
	file.close();
	if (::rename (temp_file_name.toUtf8(), target_file_name.toUtf8()) == -1)
	{
		char buf[256];
		static_cast<void> (strerror_r (errno, buf, std::size (buf)));
		throw xf::IOError (QString ("couldn't save settings file: %1").arg (buf));
	}
}

