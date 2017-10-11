#pragma once

#include <QMap>
#include <QJsonArray>
#include <QJsonObject>

struct PluginDefinition {
	QString         name;
	QString         description;
	QString         version;
	QJsonObject     dependencies;
	QJsonArray      changelog;
	QString         provider;
	QString         support;
	QString         source;
	QString         entryPy;
	QJsonObject     settingsSchema;
	QJsonObject     settings;
};

enum PluginAction{
	P_INSTALL,
	P_INSTALLED,
	P_REMOVE,
	P_REMOVED,
	P_START,
	P_STARTED,
	P_STOP,
	P_STOPPED,
	P_SAVE,
	P_SAVED,
	P_UPD_AVAIL
};
