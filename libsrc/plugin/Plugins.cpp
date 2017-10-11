// project
#include <plugin/Plugins.h>

// qt includes
#include <QTimer>
#include <QDebug>

Plugins::Plugins()
	: QObject()
	, _log(Logger::getInstance("PLUGINS"))
	, _hyperion(Hyperion::getInstance())
	, _files(_log, _hyperion->getRootPath(), _hyperion->getId())
{

	// listen for pluginActions
	connect(this, &Plugins::pluginAction, this, &Plugins::doPluginAction);

	// pipe also to files
	connect(this, &Plugins::pluginAction, &_files, &Files::pluginAction);

	QTimer::singleShot(4000, this, SLOT(start()));
}

Plugins::~Plugins()
{
	foreach (Plugin* plug, _runningPlugins)
	    plug->requestAbort(true);
}

void Plugins::pluginFinished()
{
	Plugin* plugin = qobject_cast<Plugin*>(sender());
	QString id = plugin->getId();

	// delete pointer and remove from list
	_runningPlugins.remove(id);
	plugin->deleteLater();

	// trigger restart
	if(_restartQueue.contains(id))
	{
		_restartQueue.removeAll(id);
		start(id);
	}
}

void Plugins::doPluginAction(PluginAction action, QString id, bool success, PluginDefinition def)
{
	switch(action)
	{
		case P_START:
			start(id);
			break;
		case P_STOP:
			stop(id);
			break;
		case P_REMOVE:
			stop(id);
			break;
		default:
			break;
	}
}

void Plugins::start(QString id)
{
	if(!id.startsWith("service."))
	{
		Error(_log, "Can't start plugin id '%s', it's not meant to be started",QSTRING_CSTR(id));
		emit pluginAction(P_STARTED, id, false);
	}

	// get the definition
	PluginDefinition def;
	if(!_files.getPluginDefinition(id, def))
	{
		Error(_log, "Can't start plugin id '%s', it's not installed",QSTRING_CSTR(id));
		emit pluginAction(P_STARTED, id, false);
	}

	// if plugin id is currently running stop it and add to _restartQueue
	if(_runningPlugins.contains(id))
	{
		Plugin* plugin = _runningPlugins.value(id);

		// protect queue of repeated entrys & forced Timer
		if(plugin->isAborting())
			return;

		_restartQueue << id;
		plugin->requestAbort();
		return;
	}

	// extract dependencies paths
	QStringList dPaths;
	QJsonObject deps = def.dependencies;
	deps.remove("hyperion");
	for (QJsonObject::iterator it = deps.begin(); it != deps.end(); it++)
	{
		QString dep = it.key();
		PluginDefinition tempDef;
		if(!_files.getPluginDefinition(dep, tempDef))
		{
			Error(_log,"Failed to get path of dependency %s", QSTRING_CSTR(dep));
			continue;
		}
		dPaths << tempDef.entryPy;
	}

	// create plugin instance
	Plugin* newPlugin = new Plugin(def, id, dPaths);
	_runningPlugins.insert(id, newPlugin);
	// get info when a plugin thread exits
	connect(newPlugin, &QThread::finished, this, &Plugins::pluginFinished);
}

void Plugins::stop(QString id)
{
	if(_runningPlugins.contains(id))
	{
		Plugin* plugin = _runningPlugins.value(id);

		if(plugin->isAborting())
			return;

		plugin->requestAbort();
	}
}
