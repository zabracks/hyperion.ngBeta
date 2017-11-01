// project
#include <plugin/Files.h>
#include "PDBWrapper.h"

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

Files::Files(const QString& rootPath, const QString& id, PDBWrapper* PDB)
	: QObject()
	, _log(Logger::getInstance("PLUGINS"))
	, _id(id)
	, _PDB(PDB)
	, _http(new HTTPUtils(_log))
{
	// create folder structure
	_pluginsDir = rootPath + "/plugins";
	_packageDir = _pluginsDir + "/packages";
	_configDir = _pluginsDir + "/config_"+id;

	QDir dir;
	dir.mkpath(_packageDir);
	dir.mkpath(_configDir);

	// listen for http utils reply
	connect(_http, &HTTPUtils::replyReceived, this, &Files::replyReceived);
}

Files::~Files()
{
	delete _http;
	delete _rTimer;
}

void Files::init(void)
{
	// fetch plugins from plugin directory
	readInstalledPlugins();

	// update avail plugins every 24h
	_rTimer = new QTimer(this);
	_rTimer->setInterval(86400000);
	_rTimer->start();
	connect(_rTimer, &QTimer::timeout, this, &Files::updateAvailablePlugins);
	updateAvailablePlugins();
}

void Files::replyReceived(bool success, int type, QString id, QByteArray data)
{
	if(!success)
	{
		if(id == "P_AVAIL")
		{
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
				PluginDefinition newDefinition{
					metaObj["name"].toString(),
					metaObj["description"].toString(),
					metaObj["version"].toString(),
					metaObj["dependencies"].toObject(),
					metaObj["changelog"].toArray(),
					metaObj["provider"].toString(),
					metaObj["support"].toString(),
					metaObj["source"].toString()
				};
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
			if(!updateInstalledPlugin(pid, true))
			{
				Error(_log,"Plugin '%s' install/update failed.", QSTRING_CSTR(dev.name));
				emit pluginAction(P_INSTALLED, pid, false);
				return;
			}

			// update db entry
			_PDB->setPluginUpdatedAt(pid);

			// notify with new definition
			PluginDefinition newDef = _installedPlugins.value(pid);
			Info(_log,"Plugin '%s' successfully installed/updated.", QSTRING_CSTR(dev.name));
			emit pluginAction(P_INSTALLED, pid, true, newDef);

			// start after install
			if(pid.startsWith("service.") && _PDB->isPluginEnabled(pid))
				emit pluginAction(P_START, pid);
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

void Files::doUpdateCheck(void)
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

void Files::doPluginAction(PluginAction action, QString id, bool success, PluginDefinition def)
{
	QJsonObject setting;
	PluginDefinition newDef;
	QJsonObject schema;
	switch(action)
	{
		case P_INSTALL:
			installPlugin(id);
			break;
		default:
			break;
	}
}

void Files::removePlugin(const QString& id)
{
	if(FileUtils::removeDir(_pluginsDir+"/"+id, _log))
	{
		FileUtils::removeFile(_configDir+"/"+id+".json", _log, true);

		_installedPlugins.remove(id);
		_PDB->deletePluginRecord(id);
		Info(_log,"Plugin id '%s' deleted successfully",QSTRING_CSTR(id));
		emit pluginAction(P_REMOVED, id, true);
		return;
	}
	Error(_log,"Failed to delete Plugin id '%s'",QSTRING_CSTR(id));
	emit pluginAction(P_REMOVED, id, false);
}

void Files::saveSettings(const QString& id, const QJsonObject& settings)
{
	if(!_installedPlugins.contains(id) || id.startsWith("module."))
	{
		Error(_log,"Can't save settings for id '%s', not qualified or found.",QSTRING_CSTR(id));
		emit pluginAction(P_SAVED, id, false);
		return;
	}
	PluginDefinition newDef = _installedPlugins.value(id);
	// validate against schema
	QJsonObject schema = newDef.settingsSchema;
	if(!JsonUtils::validate("PluginSave:"+id, settings, schema, _log))
	{
		Error(_log,"Failed to validate settings for id '%s'",QSTRING_CSTR(id));
		emit pluginAction(P_SAVED, id, false);
		return;
	}
	if(JsonUtils::write(_configDir+"/"+id+".json", settings, _log))
	{
		// update the definition
		newDef.settings = settings;
		_installedPlugins.insert(id, newDef);
		emit pluginAction(P_SAVED, id, true, newDef);
		return;
	}
	Error(_log,"Failed to save settings for id '%s'",QSTRING_CSTR(id));
	emit pluginAction(P_SAVED, id, false);
	return;
}

void Files::installPlugin(const QString& id)
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

void Files::updateAvailablePlugins(void)
{
	_http->sendGet(_rUrl, "P_AVAIL");
}

bool Files::getPluginDefinition(const QString& id, PluginDefinition& def)
{
	if(_installedPlugins.contains(id))
	{
		def = _installedPlugins.value(id);
		return true;
	}
	return false;
}

bool Files::updateInstalledPlugin(const QString& tid, const bool& skipStart)
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
	QJsonObject settingsObj;
	if(id.startsWith("service."))
	{
		// get the settingsSchema.json
		if(!JsonUtils::readFile(pd+"/settingsSchema.json", settingsSchemaObj, _log))
			return false;

		// check if service.py is available
		if(!FileUtils::fileExists(pd+"/service.py",_log))
			return false;

		// get settings if available
		JsonUtils::readFile(_configDir+"/"+id+".json", settingsObj, _log, true);

		// point to service.py
		entryPy = "/service.py";

		// load translations??
	}

	// create the PluginDefinition
	PluginDefinition newDefinition{
		metaObj["name"].toString(),
		metaObj["description"].toString(),
		metaObj["version"].toString(),
		metaObj["dependencies"].toObject(),
		metaObj["changelog"].toArray(),
		metaObj["provider"].toString(),
		metaObj["support"].toString(),
		metaObj["source"].toString(),
		pd+entryPy,
		settingsSchemaObj,
		settingsObj
	};
	_installedPlugins.insert(id,newDefinition);

	// create database entry if required
	_PDB->createPluginRecord(id);
	// start/restart(for updated plugins) service if enabled in db
	if(!skipStart && id.startsWith("service.") && _PDB->isPluginEnabled(id))
	{
		emit pluginAction(P_START, id);
	}
	return true;
}

void Files::readInstalledPlugins(void)
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

int Files::getIntV(QString str)
{
	return str.remove('.').toInt();
}
