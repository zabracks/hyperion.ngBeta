#pragma once

// qt
#include <QObject>

// utils
#include <plugin/PluginDefinition.h>
#include <utils/Components.h>

///
/// A Helper (threaded) class to accept QueuedConnection callbacks from Hyperion (slot invoke in target thread)
/// and forward them to Plugin instances as DirectConnection (slot invoke in caller thread) as a Plugin has no event loop and a blocking call (Py_Run())
/// Each Plugin instance get's a own CallbackThread to grant maximum event deliver speed per plugin
/// (Due to the nature of blocking calls, the slowest plugin would prevent further callbacks for all plugins)
///
class CallbackThread : public QObject
{
	Q_OBJECT
public:
	CallbackThread(QObject* parent = nullptr);

public slots:
	///
	/// CALLBACK SLOTS
	///

	///
	/// @brief called whenever a plugin action is ongoing
	/// @param action   action from enum
	/// @param id       plugin id
	/// @param def      PluginDefinition (optional)
	/// @param success  true if action was a success, else false
	///
	void handlePluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

	///
	/// @brief called when a component state is changed
	/// @param comp   the component
	/// @param state	the changed state
	///
	void handleCompStateChanged(const hyperion::Components comp, bool state);

	///
	/// @brief called when the visible priorty has changed
	/// @param priority   the prioriry
	///
	void handleVisiblePriorityChanged(const quint8& priority);

signals:
	///
	/// @brief called whenever a plugin action is ongoing
	/// @param action   action from enum
	/// @param id       plugin id
	/// @param def      PluginDefinition (optional)
	/// @param success  true if action was a success, else false
	///
	void onPluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

	///
	/// @brief called when a component state is changed
	/// @param comp   the component
	/// @param state	the changed state
	///
	void onCompStateChanged(const hyperion::Components comp, bool state);

	///
	/// @brief called when the visible priorty has changed
	/// @param priority   the prioriry
	///
	void onVisiblePriorityChanged(const quint8& priority);
};
