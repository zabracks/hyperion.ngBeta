// system includes
#include <stdexcept>

// project includes
#include <boblightserver/BoblightServer.h>
#include "BoblightClientConnection.h"

// hyperion includes
#include <hyperion/Hyperion.h>
#include <bonjour/bonjourserviceregister.h>

// qt incl
#include <QTcpServer>

using namespace hyperion;

BoblightServer::BoblightServer(const QJsonObject& config)
	: QObject()
	, _hyperion(Hyperion::getInstance())
	, _server(new QTcpServer(this))
	, _openConnections()
	, _priority(0)
	, _log(Logger::getInstance("BOBLIGHT"))
	, _port(0)
{
	Debug(_log, "Instance created");

	// listen for component change
	connect(_hyperion, SIGNAL(componentStateChanged(hyperion::Components,bool)), this, SLOT(componentStateChanged(hyperion::Components,bool)));
	// listen new connection signal from server
	connect(_server, SIGNAL(newConnection()), this, SLOT(newConnection()));

	// init
	handleSettingsUpdate(config);
}

BoblightServer::~BoblightServer()
{
	stop();
}

void BoblightServer::start()
{
	if ( _server->isListening() )
		return;

	if (!_server->listen(QHostAddress::Any, _port))
	{
		throw std::runtime_error("BOBLIGHT ERROR: server could not bind to port");
	}
	Info(_log, "Started on port %d", _port);

	_hyperion->registerPriority("Boblight", _priority);
	_hyperion->getComponentRegister().componentStateChanged(COMP_BOBLIGHTSERVER, _server->isListening());

	if(_bonjourService == nullptr)
	{
		_bonjourService = new BonjourServiceRegister();
		_bonjourService->registerService("_hyperiond-bobl._tcp", _port);
	}
}

void BoblightServer::stop()
{
	if ( ! _server->isListening() )
		return;

	foreach (BoblightClientConnection * connection, _openConnections) {
		delete connection;
	}
	_server->close();
	Info(_log, "Stopped");
	_hyperion->unRegisterPriority("Boblight");
	_hyperion->getComponentRegister().componentStateChanged(COMP_BOBLIGHTSERVER, _server->isListening());
}

bool BoblightServer::active()
{
	return _server->isListening();
}

void BoblightServer::componentStateChanged(const hyperion::Components component, bool enable)
{
	if (component == COMP_BOBLIGHTSERVER)
	{
		if (_server->isListening() != enable)
		{
			if (enable) start();
			else        stop();
		}
	}
}

uint16_t BoblightServer::getPort() const
{
	return _server->serverPort();
}

void BoblightServer::newConnection()
{
	QTcpSocket * socket = _server->nextPendingConnection();

	if (socket != nullptr)
	{
		Info(_log, "new connection");
		BoblightClientConnection * connection = new BoblightClientConnection(socket, _priority);
		_openConnections.insert(connection);

		// register slot for cleaning up after the connection closed
		connect(connection, SIGNAL(connectionClosed(BoblightClientConnection*)), this, SLOT(closedConnection(BoblightClientConnection*)));
	}
}

void BoblightServer::closedConnection(BoblightClientConnection *connection)
{
	Debug(_log, "connection closed");
	_openConnections.remove(connection);

	// schedule to delete the connection object
	connection->deleteLater();
}

void BoblightServer::handleSettingsUpdate(const QJsonObject& obj)
{
	_port = obj["port"].toInt();
	_priority = obj["priority"].toInt();
	stop();
	if(obj["enable"].toBool())
		start();
}
