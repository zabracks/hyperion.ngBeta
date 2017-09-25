#include <QMap>
#include <QJsonArray>
#include <QJsonObject>

struct PluginDefinition {
	QString         name;
	QString         description;
	QString         version;
	QJsonArray      dependencies;
	QJsonArray      changelog;
	QString         provider;
	QString         licence;
	QString         support;
	QString         source;
	QString         entryPy;
	QJsonObject     settingsSchema;
	QJsonObject     settings;
};
