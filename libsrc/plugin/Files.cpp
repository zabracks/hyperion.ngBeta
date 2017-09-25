// project include
#include <plugin/Files.h>
#include <utils/JsonUtils.h>

// qt includes
#include <QDir>
#include <QJsonObject>
#include <QDebug>


Files::Files(Logger* log)
	: QObject()
	, _log(log)
{
	// create folder structure
	_pluginsDir = QDir::homePath() + "/.hyperion/plugins";
	_packageDir = _pluginsDir + "/packages";
	_configDir = _pluginsDir + "/config";

	QDir dir;
	dir.mkpath(_packageDir);
	dir.mkpath(_configDir);

	updateInstalledPlugins();
}

Files::~Files()
{

}

void Files::updateInstalledPlugins(void)
{
	_installedPlugins.clear();
	QDir dir(_pluginsDir);
	QStringList filters("service.*");
	dir.setNameFilters(filters);
	QStringList pluginDirs = dir.entryList(QDir::Dirs);

	// iterate through each plugin folder
	for (const auto pluginDir : pluginDirs)
	{
		// get plugin.json for meta data
		QString pd(_pluginsDir+"/"+pluginDir);
		QJsonObject metaObj;
		if(!JsonUtils::readFile(pd+"/plugin.json", metaObj, _log))
			continue;

		// if the plugin is a service, more data is required
		QString id = metaObj["id"].toString();
		QJsonObject settingsSchemaObj;
		QJsonObject settingsObj;
		if(id.startsWith("service."))
		{
			// get the settingsSchema.json
			if(!JsonUtils::readFile(pd+"/settingsSchema.json", settingsSchemaObj, _log))
				continue;

			// check if service.py is available
			if(!FileUtils::fileExists(pd+"/service.py",_log))
				continue;

			// get the settings if available
			JsonUtils::readFile(_configDir+"/"+id+".json", settingsObj, _log, true);

			// load translations??
		}

		// create the PluginDefinition
		PluginDefinition newDefinition{
			metaObj["name"].toString(),
			metaObj["description"].toString(),
			metaObj["version"].toString(),
			metaObj["dependencies"].toArray(),
			metaObj["changelog"].toArray(),
			metaObj["provider"].toString(),
			metaObj["licence"].toString(),
			metaObj["support"].toString(),
			metaObj["source"].toString(),
			pd+"/service.py",
			settingsSchemaObj,
			settingsObj
		};

		_installedPlugins.insert(id,newDefinition);
	}
	//qDebug()<<"OPJECT:"<<_installedPlugins;
}
