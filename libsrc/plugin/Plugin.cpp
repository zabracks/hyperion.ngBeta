// project include
#include <plugin/Plugin.h>
#include <Python.h>
// qt includes
#include <QDebug>

Plugin::Plugin()
	: QObject()
	, _log(Logger::getInstance("PLUGINS"))
	, _hyperion(Hyperion::getInstance())
	, _files(_log, _hyperion->getRootPath())
{

}

Plugin::~Plugin()
{

}

void Plugin::updatePluginList(void)
{

}
