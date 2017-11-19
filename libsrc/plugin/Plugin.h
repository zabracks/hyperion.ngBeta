#pragma once

// Python
#undef slots
#include <Python.h>
#define slots

//hyperion incl
#include <utils/Logger.h>
#include <plugin/PluginDefinition.h>

// qt incl
#include <QThread>
#include <QStringList>

class Plugin : public QThread
{
	Q_OBJECT

public:

	Plugin(PyThreadState* mainState, const PluginDefinition& def, const QString& id, const QStringList& dPaths);
	virtual ~Plugin();

	// QThread inherited run method
	virtual void run();

	/// get plugin id
	const QString getId(){ return _id; };
	/// is true when a error occurred
	bool hasError() const { return _error; };
	/// set the remove flag
	void setRemoveFlag(){ _remove = true; };
	/// get the remove flag
	bool hasRemoveFlag() const { return _remove; };

	static void registerPluginModule();

private:
	PyThreadState* _mainState;
	/// definition of this instance
	PluginDefinition _def;
	/// id of the plugin
	const QString _id;
	/// dependencies Paths
	const QStringList _dPaths;
	/// Logger instance
	Logger *_log;
	/// store py path
	std::string _pythonPath;
	/// true if error occurred
	bool _error = false;
	/// true if plugin should be removed after stop
	bool _remove = false;

	// add a python path to _pythonPath
	void addNativePath(const std::string& path);

	// build _pythonPath and apply with dependencies
	void handlePyPath(void);

	// prints exception to log
	void printException(void);

	FILE* PyFile_AsFileWithMode(PyObject *py_file, const char *mode);

	// Wrapper methods for Python interpreter extra buildin methods
	static PyMethodDef pluginMethods[];
	static PyObject* log              (PyObject *self, PyObject *args);

	static struct PyModuleDef moduleDef;

	Plugin* getInstance();
};
