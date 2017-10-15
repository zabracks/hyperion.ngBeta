#pragma once

// hyperion
#include <utils/Logger.h>
#include <hyperion/Hyperion.h>

// proj
#include <plugin/Files.h>

// qt
#include <QList>

class Plugin;
typedef struct _ts PyThreadState;

class Plugins : public QObject
{
	Q_OBJECT

public:
	///
	/// Plugin constructor
	///
	Plugins();
	~Plugins();

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
	/// Files instance
	Files _files;

	PyThreadState* _mainThreadState;
	/// start or restart a plugin


	/// stop a plugin
	void stop(QString id);

	QMap<QString,Plugin*> _runningPlugins;

	QStringList _restartQueue;

private slots:
	/// is called when a pluginAction is ongoing
	void doPluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

	/// is called when a plugin thread exits
	void pluginFinished();


	void start(QString id="service.kodi");
};
