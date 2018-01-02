#include <utils/NetOrigin.h>

#include <QJsonObject>

NetOrigin* NetOrigin::instance = nullptr;

NetOrigin::NetOrigin(QObject* parent, Logger* log)
	: QObject(parent)
	, _log(log)
	, _internetAccessAllowed(false)
	, _ipWhitelist()
{
	NetOrigin::instance = this;
}

const bool NetOrigin::accessAllowed(const QHostAddress& address, const QHostAddress& local)
{
	if(_internetAccessAllowed)
		return true;

	if(_ipWhitelist.contains(address)) // v4 and v6
		return true;

	if(address.protocol() == QAbstractSocket::IPv4Protocol)
	{
		if(!address.isInSubnet(local, 24)) // 255.255.255.xxx; IPv4 0-32
		{
			Warning(_log,"Client connection with IP address '%s' has been rejected! It's not whitelisted/internet access denied.",QSTRING_CSTR(address.toString()));
			return false;
		}
	}
	// IPv6 handling?!
	return true;
}

void NetOrigin::handleSettingsUpdate(const settings::type& type, const QJsonDocument& config)
{
	if(type == settings::NETWORK)
	{
		const QJsonObject& obj = config.object();
		_internetAccessAllowed = obj["internetAccessAPI"].toBool(false);

		const QJsonArray& arr = obj["ipWhitelist"].toArray();
		_ipWhitelist.clear();

		for(const auto& e : arr)
		{
			const QString& entry = e.toString("");
			if(entry.isEmpty())
				continue;

			QHostAddress host(entry);
			if(host.isNull())
			{
				Error(_log,"The whitelisted IP address '%s' isn't valid! Skipped",QSTRING_CSTR(entry));
				continue;
			}
			_ipWhitelist << host;
		}
	}
}
