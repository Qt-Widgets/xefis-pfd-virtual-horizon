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

#ifndef XEFIS__MODULES__IO__UDP_H__INCLUDED
#define XEFIS__MODULES__IO__UDP_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtNetwork/QUdpSocket>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/module_io.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/utility/v2/actions.h>
#include <xefis/utility/logger.h>


class UDP_IO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<QString>			tx_udp_host			{ this, "tx_udp_host" }; // TODO express that this is optional
	xf::Setting<int>				tx_udp_port			{ this, "tx_udp_port" };
	xf::Setting<bool>				tx_interference		{ this, "tx_interference", false };
	xf::Setting<QString>			rx_udp_host			{ this, "rx_udp_host" }; // TODO OptionalSetting
	xf::Setting<int>				rx_udp_port			{ this, "rx_udp_port" };
	xf::Setting<bool>				rx_interference		{ this, "rx_interference", false };

	/*
	 * Input
	 */

	xf::PropertyIn<std::string>		send				{ this, "/send" };

	/*
	 * Output
	 */

	xf::PropertyOut<std::string>	receive				{ this, "/receive" };
};


class UDP:
	public QObject,
	public xf::Module<UDP_IO>
{
	Q_OBJECT

  private:
	static constexpr char kLoggerPrefix[] = "mod::UDP";

  public:
	// Ctor
	explicit
	UDP (std::unique_ptr<UDP_IO> module_io, xf::Logger const& parent_logger, std::string const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private slots:
	/**
	 * Called whenever there's data ready to be read from socket.
	 */
	void
	got_udp_packet();

  private:
	/**
	 * Interfere with packets for testing purposes.
	 */
	void
	interfere (QByteArray& blob);

  private:
	xf::Logger						_logger;
	QByteArray						_received_datagram;
	Unique<QUdpSocket>				_tx;
	Unique<QUdpSocket>				_rx;
	xf::PropChanged<std::string>	_send_changed	{ io.send };
};

#endif