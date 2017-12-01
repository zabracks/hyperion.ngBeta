#pragma once

// hyperion
#include <db/DBManager.h>

// qt
#include <QDateTime>
#include <QJsonDocument>

///
/// @brief settings table db interface
///
class SettingsTable : public DBManager
{

public:
	/// construct wrapper with settings table
	SettingsTable(const quint8& instance)
		: _hyperion_inst(instance)
	{
		setTable("settings");
		// create table columns
		createTable(QStringList()<<"updated_at TEXT DEFAULT CURRENT_TIMESTAMP"<<"type TEXT"<<"config TEXT"<<"hyperion_inst INTEGER");
	};
	~SettingsTable(){};

	///
	/// @brief      Create a new settings record
	/// @param[in]  type           type of setting
	/// @param[in]  config         The configuration data
	/// @return     true on success else false
	///
	inline bool createSettingsRecord(const QString& type, const QString& config) const
	{
		QVariantMap map;
		map["config"] = config;

		VectorPair cond;
		cond.append(CPair("type",type));
		// when a setting is not global we are searching also for the instance
		if(!isSettingGlobal(type))
			cond.append(CPair("AND hyperion_inst",_hyperion_inst));
		if(!createRecord(cond, map))
			return false;
		return true;
	}

	///
	/// @brief Update 'config' and 'updated_at' column
	/// @param[in]  type   The settings type
	/// @param[in]  data   The json data as string
	/// @return            true on success else false
	///
	inline bool updateSettingsRecord(const QString& type, const QString& data) const
	{
		QVariantMap map;
		map["config"] = data;
		map["updated_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

		VectorPair cond;
		cond.append(CPair("type",type));
		// when a setting is not global we are searching also for the instance
		if(!isSettingGlobal(type))
			cond.append(CPair("AND hyperion_inst",_hyperion_inst));
		if(!createRecord(cond, map))
			return false;
		return true;
	}

	///
	/// @brief      Test if record exist, type can be global setting or local (instance)
	/// @param[in]  type           type of setting
	/// @param[in]  hyperion_inst  The instance of hyperion assigned (might be empty)
	/// @return     true on success else false
	///
	inline bool recordExist(const QString& type) const
	{
		VectorPair cond;
		cond.append(CPair("type",type));
		// when a setting is not global we are searching also for the instance
		if(!isSettingGlobal(type))
			cond.append(CPair("AND hyperion_inst",_hyperion_inst));
		if(recordExists(cond))
			return true;
		return false;
	}

	///
	/// @brief Get 'config' column of settings entry as QJsonDocument
	/// @param[in]  type   The settings type
	/// @return            The QJsonDocument
	///
	inline QJsonDocument getSettingsRecord(const QString& type) const
	{
		QVariantMap results;
		VectorPair cond;
		cond.append(CPair("type",type));
		// when a setting is not global we are searching also for the instance
		if(!isSettingGlobal(type))
			cond.append(CPair("AND hyperion_inst",_hyperion_inst));
		getRecord(cond, results, QStringList("config"));
		return QJsonDocument::fromJson(results["config"].toByteArray());
	}

	///
	/// @brief Get 'config' column of settings entry as QString
	/// @param[in]  type   The settings type
	/// @return            The QString
	///
	inline QString getSettingsRecordString(const QString& type) const
	{
		QVariantMap results;
		VectorPair cond;
		cond.append(CPair("type",type));
		// when a setting is not global we are searching also for the instance
		if(!isSettingGlobal(type))
			cond.append(CPair("AND hyperion_inst",_hyperion_inst));
		getRecord(cond, results, QStringList("config"));
		return results["config"].toString();
	}

	bool isSettingGlobal(const QString& type) const
	{
		// list of global settings
		QStringList list;
		// server port services
		list << "boblightServer" << "jsonServer" << "protoServer" << "udpListener" << "webConfig"
		// capture
		<< "framegrabber" << "grabberV4L2"
		// other
		<< "logger";

		return list.contains(type);
	}

private:
	const quint8 _hyperion_inst;
};
