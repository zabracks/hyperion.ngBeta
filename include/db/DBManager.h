#pragma once

#include <utils/Logger.h>
#include <QMap>
#include <QVariant>

class QSqlDatabase;

///
/// @brief Database interface for SQLite3.
///        Inherit this class to create component specific methods based on this interface
///        Usage: setTable(name) once before you use read/write actions
///        To use another database use setDb(newDB) (defaults to "hyperion")
///
class DBManager : public QObject
{
	Q_OBJECT

public:
	DBManager();
	~DBManager();

	/// set root path
	void setRootPath(const QString& rootPath);
	/// define the database to work with
	void setDB(const QString& dbn);
	/// set a table to work with
	void setTable(const QString& table);

	/// get current database object set with setDB()
	QSqlDatabase getDB() const;

	///
	/// @brief Create a table (if required) with the given columns. Older tables will be updated accordingly with missing columns
	///        Does not delete or migrate old columns
	/// @param[in]  columns  The columns of the table. Requires at least one entry!
	/// @return              True on success else false
	///
	bool createTable(QStringList& columns) const;

	///
	/// @brief Create a column if the column already exists returns false and logs error
	/// @param[in]  column   The column of the table
	/// @return              True on success else false
	///
	bool createColumn(const QString& column) const;

	///
	/// @brief Check if record (id) exists in table, of column
	/// @param[in]  column   The column name of the table
	/// @param[in]  id       The value of the column to search for
	/// @return              True on success else false
	///
	bool recordExists(const QString& column, const QString& id) const;

	///
	/// @brief Create a new record in table with idfield and idvalue (primary key) and additional key:value pairs in columns
	///        If the record already exists, updateRecord() is called to update the existing record identified by idfield and idvalue
	/// @param[in]  idfield  primary key column name
	/// @param[in]  idvalue  primary key column value
	/// @param[in]  columns  columns to create or update
	/// @return              True on success else false
	///
	bool createRecord(const QString& idfield, const QString& idvalue, const QMap<QString,QVariant>& columns) const;

	///
	/// @brief Update a record in table with idfield and idvalue (primary key) and additional key:value pairs in columns
	/// @param[in]  idfield  primary key column name
	/// @param[in]  idvalue  primary key column value
	/// @param[in]  columns  columns to update
	/// @return              True on success else false
	///
	bool updateRecord(const QString& idfield, const QString& idvalue, const QMap<QString,QVariant>& columns) const;

private:

	Logger* _log;
	/// databse connection & file name, defaults to hyperion
	QString _dbn = "hyperion";
	/// table in database
	QString _table;
};
