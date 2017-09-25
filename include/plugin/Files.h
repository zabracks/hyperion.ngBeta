#pragma once

#include <utils/Logger.h>
#include <plugin/PluginDefinition.h>

class Files : public QObject
{
	Q_OBJECT

public:
	///
	/// Files constructor
	///
	Files(Logger* log);
	~Files();

private:
	/// Logger instance
	Logger* _log;

	/// plugins, package, config dir
	QString _pluginsDir;
	QString _packageDir;
	QString _configDir;

	/// List of installed plugins <id,definition>
	QMap<QString, PluginDefinition> _installedPlugins;
	//QList _enabledPlugins;
	void updateInstalledPlugins(void);

};
