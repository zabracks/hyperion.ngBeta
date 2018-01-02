// proj
#include <hyperion/SettingsManager.h>

// util
#include <utils/JsonUtils.h>
#include <db/SettingsTable.h>
// json schema process
#include <utils/jsonschema/QJsonFactory.h>
#include <utils/jsonschema/QJsonSchemaChecker.h>

// write config to filesystem
#include <utils/JsonUtils.h>

// hyperion
#include <hyperion/Hyperion.h>

QJsonObject SettingsManager::schemaJson;

SettingsManager::SettingsManager(Hyperion* hyperion, const quint8& instance, const QString& configFile)
	: _hyperion(hyperion)
	, _log(Logger::getInstance("SettingsManager"))
	, _sTable(new SettingsTable(instance))
{
	Q_INIT_RESOURCE(resource);
	connect(this, &SettingsManager::settingsChanged, _hyperion, &Hyperion::settingsChanged);
	// get schema
	if(schemaJson.isEmpty())
	{
		try
		{
			schemaJson = QJsonFactory::readSchema(":/hyperion-schema");
		}
		catch(const std::runtime_error& error)
		{
			throw std::runtime_error(error.what());
		}
	}
	// get default config
	QJsonObject defaultConfig;
	if(!JsonUtils::readFile(":/hyperion_default.config", defaultConfig, _log))
		throw std::runtime_error("Failed to read default config");

	// get USER config file
	// //////////////////////
	// DEPRECATION START - remove when database migration is done
	Info(_log, "Selected configuration file: %s", QSTRING_CSTR(configFile));
	QJsonSchemaChecker schemaCheckerT;

	if(!JsonUtils::readFile(configFile, _qconfig, _log))
		throw std::runtime_error("Failed to load config!");

	// validate config with schema and correct it if required
	QPair<bool, bool> validate = schemaCheckerT.validate(_qconfig);

	// errors in schema syntax, abort
	if (!validate.second)
	{
		foreach (auto & schemaError, schemaCheckerT.getMessages())
			Error(_log, "Schema Syntax Error: %s", QSTRING_CSTR(schemaError));

		throw std::runtime_error("ERROR: Hyperion schema has syntax errors!");
	}
	// errors in configuration, correct it!
	if (!validate.first)
	{
		Warning(_log,"Errors have been found in the configuration file. Automatic correction has been applied");
		_qconfig = schemaCheckerT.getAutoCorrectedConfig(_qconfig);

		foreach (auto & schemaError, schemaCheckerT.getMessages())
			Warning(_log, "Config Fix: %s", QSTRING_CSTR(schemaError));

		if (!JsonUtils::write(configFile, _qconfig, _log))
			throw std::runtime_error("ERROR: Can't save configuration file, aborting");
	}
	// DEPRECATION END - remove when database migration is done
	// ////////////////////////

	// transform json to string lists
	QStringList keyList = defaultConfig.keys();
	QStringList defValueList;
	for(const auto key : keyList)
	{
		if(defaultConfig[key].isObject())
		{
			defValueList << QString(QJsonDocument(defaultConfig[key].toObject()).toJson(QJsonDocument::Compact));
		}
		else if(defaultConfig[key].isArray())
		{
			defValueList << QString(QJsonDocument(defaultConfig[key].toArray()).toJson(QJsonDocument::Compact));
		}
	}

	// fill database with default data if required
	for(const auto key : keyList)
	{
		QString val = defValueList.takeFirst();
		// prevent overwrite
		if(!_sTable->recordExist(key))
			_sTable->createSettingsRecord(key,val);
	}

	// need to validate all data in database constuct the entire data object
	// TODO refactor schemaChecker to accept QJsonArray in validate(); QJsonDocument container? To validate them per entry...
	QJsonObject dbConfig;
	for(const auto key : keyList)
	{
		QJsonDocument doc = _sTable->getSettingsRecord(key);
		if(doc.isArray())
			dbConfig[key] = doc.array();
		else
			dbConfig[key] = doc.object();
	}

	// validate full dbconfig against schema, on error we need to rewrite entire table
	QJsonSchemaChecker schemaChecker;
	schemaChecker.setSchema(schemaJson);
	QPair<bool,bool> valid = schemaChecker.validate(dbConfig);
	// check if our main schema syntax is IO
	if (!valid.second)
	{
		foreach (auto & schemaError, schemaChecker.getMessages())
			Error(_log, "Schema Syntax Error: %s", QSTRING_CSTR(schemaError));
		throw std::runtime_error("The config schema has invalid syntax. This should never happen! Go fix it!");
	}
	if (!valid.first)
	{
		Info(_log,"Table upgrade required...");
		dbConfig = schemaChecker.getAutoCorrectedConfig(dbConfig);

		foreach (auto & schemaError, schemaChecker.getMessages())
			Warning(_log, "Config Fix: %s", QSTRING_CSTR(schemaError));

		saveSettings(dbConfig);
	}

	Debug(_log,"Settings database initialized")
}

