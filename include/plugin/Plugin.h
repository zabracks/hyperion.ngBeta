#pragma once

//hyperion incl
#include <utils/Logger.h>
#include <plugin/PluginDefinition.h>

// qt incl
#include <QThread>

class Plugin : public QThread
{
	Q_OBJECT

public:

	Plugin(const PluginDefinition& def, const QString& id, const QStringList& dPaths);
	virtual ~Plugin();

	// QThread inherited run method
	virtual void run();

	/// get plugin id
	QString getId(){ return _id; };
	/// request a plugin abort, forced on destruction
	void requestAbort(bool forced = false){ _abortRequested = true; };
	/// check if abort is in progress
	bool isAborting(){ return _abortRequested; };

private:
	/// definition of this instance
	PluginDefinition _def;
	/// id of the plugin
	const QString _id;
	/// dependencies Paths
	const QStringList _dPaths;
	/// Logger instance
	Logger *_log;
	/// abort requested
	bool _abortRequested = false;
	/// store py path
	std::string _pythonPath;

	// add a python path to _pythonPath
	void addNativePath(const std::string& path);

	// build _pythonPath and apply with dependencies
	void handlePyPath(void);

	// prints exception to log
	void printException(void);
};
