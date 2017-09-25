// project include
#include <plugin/Plugin.h>
#include <Python.h>
// qt includes
#include <QDebug>

Plugin::Plugin()
	: QObject()
	, _log(Logger::getInstance("PLUGINS"))
	, _files(_log)
{

}

Plugin::~Plugin()
{

}

void Plugin::updatePluginList(void)
{

}
