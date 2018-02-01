#include "CallbackThread.h"

CallbackThread::CallbackThread(QObject* parent)
	: QObject(parent)
{

}

void CallbackThread::handlePluginAction(PluginAction action, QString id, bool success, PluginDefinition def)
{
	emit onPluginAction(action, id, success, def);
}

void CallbackThread::handleCompStateChanged(const hyperion::Components comp, bool state)
{
	emit onCompStateChanged(comp, state);
}

void CallbackThread::handleVisiblePriorityChanged(const quint8& priority)
{
	emit onVisiblePriorityChanged(priority);
}
