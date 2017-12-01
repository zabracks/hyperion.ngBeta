// proj incl
#include <api/JsonCB.h>

// hyperion
#include <hyperion/Hyperion.h>
// components
#include <hyperion/ComponentRegister.h>
// plugins
#include <plugin/Plugins.h>
// bonjour wrapper
#include <bonjour/bonjourbrowserwrapper.h>

using namespace hyperion;

JsonCB::JsonCB(QObject* parent)
	: QObject(parent)
	, _componentRegister(& Hyperion::getInstance()->getComponentRegister())
	, _plugins(Hyperion::getInstance()->getPluginsInstance())
	, _bonjour(Hyperion::getInstance()->getBonjourInstance())
{
	_availableCommands << "components-update" << "plugins-update" << "sessions-update";
}

bool JsonCB::subscribeFor(const QString& type)
{
	if(!_availableCommands.contains(type))
		return false;

	if(type == "components-update")
	{
		_subscribedCommands << "components-update";
		connect(_componentRegister, &ComponentRegister::updatedComponentState, this, &JsonCB::handleComponentState, Qt::UniqueConnection);
	}

	if(type == "plugins-update")
	{
		_subscribedCommands << "plugins-update";
		connect(_plugins, &Plugins::pluginAction, this, &JsonCB::handlePluginAction, Qt::UniqueConnection);
	}

	if(type == "sessions-update")
	{
		_subscribedCommands << "sessions-update";
		connect(_bonjour, &BonjourBrowserWrapper::browserChange, this, &JsonCB::handleBonjourChange, Qt::UniqueConnection);
	}

	return true;
}

void JsonCB::doCallback(const QString& cmd, const QVariant& data)
{
	QJsonObject obj;
	obj["command"] = cmd;

	if(static_cast<QMetaType::Type>(data.type()) == QMetaType::QJsonArray)
		obj["data"] = data.toJsonArray();
	else
		obj["data"] = data.toJsonObject();

	emit newCallback(obj);
}

void JsonCB::handleComponentState(const hyperion::Components comp, const bool state)
{
	QJsonObject data;
	data["name"] = componentToIdString(comp);
	data["enabled"] = state;

	doCallback("components-update", QVariant(data));
}

void JsonCB::handlePluginAction(PluginAction action, QString id, bool success, PluginDefinition def)
{
	QJsonObject data, plug;
	bool send = false;

	// limit to service. plugins
	if(!id.startsWith("service."))
		return;
	
	switch(action)
	{
		case P_INSTALLED:
			if(success)
			{
				plug["name"] = def.name;
				plug["description"] = def.description;
				plug["version"] = def.version;
				data[id] = plug;
				send = true;
			}
			break;
		case P_REMOVED:
			if(success)
			{
				plug["removed"] = true;
				data["removed"] = true;
				data[id] = plug; // TODO a way to init a empty data {}, currently it's 'null'
				send = true;
			}
			break;
		default:
			break;
	}
	if(send)
		doCallback("components-update", QVariant(data));
}

void JsonCB::handleBonjourChange(const QMap<QString,BonjourRecord>& bRegisters)
{
	QJsonArray data;
	for (const auto & session: bRegisters)
	{
		if (session.port<0) continue;
		QJsonObject item;
		item["name"]   = session.serviceName;
		item["type"]   = session.registeredType;
		item["domain"] = session.replyDomain;
		item["host"]   = session.hostName;
		item["address"]= session.address;
		item["port"]   = session.port;
		data.append(item);
	}

	doCallback("sessions-update", QVariant(data));
}
