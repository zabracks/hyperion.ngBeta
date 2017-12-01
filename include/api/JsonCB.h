#pragma once

// qt incl
#include <QObject>
#include <QJsonObject>

// components def
#include <utils/Components.h>
// plugins def
#include <plugin/PluginDefinition.h>
// bonjour
#include <bonjour/bonjourrecord.h>

class ComponentRegister;
class Plugins;
class BonjourBrowserWrapper;

class JsonCB : public QObject
{
	Q_OBJECT

public:
	JsonCB(QObject* parent);

	///
	/// @brief Subscribe to future data updates given by cmd
	/// @param cmd   The cmd which will be subscribed for
	/// @return      True on success, false if not found
	///
	bool subscribeFor(const QString& cmd);

	///
	/// @brief Get all possible commands to subscribe for
	/// @return  The list of commands
	///
	QStringList getCommands() { return _availableCommands; };
	///
	/// @brief Get all subscribed commands
	/// @return  The list of commands
	///
	QStringList getSubscribedCommands() { return _subscribedCommands; };
signals:
	///
	/// @brief Emits whenever a new json mesage callback is ready to send
	/// @param The JsonObject message
	///
	void newCallback(QJsonObject);

private slots:
	///
	/// @brief handle component state changes
	///
	void handleComponentState(const hyperion::Components comp, const bool state);

	///
	/// @brief slot to handle plugin actions
	/// @param action   action from enum
	/// @param id       plugin id
	/// @param def      PluginDefinition (optional)
	/// @param success  true if action was a success, else false
	///
	void handlePluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

	///
	/// @brief handle emits from bonjour wrapper
	/// @param  bRegisters   The full register map
	///
	void handleBonjourChange(const QMap<QString,BonjourRecord>& bRegisters);

private:
	/// pointer of comp register
	ComponentRegister* _componentRegister;
	/// plugins pointer
	Plugins* _plugins;
	/// Bonjour instance
	BonjourBrowserWrapper* _bonjour;
	/// contains all available commands
	QStringList _availableCommands;
	/// contains active subscriptions
	QStringList _subscribedCommands;
	/// construct callback msg
	void doCallback(const QString& cmd, const QVariant& data);
};
