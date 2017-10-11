// Python
#include <Python.h>
#undef B0

// proj include
#include <plugin/Plugin.h>

// qt include
#include <QDebug>

// py path seperator
#ifdef TARGET_WINDOWS
	#define PY_PATH_SEP ";";
#else // not windows
	#define PY_PATH_SEP ":";
#endif

Plugin::Plugin(const PluginDefinition& def, const QString& id,  const QStringList& dPaths)
	: QThread()
	, _def(def)
	, _id(id)
	, _dPaths(dPaths)
	, _log(Logger::getInstance("PLUGIN"))
{
	// start the thread
	start();
}

Plugin::~Plugin()
{

}

void Plugin::run()
{
	qDebug()<<"PLUGIN RUNNER"<<_id;
	// gil lock
	PyEval_AcquireLock();
	// Initialize a new thread state
	PyThreadState* state = Py_NewInterpreter();
	// verify we got a thread state
	if (state == nullptr)
	{
		PyEval_ReleaseLock();
		Error(_log, "No thread state for id %s!",QSTRING_CSTR(_id));
		return;
	}
	// swap interpreter thread
	PyThreadState_Swap(state);

	// create and apply Python path with dependencies
	handlePyPath();

	PyObject *file = PyFile_FromString((char *)QSTRING_CSTR(_def.entryPy), (char*)"r");
	FILE *fp = PyFile_AsFile(file);

	if (fp == nullptr)
	{
		Error(_log, "ID %s: Failed to open script '%s'", QSTRING_CSTR(_id), QSTRING_CSTR(_def.entryPy));
	}
	else
	{
		PyObject *f = PyString_FromString(QSTRING_CSTR(_def.entryPy)); // New ref
		//PyDict_SetItemString(moduleDict, "__file__", f);
		//onPythonModuleInitialization(moduleDict);
		Py_DECREF(f);

		PyObject *main_module = PyImport_ImportModule("__main__"); // New Reference
		PyObject *main_dict = PyModule_GetDict(main_module); // Borrowed reference
		Py_INCREF(main_dict); // Incref "main_dict" to use it in PyRun_File(), because PyModule_GetDict() has decref "main_dict"
		Py_DECREF(main_module); // // release "main_module"

		// run
		PyObject* result = PyRun_File(fp, QSTRING_CSTR(_def.entryPy), Py_file_input, main_dict, main_dict);  // New Reference, may be NULL

		// handle exception
		if (!result && PyErr_Occurred()) // borrowed
		{
			printException();
		}
		Py_XDECREF(result);  // release "result"
		Py_DECREF(main_dict);  // release "main_dict"
	}

	// make sure all sub threads have finished
	for (PyThreadState* s = state->interp->tstate_head, *old = nullptr; s;)
	{
		if (s == state)
		{
			s = s->next;
			continue;
		}
		if (old != s)
		{
  			Debug(_log,"ID %s: Waiting on thread %u", QSTRING_CSTR(_id), s->thread_id);
			old = s;
		}

		Py_BEGIN_ALLOW_THREADS;
		msleep(100);
		Py_END_ALLOW_THREADS;

		s = state->interp->tstate_head;
	}

	Py_EndInterpreter(state);
	PyEval_ReleaseLock();
	qDebug()<<"PLUGIN STOP"<<_id;
}

void Plugin::handlePyPath(void)
{
	// get current paths, prefer sys.path over Py_GetPath
	PyObject *sysMod(PyImport_ImportModule((char*)"sys")); // new ref
	PyObject *sysModDict(PyModule_GetDict(sysMod)); // borrowed ref
	PyObject *pathObj(PyDict_GetItemString(sysModDict, "path")); // borrowed ref

	if (pathObj != NULL && PyList_Check(pathObj))
	{
		for (int i = 0; i < PyList_Size(pathObj); i++)
		{
			PyObject *e = PyList_GetItem(pathObj, i); // borrowed ref
			if (e != NULL && PyString_Check(e))
				addNativePath(PyString_AsString(e));
		}
	}
	else
		addNativePath(Py_GetPath());

	Py_DECREF(sysMod);

	// add dependencies to _pythonPath
	for(const auto entry : _dPaths)
	{
		addNativePath(entry.toStdString());
	}

	// apply path
	Debug(_log,"ID %s: Python path: %s",QSTRING_CSTR(_id),_pythonPath.c_str())
	PySys_SetPath((char *)_pythonPath.c_str());
}

