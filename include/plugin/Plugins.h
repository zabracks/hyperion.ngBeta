#pragma once

// hyperion
#include <utils/Logger.h>
#include <hyperion/Hyperion.h>
#include <plugin/PluginDefinition.h>

// qt
#include <QList>

class Plugin;
class PluginTable;
class PriorityMuxer;
class PluginFilesHandler;

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

	QMap<QString, PluginDefinition> getInstalledPlugins(void) const { return _installedPlugins; };
	QMap<QString, PluginDefinition> getAvailablePlugins(void);

	bool isPluginRunning(const QString& id) const { return  _runningPlugins.contains(id); };

	bool isPluginAutoUpdateEnabled(const QString& id) const;

	const QJsonValue getSettingsOfPlugin(const QString& id) { QMap<QString, PluginDefinition> inst = getInstalledPlugins(); const PluginDefinition& def =  inst.value(id); return def.settings; };

	///
	/// @brief stop a plugin by id, as blocking, returns false if plugin is not running
	/// Used from PluginFilesHandler only!
	/// @param id  The id of the plugin to stop
	/// @param blocking  If true the method retuns after the plugin has been stopped
	///
	bool stop(const QString& id, const bool& blocking = false) const;

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
	/// start or restart a plugin
	void start(QString id);

	///
	/// @brief Save settings for given plugin id in db, assigned with Hyperion instance
	/// @param  id        The plugin id
	/// @param  settings  The settings for this plugin
	///
	void saveSettings(const QString& id, const QJsonObject& settings);

	/// Logger instance
	Logger* _log;
	/// Hyperion instance
	Hyperion* _hyperion;

	/// PrioMuxer
	PriorityMuxer* _prioMuxer;

	/// database wrapper
	PluginTable* _PDB;

	/// Plugin files handler
	PluginFilesHandler* _pluginFilesHandler;

	// instance specific copy of PluginFilesHandler with injected settings
	QMap<QString, PluginDefinition> _installedPlugins;

	QMap<QString,Plugin*> _runningPlugins;

	QStringList _restartQueue;

public slots:
	/// is called from JsonAPI
	void doPluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

private slots:
	/// is called when a plugin thread exits
	void pluginFinished();
};
