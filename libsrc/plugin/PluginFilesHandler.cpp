// project
#include <plugin/PluginFilesHandler.h>
#include <db/PluginTable.h>
#include <plugin/Plugins.h>

// hyperion
#include <utils/JsonUtils.h>
#include <plugin/HTTPUtils.h>
#include <HyperionConfig.h>

// qt
#include <QDir>
#include <QJsonObject>
#include <QTimer>

// QuaZip
#include <JlCompress.h>

PluginFilesHandler* PluginFilesHandler::pfhInstance;

PluginFilesHandler::PluginFilesHandler(const QString& rootPath, QObject* parent)
	: QObject(parent)
	, _log(Logger::getInstance("PLUGINS"))
	, _PDB(new PluginTable(0,this))
	, _http(new HTTPUtils(_log))
{
	PluginFilesHandler::pfhInstance = this;

	// create folder structure
	_pluginsDir = rootPath + "/plugins";
	_packageDir = _pluginsDir + "/packages";

	QDir dir;
	dir.mkpath(_packageDir);

	// listen for http utils reply
	connect(_http, &HTTPUtils::replyReceived, this, &PluginFilesHandler::replyReceived);

	// init
	init();
}

PluginFilesHandler::~PluginFilesHandler()
{
	delete _http;
}

void PluginFilesHandler::init(void)
{
	// fetch plugins from plugin directory
	readInstalledPlugins();

	// update avail plugins every 24h
	_rTimer = new QTimer(this);
	_rTimer->setInterval(86400000);
	_rTimer->start();
	connect(_rTimer, &QTimer::timeout, this, &PluginFilesHandler::updateAvailablePlugins);
	updateAvailablePlugins();
}

void PluginFilesHandler::registerMe(Plugins* instance)
{
	if(_registeredInstances.contains(instance))
		_registeredInstances.removeAll(instance);
	else
		_registeredInstances.append(instance);
}

void PluginFilesHandler::doPluginAction(PluginAction action, QString id, bool success, PluginDefinition def)
{
	switch(action)
	{
		case P_INSTALL:
			installPlugin(id);
			break;
		case P_REMOVE:
			for(auto &entry : _registeredInstances)
			{
				// blocking stop
				entry->stop(id, true);
			}
			// now remove, the result will be emited
			removePlugin(id);
			break;
		default:
			break;
	}
}

void PluginFilesHandler::replyReceived(bool success, int type, QString id, QByteArray data)
{
	if(!success)
	{
		if(id == "P_AVAIL")
		{
			Error(_log,"Failed to download plugin repository");
			emit pluginAction(P_UPDATED_AVAIL, id, false);
		}
		else
		{
			Error(_log,"Failed to download plugin '%s'",QSTRING_CSTR(id));
		}
	}
	else
	{
		if(id == "P_AVAIL")
		{
			// updateAvailablePlugins
			QJsonArray arr;
			if(!JsonUtils::parse(id, QString(data), arr, _log))
			{
				Error(_log, "Failed to parse plugin repository");
				return;
			}

			_availablePlugins.clear();
			for(const auto & e : arr)
			{
				QJsonObject metaObj = e.toObject();
				PluginDefinition newDefinition;
				newDefinition.name = metaObj["name"].toString();
				newDefinition.description = metaObj["description"].toString();
				newDefinition.version = metaObj["version"].toString();
				newDefinition.dependencies = metaObj["dependencies"].toObject();
				newDefinition.changelog = metaObj["changelog"].toArray();
				newDefinition.provider = metaObj["provider"].toString();
				newDefinition.support = metaObj["support"].toString();
				newDefinition.source = metaObj["source"].toString();
				QString pid = metaObj["id"].toString();
				_availablePlugins.insert(pid,newDefinition);
			}
			// notify the update
			emit pluginAction(P_UPDATED_AVAIL, id, true);
			// check all installed plugins for updates
			doUpdateCheck();
		}

		if(id.contains("P_INSTALL"))
		{
			// assume a plugin to install
			QStringList sid = id.split(':');
			if(sid.size() != 2)
			{
				Error(_log,"No plugin id from request!")
				return;
			}

			// get PluginDefinition
			QString pid = sid.takeLast();
			PluginDefinition dev = _availablePlugins.value(pid);
			const QString fname(pid+"["+dev.version+"].zip");
			const QString fpath(_packageDir+"/"+fname);

			// write zip to filesystem
			if(!FileUtils::writeFile(fpath, data, _log))
			{
				Error(_log,"Failed to write zip for '%s'", QSTRING_CSTR(dev.name));
				emit pluginAction(P_INSTALLED, pid, false);
				return;
			}

			// Extract to plugin folder
			// QuaZip: http://quazip.sourceforge.net/annotated.html
			QStringList parts = JlCompress::extractDir(fpath, _pluginsDir+"/"+pid);
			if(parts.isEmpty())
			{
				Error(_log,"Extraction of plugin '%s' failed! The file seems to be corrupt.", QSTRING_CSTR(dev.name));
				emit pluginAction(P_INSTALLED, pid, false);
				return;
			}

			// try to update _installedPlugins
			if(!updateInstalledPlugin(pid))
			{
				Error(_log,"Plugin '%s' install/update failed.", QSTRING_CSTR(dev.name));
				emit pluginAction(P_INSTALLED, pid, false);
				return;
			}

			// notify with new definition
			PluginDefinition newDef = _installedPlugins.value(pid);
			Info(_log,"Plugin '%s' successfully installed/updated.", QSTRING_CSTR(dev.name));
			emit pluginAction(P_INSTALLED, pid, true, newDef);

			// bytearray to unzip
			//QBuffer buffer(&data);
			//buffer->open(QIODevice::ReadOnly);
			//QuaZip archive(buffer);
			//archive.setZipName(fname);
			//buffer->close();
			//archive.open(QuaZip::mdUnzip);
		}
	}
}