void Plugin::addNativePath(const std::string& path)
{
  if (path.empty())
    return;

  if (!_pythonPath.empty())
    _pythonPath += PY_PATH_SEP;

  _pythonPath += path;
}

void Plugin::printException(void)
{
	Error(_log,"###### PYTHON EXCEPTION ######");
	Error(_log,"## In plugin '%s' id '%s'", QSTRING_CSTR(_def.name), QSTRING_CSTR(_id));
	/* Objects all initialized to NULL for Py_XDECREF */
	PyObject *errorType = nullptr, *errorValue = nullptr, *errorTraceback = nullptr;

	PyErr_Fetch(&errorType, &errorValue, &errorTraceback); // New Reference or NULL
	PyErr_NormalizeException(&errorType, &errorValue, &errorTraceback);

	// Extract exception message from "errorValue"
	if(errorValue)
	{
		QString message;
		if(PyObject_HasAttrString(errorValue, "__class__"))
		{
			PyObject *classPtr = PyObject_GetAttrString(errorValue, "__class__"); // New Reference
			PyObject *class_name = nullptr; // Object "class_name" initialized to NULL for Py_XDECREF
			class_name = PyObject_GetAttrString(classPtr, "__name__"); // New Reference or NULL

			if(class_name && PyString_Check(class_name))
				message.append(PyString_AsString(class_name));

			Py_DECREF(classPtr); // release "classPtr" when done
			Py_XDECREF(class_name); // Use Py_XDECREF() to ignore NULL references
		}

		PyObject *valueString = nullptr;
		valueString = PyObject_Str(errorValue); // New Reference or NULL

		if(valueString && PyString_Check(valueString))
		{
			if(!message.isEmpty())
				message.append(": ");

			message.append(PyString_AsString(valueString));
		}
		Py_XDECREF(valueString); // Use Py_XDECREF() to ignore NULL references

		Error(_log, "## %s", QSTRING_CSTR(message));
	}

	// Extract exception message from "errorTraceback"
	if(errorTraceback)
	{
		// Object "tracebackList" initialized to NULL for Py_XDECREF
		PyObject *tracebackModule = nullptr, *methodName = nullptr, *tracebackList = nullptr;

		tracebackModule = PyImport_ImportModule("traceback"); // New Reference or NULL
		methodName = PyString_FromString("format_exception"); // New Reference or NULL
		tracebackList = PyObject_CallMethodObjArgs(tracebackModule, methodName, errorType, errorValue, errorTraceback, NULL); // New Reference or NULL

		if(tracebackList)
		{
			PyObject* iterator = PyObject_GetIter(tracebackList); // New Reference

			PyObject* item;
			while( (item = PyIter_Next(iterator)) ) // New Reference
			{
				Error(_log, "## %s", QSTRING_CSTR(QString(PyString_AsString(item)).trimmed()));
				Py_DECREF(item); // release "item" when done
			}
			Py_DECREF(iterator);  // release "iterator" when done
		}

		// Use Py_XDECREF() to ignore NULL references
		Py_XDECREF(tracebackModule);
		Py_XDECREF(methodName);
		Py_XDECREF(tracebackList);

		// Give the exception back to python and print it to stderr in case anyone else wants it.
		Py_XINCREF(errorType);
		Py_XINCREF(errorValue);
		Py_XINCREF(errorTraceback);

		PyErr_Restore(errorType, errorValue, errorTraceback);
		// PyErr_PrintEx(0); // Remove this line to switch off stderr output
	}
	Error(_log,"###### EXCEPTION END ######");
}
