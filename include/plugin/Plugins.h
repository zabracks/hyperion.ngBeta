#pragma once

// hyperion
#include <utils/Logger.h>
#include <hyperion/Hyperion.h>

// proj
#include <plugin/Files.h>

// qt
#include <QList>

class Plugin;
class PluginTable;
class PriorityMuxer;

///
/// @brief Manages everything related to Plugins
///
class Plugins : public QObject
{
	Q_OBJECT

public:
	///
	/// Plugin constructor
	///
	Plugins(Hyperion* hyperion, const quint8& instance);
	~Plugins();

	QMap<QString, PluginDefinition> getInstalledPlugins(void) const { return _files.getInstalledPlugins(); };
	QMap<QString, PluginDefinition> getAvailablePlugins(void) const { return _files.getAvailablePlugins(); };

	bool isPluginRunning(const QString& id) const { return  _runningPlugins.contains(id); };

	bool isPluginAutoUpdateEnabled(const QString& id) const;

	const QJsonValue getSettingsOfPlugin(const QString& id) { QMap<QString, PluginDefinition> inst = getInstalledPlugins(); const PluginDefinition& def =  inst.value(id); return def.settings; };

signals:
	///
	/// @brief emits whenever a plugin action is ongoing
	/// @param action   action from enum
	/// @param id       plugin id
	/// @param def      PluginDefinition (optional)
	/// @param success  true if action was a success, else false
	///
	void pluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

private:
	/// Logger instance
	Logger* _log;
	/// Hyperion instance
	Hyperion* _hyperion;

	/// PrioMuxer
	PriorityMuxer* _prioMuxer;

	/// database wrapper
	PluginTable* _PDB;

	/// Files instance
	Files _files;

	/// start or restart a plugin
	void start(QString id);

	/// stop a plugin by id, with remove flag, returns false if plugin is not running
	bool stop(const QString& id, const bool& remove = false) const;

	QMap<QString,Plugin*> _runningPlugins;

	QStringList _restartQueue;

public slots:
	/// is called from JsonAPI
	void doPluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

private slots:

	/// is called when a plugin thread exits
	void pluginFinished();
};
