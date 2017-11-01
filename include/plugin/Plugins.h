#pragma once

// hyperion
#include <utils/Logger.h>
#include <hyperion/Hyperion.h>

// proj
#include <plugin/Files.h>

// qt
#include <QList>

class Plugin;
class PDBWrapper;
typedef struct _ts PyThreadState;

class Plugins : public QObject
{
	Q_OBJECT

public:
	///
	/// Plugin constructor
	///
	Plugins(Hyperion* hyperion);
	~Plugins();

	QMap<QString, PluginDefinition> getInstalledPlugins(void) const { return _files.getInstalledPlugins(); };
	QMap<QString, PluginDefinition> getAvailablePlugins(void) const { return _files.getAvailablePlugins(); };

	bool isPluginRunning(const QString& id) const { return  _runningPlugins.contains(id); };

	bool isPluginAutoUpdateEnabled(const QString& id) const;

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

	/// database wrapper
	PDBWrapper* _PDB;

	/// Files instance
	Files _files;

	PyThreadState* _mainThreadState;

	/// start or restart a plugin


	/// stop a plugin by id, with remove flag, returns false if plugin is not running
	bool stop(const QString& id, const bool& remove = false) const;

	QMap<QString,Plugin*> _runningPlugins;

	QStringList _restartQueue;

public slots:
	/// is called from JsonProcessor
	void doPluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

private slots:

	/// is called when a plugin thread exits
	void pluginFinished();


	void start(QString id="service.kodi");
};
