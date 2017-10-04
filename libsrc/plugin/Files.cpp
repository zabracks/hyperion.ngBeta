// project include
#include <plugin/Files.h>
#include <utils/JsonUtils.h>
#include <plugin/HTTPUtils.h>
// qt includes
#include <QDir>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>


Files::Files(Logger* log, const QString& rootPath)
	: QObject()
	, _log(log)
	, _http(new HTTPUtils(log))
{
	// create folder structure
	_pluginsDir = rootPath + "/plugins";
	_packageDir = _pluginsDir + "/packages";
	_configDir = _pluginsDir + "/config";

	QDir dir;
	dir.mkpath(_packageDir);
	dir.mkpath(_configDir);

	// listen for http utils reply
	connect(_http, &HTTPUtils::replyReceived, this, &Files::replyReceived);
	updateAvailablePlugins();
	updateInstalledPlugins();
}

Files::~Files()
{
	delete _http;
}

void Files::replyReceived(bool success, int type, QString url, QByteArray data)
{
	qDebug()<<"REPLY RECIEVED: success,type,url"<<success<<type<<url;
	if(url == _pUrl && success && type == 2)
	{
		// updateAvailablePlugins map
		QJsonArray arr;
		if(!JsonUtils::parse(url, QString(data), arr, _log))
			return;

		for(const auto & e : arr)
		{
			//qDebug()<<"Entry of Array:" << e;
		}
	}
}

bool Files::installPlugin(const QString& id)
{
return true;
	// verify the plugin exists

}

bool Files::removePlugin(const QString& id)
{
return true;
}

void Files::updateAvailablePlugins(void)
{
	_http->sendGet(_pUrl);
}


void Files::updateInstalledPlugins(void)
{
	_installedPlugins.clear();
	QDir dir(_pluginsDir);
	QStringList filters;
	filters << "service.*" << "module.*";
	dir.setNameFilters(filters);
	QStringList pluginDirs = dir.entryList(QDir::Dirs);

	// iterate through each plugin folder
	for (const auto & pluginDir : pluginDirs)
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

			// get settings if available
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
