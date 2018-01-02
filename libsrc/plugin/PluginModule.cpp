
#include "Plugin.h"
#include <plugin/PluginModule.h>

// hyperion
#include <hyperion/Hyperion.h>
#include <utils/Logger.h>
#include <utils/ColorRgb.h>

// qt
#include <QJsonArray>

struct PyModuleDef PluginModule::moduleDef = {
	PyModuleDef_HEAD_INIT,
	"plugin",            /* m_name */
	"Plugin module",     /* m_doc */
	-1,                    /* m_size */
	PluginModule::pluginMethods, /* m_methods */
	NULL,                  /* m_reload */
	NULL,                  /* m_traverse */
	NULL,                  /* m_clear */
	NULL,                  /* m_free */
};

PyObject* PluginModule::PyInit_plugin()
{
	return PyModule_Create(&moduleDef);
}

void PluginModule::registerPluginExtensionModule()
{
	PyImport_AppendInittab("plugin", &PyInit_plugin);
}

PyObject *PluginModule::json2python(const QJsonValue &jsonData)
{
	switch (jsonData.type())
	{
		case QJsonValue::Null:
			return Py_BuildValue("");
		case QJsonValue::Undefined:
			return Py_BuildValue("");
		case QJsonValue::Double:
		{
			if (std::rint(jsonData.toDouble()) != jsonData.toDouble())
			{
				return Py_BuildValue("d", jsonData.toDouble());
			}
			return Py_BuildValue("i", jsonData.toInt());
		}
		case QJsonValue::Bool:
			return Py_BuildValue("i", jsonData.toBool() ? 1 : 0);
		case QJsonValue::String:
			return Py_BuildValue("s", jsonData.toString().toUtf8().constData());
		case QJsonValue::Object:
		{
			PyObject * dict= PyDict_New();
			QJsonObject objectData = jsonData.toObject();
			for (QJsonObject::iterator i = objectData.begin(); i != objectData.end(); ++i)
			{
				PyObject * obj = json2python(*i);
				PyDict_SetItemString(dict, i.key().toStdString().c_str(), obj);
				Py_XDECREF(obj);
			}
			return dict;
		}
		case QJsonValue::Array:
		{
			QJsonArray arrayData = jsonData.toArray();
			PyObject * list = PyList_New(arrayData.size());
			int index = 0;
			for (QJsonArray::iterator i = arrayData.begin(); i != arrayData.end(); ++i, ++index)
			{
				PyObject * obj = json2python(*i);
				Py_INCREF(obj);
				PyList_SetItem(list, index, obj);
				Py_XDECREF(obj);
			}
			return list;
		}
	}

	assert(false);
	return nullptr;
}

// Python method table
PyMethodDef PluginModule::pluginMethods[] = {
	{"log"                   , PluginModule::wrapLog                   , METH_VARARGS, "Write a message to the log"},
	{"abort"                 , PluginModule::wrapAbort                 , METH_NOARGS,  "Check if the plugin should abort execution."},
	{"getSettings"           , PluginModule::wrapGetSettings           , METH_NOARGS,  "Get the settings object"},
	{"setComponentState"     , PluginModule::wrapSetComponentState     , METH_VARARGS, "Set a component to a state, returns false if comp is not found."},
	{"setColor"              , PluginModule::wrapSetColor              , METH_VARARGS, "Set a single color"},
	{"setEffect"             , PluginModule::wrapSetEffect             , METH_VARARGS, "Set a effect by name. Timeout and priority are optional"},

	// callback methods
	{"onSettingsChanged"     , PluginModule::wrapOnSettingsChanged     , METH_VARARGS, "Callback whenever settings changed"},
	{NULL, NULL, 0, NULL}
};

Plugin * PluginModule::getPlugin()
{
	// extract the module from the runtime
	PyObject * module = PyObject_GetAttrString(PyImport_AddModule("__main__"), "plugin");

	if (!PyModule_Check(module))
	{
		// something is wrong
		Py_XDECREF(module);
		Error(Logger::getInstance("PLUGIN"), "Unable to retrieve the plugin object from the Python runtime");
		return nullptr;
	}

	// retrieve the capsule with the effect
	PyObject * pluginCapsule = PyObject_GetAttrString(module, "__pluginObj");
	Py_XDECREF(module);

	if (!PyCapsule_CheckExact(pluginCapsule))
	{
		// something is wrong
		Py_XDECREF(pluginCapsule);
		Error(Logger::getInstance("PLUGIN"), "Unable to retrieve the plugin object from the Python runtime");
		return nullptr;
	}

	// Get the effect from the capsule
	Plugin * plugin = reinterpret_cast<Plugin *>(PyCapsule_GetPointer(pluginCapsule, nullptr));
	Py_XDECREF(pluginCapsule);
	return plugin;
}

