#pragma once

// hyperion
#include <db/DBManager.h>

// qt
#include <QDateTime>

///
/// @brief plugin table specific database interface
///
class PluginTable : public DBManager
{

public:
	/// construct wrapper with plugins table and columns
	PluginTable(const QString& id)
	: _hyID(id)
	{
		setTable("plugins");

		createTable(QStringList()<<"updated_at TEXT DEFAULT CURRENT_TIMESTAMP"<<"id TEXT"<<"enabled INTEGER DEFAULT 0"<<"auto_update INTEGER DEFAULT 1"<<"hyperion_name TEXT");
	};
	~PluginTable(){};

	///
	/// @brief      Create a plugin record
	/// @param[in]  id      plugin id
	/// @return             true on success else false
	///
	inline const bool createPluginRecord(const QString& id) const
	{
		VectorPair cond;
		cond.append(CPair("id",id));
		cond.append(CPair("AND hyperion_name",_hyID));
		return createRecord(cond);
	}

	///
	/// @brief      Update 'updated_at' field of plugin record
	/// @param[in]  id      plugin id
	/// @return             true on success else false
	///
	inline const bool setPluginUpdatedAt(const QString& id) const
	{
		QVariantMap map;
		VectorPair cond;
		cond.append(CPair("id",id));
		map["updated_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
		return createRecord(cond, map);
	}

	///
	/// @brief      Update plugin enabled state in database
	/// @param[in]  id      plugin id
	/// @param[in]  enable  true if plugin should be set to enabled
	/// @return             true on success else false
	///
	inline const bool setPluginEnable(const QString& id, const bool& enable) const
	{
		QVariantMap map;
		map["enabled"] = enable;
		VectorPair cond;
		cond.append(CPair("id",id));
		cond.append(CPair("AND hyperion_name",_hyID));
		return createRecord(cond, map);
	}

	///
	/// @brief Get 'enabled' state of plugin
	/// @param[in]  id     plugin id
	/// @return            enabled state
	///
	inline const bool isPluginEnabled(const QString& id) const
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
	inline const bool setPluginAutoUpdateEnable(const QString& id, const bool& enable) const
	{
		QVariantMap map;
		map["auto_update"] = enable;
		VectorPair cond;
		cond.append(CPair("id",id));
		return createRecord(cond, map);
	}

	///
	/// @brief Get 'auto_update' state of plugin
	/// @param[in]  id     plugin id
	/// @return            auto_update state
	///
	inline const bool isPluginAutoUpdateEnabled(const QString& id) const
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
	inline const bool deletePluginRecord(const QString& id) const
	{
		VectorPair cond;
		cond.append(CPair("id",id));
		return deleteRecord(cond);
	}
private:
	// Hyperion id
	const QString _hyID;
};
