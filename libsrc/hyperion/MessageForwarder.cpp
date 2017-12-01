// STL includes
#include <stdexcept>

#include <hyperion/MessageForwarder.h>

#include <utils/Logger.h>
#include <hyperion/Hyperion.h>

MessageForwarder::MessageForwarder(Hyperion* hyperion, const QJsonObject & config)
{
	if ( !config.isEmpty() && config["enable"].toBool(true) )
	{
		if ( !config["json"].isNull() && config["json"].isArray() )
		{
			const QJsonArray & addr = config["json"].toArray();
			for (signed i = 0; i < addr.size(); ++i)
			{
				Info(Logger::getInstance("Core"), "Json forward to %s", addr.at(i).toString().toStdString().c_str());
				addJsonSlave(addr[i].toString());
			}
		}

		if ( !config["proto"].isNull() && config["proto"].isArray() )
		{
			const QJsonArray & addr = config["proto"].toArray();
			for (signed i = 0; i < addr.size(); ++i)
			{
				Info(Logger::getInstance("Core"), "Proto forward to %s", addr.at(i).toString().toStdString().c_str());
				addProtoSlave(addr[i].toString());
			}
		}
	}
	// set initial state
	hyperion->getComponentRegister().componentStateChanged(hyperion::COMP_FORWARDER, config["enable"].toBool(true));
}

MessageForwarder::~MessageForwarder()
{
}


void MessageForwarder::addJsonSlave(QString slave)
{
	QStringList parts = slave.split(":");
	if (parts.size() != 2)
		throw std::runtime_error(QString("HYPERION (forwarder) ERROR: Wrong address: unable to parse address (%1)").arg(slave).toStdString());

	bool ok;
	quint16 port = parts[1].toUShort(&ok);
	if (!ok)
		throw std::runtime_error(QString("HYPERION (forwarder) ERROR: Wrong address: Unable to parse the port number (%1)").arg(parts[1]).toStdString());

	JsonSlaveAddress c;
	c.addr = QHostAddress(parts[0]);
	c.port = port;
	_jsonSlaves << c;
}

void MessageForwarder::addProtoSlave(QString slave)
{
	_protoSlaves << slave;
}

QStringList MessageForwarder::getProtoSlaves()
{
	return _protoSlaves;
}

QList<MessageForwarder::JsonSlaveAddress> MessageForwarder::getJsonSlaves()
{
	return _jsonSlaves;
}

bool MessageForwarder::protoForwardingEnabled()
{
	return ! _protoSlaves.empty();
}

bool MessageForwarder::jsonForwardingEnabled()
{
	return ! _jsonSlaves.empty();
}