PyObject* PluginModule::wrapAbort(PyObject *self, PyObject *args)
{
	Plugin * plugin = getPlugin();

	return Py_BuildValue("i", plugin->isInterruptionRequested() ? 1 : 0);
}

PyObject* PluginModule::wrapLog(PyObject *self, PyObject *args)
{
	Plugin * plugin = getPlugin();

	char *msg;
	int lvl;

	int argCount = PyTuple_Size(args);
	if (argCount == 2)
	{
		// log with lvl
		if (!PyArg_ParseTuple(args, "si", &msg, &lvl))
		{
			PyErr_SetString(PyExc_TypeError, "String as message and lvl as integer required");
			return nullptr;
		}
		plugin->printToLog(msg, lvl);
		return Py_BuildValue("");
	}
	else if (argCount == 1)
	{
		if(!PyArg_ParseTuple(args, "s", &msg))
		{
			PyErr_SetString(PyExc_TypeError, "String required to print a log message");
			return nullptr;
		}
		plugin->printToLog(msg);
		return Py_BuildValue("");
	}
	return nullptr;
}

PyObject* PluginModule::wrapGetSettings(PyObject *self, PyObject *args)
{
	return Py_BuildValue("O",PluginModule::json2python(getPlugin()->getSettings()));
}

PyObject* PluginModule::wrapSetComponentState(PyObject *self, PyObject *args)
{
	int comp, enable;

	// log with lvl
	if (!PyArg_ParseTuple(args, "ii", &comp, &enable))
	{
		PyErr_SetString(PyExc_TypeError, "To set a component state, you need two int args");
		return nullptr;
	}
	return Py_BuildValue("i",getPlugin()->setComponentState(comp, enable));
}

PyObject* PluginModule::wrapSetColor(PyObject *self, PyObject *args)
{
	Plugin * plugin = getPlugin();

	ColorRgb color;
	int duration = -1;
	int priority = 50;

	int argCount = PyTuple_Size(args);
	if(argCount == 3 && PyArg_ParseTuple(args, "bbb", &color.red, &color.green, &color.blue))
	{
		plugin->setColor(color, priority, duration);
		return Py_BuildValue("");
	}
	else if(argCount == 4 && PyArg_ParseTuple(args, "bbbi", &color.red, &color.green, &color.blue, &duration))
	{
		plugin->setColor(color, priority, duration);
		return Py_BuildValue("");
	}
	else if(argCount == 5 && PyArg_ParseTuple(args, "bbbii", &color.red, &color.green, &color.blue, &duration, &priority))
	{
		plugin->setColor(color, priority, duration);
		return Py_BuildValue("");
	}
	PyErr_SetString(PyExc_RuntimeError, "Invalid arguments for setColor()");
	return nullptr;
}

PyObject* PluginModule::wrapSetEffect(PyObject *self, PyObject *args)
{
	Plugin * plugin = getPlugin();

	char* name;
	int duration = -1;
	int priority = 50;

	int argCount = PyTuple_Size(args);
	if(argCount == 1 && PyArg_ParseTuple(args, "s", &name))
	{
		return Py_BuildValue("i", plugin->setEffect(name, priority, duration));
	}
	else if(argCount == 2 && PyArg_ParseTuple(args, "si", &name, &duration))
	{
		return Py_BuildValue("i", plugin->setEffect(name, priority, duration));
	}
	else if(argCount == 3 && PyArg_ParseTuple(args, "sii", &name, &duration, &priority))
	{
		return Py_BuildValue("i", plugin->setEffect(name, priority, duration));
	}
	PyErr_SetString(PyExc_RuntimeError, "Invalid arguments for setEffect()");
	return nullptr;
}

PyObject* PluginModule::wrapOnSettingsChanged(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:onSettingsChanged", &temp))
	{
        if (!PyCallable_Check(temp))
		{
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        //Py_XDECREF(my_callback);  /* Dispose of previous callback */
        getPlugin()->callbackObjects.insert("onSettingsChanged", temp);  /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}
