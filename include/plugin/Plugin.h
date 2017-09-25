#pragma once

#include <utils/Logger.h>
#include <plugin/Files.h>

#include <QList>

class Plugin : public QObject
{
	Q_OBJECT

public:
	///
	/// Plugin constructor
	///
	Plugin();
	~Plugin();

private:
	/// Logger instance
	Logger* _log;
	/// Files instance
	Files _files;

	/// plugins, package, config dir
	QString _pluginsDir;
	QString _packageDir;
	QString _configDir;

	/// List of available/enabled plugins
	//QList _availablePlugins;
	//QList _enabledPlugins;

private slots:
	/// update _availablePlugins
	void updatePluginList(void);
};
