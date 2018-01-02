#pragma once

#undef slots
#include <Python.h>
#define slots

#include <QJsonValue>

class Plugin;

class PluginModule
{
public:
	// Python 3 module def
	static struct PyModuleDef moduleDef;

	// Init module
	static PyObject* PyInit_plugin();

	// Register module once
	static void registerPluginExtensionModule();

	// json 2 python
	static PyObject * json2python(const QJsonValue & jsonData);

	// Wrapper methods for Python interpreter extra buildin methods
	static PyMethodDef pluginMethods[];
	static PyObject* wrapAbort                 (PyObject *self, PyObject *args);
	static PyObject* wrapLog                   (PyObject *self, PyObject *args);
	static PyObject* wrapGetSettings           (PyObject *self, PyObject *args);
	static PyObject* wrapSetComponentState     (PyObject *self, PyObject *args);
	static PyObject* wrapSetColor              (PyObject *self, PyObject *args);
	static PyObject* wrapSetEffect             (PyObject *self, PyObject *args);
	static Plugin * getPlugin();

	// Callback methods
	static PyObject* wrapOnSettingsChanged         (PyObject *self, PyObject *args);
};
