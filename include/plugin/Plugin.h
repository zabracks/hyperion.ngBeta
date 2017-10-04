#pragma once

#include <utils/Logger.h>
#include <plugin/Files.h>
#include <hyperion/Hyperion.h>

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
	/// Hyperion instance
	Hyperion* _hyperion;
	/// Files instance
	Files _files;

	/// List of available/enabled plugins
	//QList _availablePlugins;
	//QList _enabledPlugins;

private slots:
	/// update _availablePlugins
	void updatePluginList(void);
};