void PluginFilesHandler::doUpdateCheck(void)
{
	// iterate installed plugins
	QMap<QString, PluginDefinition>::const_iterator i = _installedPlugins.constBegin();
	while (i != _installedPlugins.constEnd()) {
	    QString id = i.key();
		PluginDefinition instDev = i.value();

		// during development a plugin won't exist in repo, don't try to update
		// check also for auto updates enabled
		if(_availablePlugins.contains(id) && _PDB->isPluginAutoUpdateEnabled(id))
		{
			PluginDefinition availDev = _availablePlugins.value(id);
			if(getIntV(availDev.version) > getIntV(instDev.version))
			{
				// update plugin, newer version in repo
				installPlugin(id);
			}
		}
	    ++i;
	}
}

void PluginFilesHandler::removePlugin(const QString& id)
{
	if(FileUtils::removeDir(_pluginsDir+"/"+id, _log))
	{
		_installedPlugins.remove(id);
		_PDB->deletePluginRecord(id);
		Info(_log,"Plugin id '%s' deleted successfully",QSTRING_CSTR(id));
		emit pluginAction(P_REMOVED, id, true);
		return;
	}
	Error(_log,"Failed to delete Plugin id '%s'",QSTRING_CSTR(id));
	emit pluginAction(P_REMOVED, id, false);
}

void PluginFilesHandler::installPlugin(const QString& id)
{
	// verify the plugin exists in availablePlugins
	if(!_availablePlugins.contains(id))
	{
		Error(_log, "Plugin id '%s' doesn't exist in repository, therefore you can't install/update it", QSTRING_CSTR(id));
		emit pluginAction(P_INSTALLED, id, false);
		return;
	}

	PluginDefinition availDev = _availablePlugins.value(id);
	PluginDefinition instDev = _installedPlugins.value(id);

	// compare plugin version
	if(getIntV(instDev.version) >= getIntV(availDev.version))
	{
		Error(_log, "Plugin '%s' has no new version to install (installed: %s) (available: %s)", QSTRING_CSTR(availDev.name), QSTRING_CSTR(instDev.version), QSTRING_CSTR(availDev.version));
		emit pluginAction(P_INSTALLED, id, false);
		return;
	}

	// check dependencies
	QJsonObject deps = availDev.dependencies;
	if(!deps.isEmpty())
	{
		for (QJsonObject::iterator it = deps.begin(); it != deps.end(); it++)
		{
			QString dep = it.key();
			const QJsonValue &ref  = *it;
			QString dver = ref.toString();

			// special key hyperion
			if(dep == "hyperion")
			{
				if((getIntV(dver) > getIntV(QString(HYPERION_VERSION))))
				{
					Warning(_log, "Plugin '%s' requires newer version of Hyperion (%s) in order to be installed/updated. Your current version is '%s'", QSTRING_CSTR(availDev.name), QSTRING_CSTR(dver), QSTRING_CSTR(QString(HYPERION_VERSION)));
					emit pluginAction(P_INSTALLED, id, false);
					return;
				}
				continue;
			}
			// all other deps
			// check if dep is installed
			if(_installedPlugins.contains(dep))
			{
				// in target version (edge case error)
				PluginDefinition idep = _installedPlugins.value(dep);
				if(getIntV(idep.version) < getIntV(dver))
				{
					Error(_log, "Plugin '%s' requires plugin '%s' in version '%s', installed is '%s', please update!",QSTRING_CSTR(availDev.name), QSTRING_CSTR(dep), QSTRING_CSTR(dver), QSTRING_CSTR(idep.version));
					emit pluginAction(P_INSTALLED, id, false);
					return;
				}
			}
			// check if dep is available
			else if(_availablePlugins.contains(dep))
			{
				// in target version
				PluginDefinition adep = _availablePlugins.value(dep);
				if(getIntV(adep.version) < getIntV(dver))
				{
					Error(_log, "Plugin '%s' requires plugin '%s' in version '%s', which is NOT FOUND! Next available version is '%s', abort install/update",QSTRING_CSTR(availDev.name), QSTRING_CSTR(dep), QSTRING_CSTR(dver), QSTRING_CSTR(adep.version));
					emit pluginAction(P_INSTALLED, id, false);
					return;
				}
				// install the dep
				installPlugin(dep);
			}
			else
			{
				Error(_log, "Plugin '%s' requires plugin '%s' which is NOT FOUND, abort install/update",QSTRING_CSTR(availDev.name), QSTRING_CSTR(dep));
				emit pluginAction(P_INSTALLED, id, false);
				return;
			}
		}
	}

	// ready for download
	_http->sendGet(_dUrl+id+".zip", "P_INSTALL:"+id);
}

