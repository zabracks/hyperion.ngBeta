#pragma once

#include <ssdp/SSDPServer.h>

#include <QNetworkConfiguration>

class WebServer;
class QNetworkConfigurationManager;

///
/// Manage SSDP discovery. SimpleServiceDisoveryProtocol is the discovery subset of UPnP. Implemented is spec V1.0.
/// As SSDP requires a webserver, this class depends on it
/// UPnP 2.0: spec: http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v2.0.pdf
/// Detailed blog post: https://embeddedinn.wordpress.com/tutorials/upnp-device-architecture/
///

class SSDPHandler : public SSDPServer{
	Q_OBJECT
public:
	SSDPHandler(QObject * parent, WebServer* webserver);

public slots:
	///
	/// @brief get state changes from webserver
	///
	void handleWebServerStateChange(const bool newState);

private:
	///
	/// @brief Build http url for current ip:port/desc.xml
	///
	const QString getDescAddress();

	///
	/// @brief Get the base address
	///
	const QString getBaseAddress();

	///
	/// @brief Build the ssdp description (description.xml)
	///
	const QString buildDesc();

	///
	/// @brief Get the local address of interface
	/// @return the address, might be empty
	///
	const QString getLocalAddress();

private slots:
	///
	/// @brief Handle the mSeach request from SSDPServer
	/// @param target  The ST service type
	/// @param mx      Answer with delay in s
	/// @param address The ip of the caller
	/// @param port    The port of the caller
	///
	void handleMSearchRequest(const QString& target, const QString& mx, const QString address, const quint16 & port);

	void handleNetworkConfigurationChanged(const QNetworkConfiguration &config);

private:
	WebServer* _webserver;
	QString    _localAddress;
	QNetworkConfigurationManager* _NCA;
};