SettingsManager::SettingsManager(const quint8& instance, const QString& configFile)
	: _hyperion(nullptr)
	, _log(Logger::getInstance("SettingsManager"))
	, _sTable(new SettingsTable(instance))
{
	Q_INIT_RESOURCE(resource);
	// get schema
	if(schemaJson.isEmpty())
	{
		try
		{
			schemaJson = QJsonFactory::readSchema(":/hyperion-schema");
		}
		catch(const std::runtime_error& error)
		{
			throw std::runtime_error(error.what());
		}
	}
	// get default config
	QJsonObject defaultConfig;
	if(!JsonUtils::readFile(":/hyperion_default.config", defaultConfig, _log))
		throw std::runtime_error("Failed to read default config");

	// get USER config file
	// //////////////////////
	// DEPRECATION START - remove when database migration is done
	Info(_log, "Selected configuration file: %s", QSTRING_CSTR(configFile));
	QJsonSchemaChecker schemaCheckerT;

	if(!JsonUtils::readFile(configFile, _qconfig, _log))
		throw std::runtime_error("Failed to load config!");

	// validate config with schema and correct it if required
	QPair<bool, bool> validate = schemaCheckerT.validate(_qconfig);

	// errors in schema syntax, abort
	if (!validate.second)
	{
		foreach (auto & schemaError, schemaCheckerT.getMessages())
			Error(_log, "Schema Syntax Error: %s", QSTRING_CSTR(schemaError));

		throw std::runtime_error("ERROR: Hyperion schema has syntax errors!");
	}
	// errors in configuration, correct it!
	if (!validate.first)
	{
		Warning(_log,"Errors have been found in the configuration file. Automatic correction has been applied");
		_qconfig = schemaCheckerT.getAutoCorrectedConfig(_qconfig);

		foreach (auto & schemaError, schemaCheckerT.getMessages())
			Warning(_log, "Config Fix: %s", QSTRING_CSTR(schemaError));

		if (!JsonUtils::write(configFile, _qconfig, _log))
			throw std::runtime_error("ERROR: Can't save configuration file, aborting");
	}
	// DEPRECATION END - remove when database migration is done
	// ////////////////////////

	// transform json to string lists
	QStringList keyList = defaultConfig.keys();
	QStringList defValueList;
	for(const auto key : keyList)
	{
		if(defaultConfig[key].isObject())
		{
			defValueList << QString(QJsonDocument(defaultConfig[key].toObject()).toJson(QJsonDocument::Compact));
		}
		else if(defaultConfig[key].isArray())
		{
			defValueList << QString(QJsonDocument(defaultConfig[key].toArray()).toJson(QJsonDocument::Compact));
		}
	}

	// fill database with default data if required
	for(const auto key : keyList)
	{
		QString val = defValueList.takeFirst();
		// prevent overwrite
		if(!_sTable->recordExist(key))
			_sTable->createSettingsRecord(key,val);
	}

	// need to validate all data in database constuct the entire data object
	// TODO refactor schemaChecker to accept QJsonArray in validate(); QJsonDocument container? To validate them per entry...
	QJsonObject dbConfig;
	for(const auto key : keyList)
	{
		QJsonDocument doc = _sTable->getSettingsRecord(key);
		if(doc.isArray())
			dbConfig[key] = doc.array();
		else
			dbConfig[key] = doc.object();
	}

	// validate full dbconfig against schema, on error we need to rewrite entire table
	QJsonSchemaChecker schemaChecker;
	schemaChecker.setSchema(schemaJson);
	if (!schemaChecker.validate(dbConfig).first)
	{
		Info(_log,"Table upgrade required...");
		dbConfig = schemaChecker.getAutoCorrectedConfig(dbConfig);
		saveSettings(dbConfig);
	}

	// store the current state
	_qconfig = dbConfig;

	Debug(_log,"Settings database initialized")
}

SettingsManager::~SettingsManager()
{
	delete _sTable;
}

const QJsonDocument SettingsManager::getSetting(const settings::type& type)
{
	//return _sTable->getSettingsRecord(settings::typeToString(type));

	// DEPRECATION - remove when migration to database is done
	QString key = settings::typeToString(type);
	if(_qconfig[key].isObject())
		return QJsonDocument(_qconfig[key].toObject());
	else
		return QJsonDocument(_qconfig[key].toArray());
}

const bool SettingsManager::saveSettings(QJsonObject config, const bool& correct)
{
	// we need to validate data against schema
	QJsonSchemaChecker schemaChecker;
	schemaChecker.setSchema(schemaJson);
	if (!schemaChecker.validate(config).first)
	{
		if(!correct)
		{
			Error(_log,"Failed to save configuration, errors during validation");
			return false;
		}
		Warning(_log,"Fixing json data!");
		config = schemaChecker.getAutoCorrectedConfig(config);

		foreach (auto & schemaError, schemaChecker.getMessages())
			Warning(_log, "Config Fix: %s", QSTRING_CSTR(schemaError));
	}

	// save data to file, remove when migration is done
	if(_hyperion != nullptr)
	{
		if(!JsonUtils::write(_hyperion->getConfigFilePath(), config, _log))
			return false;
	}

	// store the current state
	_qconfig = config;

	// extract keys and data
	QStringList keyList = config.keys();
	QStringList newValueList;
	for(const auto key : keyList)
	{
		if(config[key].isObject())
		{
			newValueList << QString(QJsonDocument(config[key].toObject()).toJson(QJsonDocument::Compact));
		}
		else if(config[key].isArray())
		{
			newValueList << QString(QJsonDocument(config[key].toArray()).toJson(QJsonDocument::Compact));
		}
	}

	// compare database data with new data to emit/save changes accordingly
	for(const auto key : keyList)
	{
		QString data = newValueList.takeFirst();
		if(_sTable->getSettingsRecordString(key) != data)
		{
			_sTable->createSettingsRecord(key, data);

			emit settingsChanged(settings::stringToType(key), QJsonDocument::fromJson(data.toLocal8Bit()));
		}
	}
	return true;
}
