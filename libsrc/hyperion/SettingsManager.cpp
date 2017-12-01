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

#include <QDebug>

SettingsManager::SettingsManager(Hyperion* hyperion, const quint8& instance)
	: _hyperion(hyperion)
	, _log(Logger::getInstance("SettingsManager"))
	, _sTable(new SettingsTable(instance))
	, _schemaJson()
{
	Q_INIT_RESOURCE(resource);
	//connect(this, &SettingsManager::settingsChanged, hyperion, &Hyperion::settingsChanged);
	// get schema
	try
	{
		_schemaJson = QJsonFactory::readSchema(":/hyperion-schema");
	}
	catch(const std::runtime_error& error)
	{
		throw std::runtime_error(error.what());
	}
	// get default config
	QJsonObject defaultConfig;
	if(!JsonUtils::readFile(":/hyperion_default.config", defaultConfig, _log))
		throw std::runtime_error("Failed to read default config");

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
		// prevent overwrite
		if(!_sTable->recordExist(key))
			_sTable->createSettingsRecord(key,defValueList.takeFirst());
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
	schemaChecker.setSchema(_schemaJson);
	if (!schemaChecker.validate(dbConfig).first)
	{
		Debug(_log,"Upgrade database!");
		dbConfig = schemaChecker.getAutoCorrectedConfig(dbConfig);
		saveSettings(dbConfig);
	}

	Debug(_log,"Settings database initialized")
}

SettingsManager::~SettingsManager()
{
	delete _sTable;
}

QJsonDocument SettingsManager::getSetting(const settings::type& type)
{
	return _sTable->getSettingsRecord(settings::typeToString(type));
}

bool SettingsManager::saveSettings(QJsonObject config, const bool& correct)
{
	// we need to validate data against schema
	QJsonSchemaChecker schemaChecker;
	schemaChecker.setSchema(_schemaJson);
	if (!schemaChecker.validate(config).first)
	{
		if(!correct)
		{
			Error(_log,"Failed to save configuration, there where errors during validation");
			return false;
		}
		Debug(_log,"Fixing data!");
		config = schemaChecker.getAutoCorrectedConfig(config);
	}

	// save this to file, remove when migration is done
	if(!JsonUtils::write(_hyperion->getConfigFilePath(), config, _log))
		return false;

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
			_sTable->updateSettingsRecord(key, data);

			emit settingsChanged(settings::stringToType(key), QJsonDocument::fromJson(data.toLocal8Bit()));
		}
	}
	return true;
}