void PluginFilesHandler::updateAvailablePlugins(void)
{
	_http->sendGet(_rUrl, "P_AVAIL");
}

bool PluginFilesHandler::getPluginDefinition(const QString& id, PluginDefinition& def)
{
	if(_installedPlugins.contains(id))
	{
		def = _installedPlugins.value(id);
		return true;
	}
	return false;
}

bool PluginFilesHandler::updateInstalledPlugin(const QString& tid)
{
	// get plugin.json for meta data
	QString pd(_pluginsDir+"/"+tid);
	QJsonObject metaObj;
	if(!JsonUtils::readFile(pd+"/plugin.json", metaObj, _log))
		return false;

	// if the plugin is a service, more data is required
	QString id = metaObj["id"].toString().toLower();
	QString entryPy("/lib");
	QJsonObject settingsSchemaObj;

	// compare provided id(eg. folder name) with meta id
	if(tid != id)
	{
		Error(_log, "The plugin folder name (%s) does not match the provided plugin.json id (%s)", QSTRING_CSTR(tid), QSTRING_CSTR(id));
		return false;
	}

	if(id.startsWith("service."))
	{
		// get the settingsSchema.json
		if(!JsonUtils::readFile(pd+"/settingsSchema.json", settingsSchemaObj, _log))
			return false;

		// check if service.py is available
		if(!FileUtils::fileExists(pd+"/service.py",_log))
			return false;

		// point to service.py
		entryPy = "/service.py";

		// load translations??
	}

	// create the PluginDefinition
	PluginDefinition newDefinition;
	newDefinition.name = metaObj["name"].toString();
	newDefinition.description = metaObj["description"].toString();
	newDefinition.version = metaObj["version"].toString();
	newDefinition.dependencies = metaObj["dependencies"].toObject();
	newDefinition.changelog = metaObj["changelog"].toArray();
	newDefinition.provider = metaObj["provider"].toString();
	newDefinition.support = metaObj["support"].toString();
	newDefinition.source = metaObj["source"].toString();
	newDefinition.entryPy = pd+entryPy;
	newDefinition.settingsSchema = settingsSchemaObj;

	_installedPlugins.insert(id,newDefinition);

	return true;
}

void PluginFilesHandler::readInstalledPlugins(void)
{
	QDir dir(_pluginsDir);
	QStringList filters;
	filters << "service.*" << "module.*";
	dir.setNameFilters(filters);
	QStringList pluginDirs = dir.entryList(QDir::Dirs);

	// iterate through each plugin folder
	for (const auto & pluginDir : pluginDirs)
	{
		if(!updateInstalledPlugin(pluginDir))
			Error(_log, "Failed to parse plugin directory '%s'",QSTRING_CSTR(pluginDir));
	}
}

int PluginFilesHandler::getIntV(QString str)
{
	return str.remove('.').toInt();
}
