#pragma once

// hyperion
#include <db/DBManager.h>

// qt
#include <QDateTime>

///
/// @brief plugin specific database wrapper
///
class PDBWrapper : public DBManager
{

public:
	/// construct wrapper with plugins table
	PDBWrapper(const QString& id){ _hyID = id; setTable("plugins"); };
	~PDBWrapper(){};

	///
	/// @brief      Create a plugin record
	/// @param[in]  id      plugin id
	/// @return             true on success else false
	///
	inline bool createPluginRecord(const QString& id) const
	{
		VectorPair cond;
		cond.append(CPair("id",id));
		cond.append(CPair("AND hyperion_name",_hyID));
		if(!createRecord(cond))
			return false;
		return true;
	}

	///
	/// @brief      Update 'updated_at' field of plugin record
	/// @param[in]  id      plugin id
	/// @return             true on success else false
	///
	inline bool setPluginUpdatedAt(const QString& id) const
	{
		QVariantMap map;
		VectorPair cond;
		cond.append(CPair("id",id));
		map["updated_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
		if(!createRecord(cond, map))
			return false;
		return true;
	}

	///
	/// @brief      Update plugin enabled state in database
	/// @param[in]  id      plugin id
	/// @param[in]  enable  true if plugin should be set to enabled
	/// @return             true on success else false
	///
	inline bool setPluginEnable(const QString& id, const bool& enable) const
	{
		QVariantMap map;
		map["enabled"] = enable;
		VectorPair cond;
		cond.append(CPair("id",id));
		cond.append(CPair("AND hyperion_name",_hyID));
		if(!createRecord(cond, map))
			return false;
		return true;
	}

	///
	/// @brief Get 'enabled' state of plugin
	/// @param[in]  id     plugin id
	/// @return            enabled state
	///
	inline bool isPluginEnabled(const QString& id) const
	{
		QVariantMap results;
		VectorPair cond;
		cond.append(CPair("id",id));
		cond.append(CPair("AND hyperion_name",_hyID));
		getRecord(cond, results, QStringList("enabled"));
		return results["enabled"].toBool();
	}

	///
	/// @brief Set 'auto_update' state of plugin
	/// @param[in]  id     plugin id
	/// @param[in]  enable true if plugin should auto update
	/// @return            true on success else false
	///
	inline bool setPluginAutoUpdateEnable(const QString& id, const bool& enable) const
	{
		QVariantMap map;
		map["auto_update"] = enable;
		VectorPair cond;
		cond.append(CPair("id",id));
		if(!createRecord(cond, map))
			return false;
		return true;
	}

	///
	/// @brief Get 'auto_update' state of plugin
	/// @param[in]  id     plugin id
	/// @return            auto_update state
	///
	inline bool isPluginAutoUpdateEnabled(const QString& id) const
	{
		QVariantMap results;
		VectorPair cond;
		cond.append(CPair("id",id));
		getRecord(cond, results, QStringList("auto_update"));
		return results["auto_update"].toBool();
	}

	///
	/// @brief      Delete a plugin record
	/// @param[in]  id      plugin id
	/// @return             true on success else false
	///
	inline bool deletePluginRecord(const QString& id) const
	{
		VectorPair cond;
		cond.append(CPair("id",id));
		if(!deleteRecord(cond))
			return false;
		return true;
	}
private:
	// Hyperion id
	QString _hyID;
};
