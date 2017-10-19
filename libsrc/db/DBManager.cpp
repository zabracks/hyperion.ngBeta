#include <db/DBManager.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDir>

#include <QDebug>

// not in header because of linking
static QString _rootPath;

DBManager::DBManager()
	: QObject()
	, _log(Logger::getInstance("DB"))
{

	// main table
	//QStringList mainc;
	//mainc << "id INTEGER PRIMARY KEY" << "name TEXT" << "created_at DATETIME";
	//if(!createTable("main",mainc))
	//	return;

	// plugin table
	//if(!createTable("plugins",QStringList()<<"id TEXT PRIMARY KEY"<<"enabled INTEGER DEFAULT 1"<<"updated_at DATETIME"))
	//	return;

	//createPluginsRecord("service.kodi", false);
	//QMap<QString,QVariant> map;
	//map["enabled"] = false;

	//createRecord("plugins","id","service.another",map);

}

DBManager::~DBManager()
{
}

void DBManager::setRootPath(const QString& rootPath)
{
	_rootPath = rootPath;
	// create directory
	QDir().mkpath(_rootPath+"/db");
}

void DBManager::setDB(const QString& dbn)
{
	_dbn = dbn;
	// new database connection if not found
	if(!QSqlDatabase::contains(dbn))
	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",dbn);
		db.setDatabaseName(_rootPath+"/db/"+dbn+".db");
		if(!db.open())
		{
			Error(_log, QSTRING_CSTR(db.lastError().text()));
			throw std::runtime_error("Failed to open database connection!");
		}
	}
}

void DBManager::setTable(const QString& table)
{
	_table = table;
}

bool DBManager::createRecord(const QString& idfield, const QString& idvalue, const QMap<QString,QVariant>& columns) const
{
	if(recordExists(idfield, idvalue))
	{
		if(!updateRecord(idfield, idvalue, columns))
			return false;

		return true;
	}

	QSqlDatabase idb = getDB();
	QSqlQuery query(idb);
	QVariantList values;
	QStringList prep(idfield);
	QStringList placeh("?");

	QMap<QString, QVariant>::const_iterator i = columns.constBegin();
	while (i != columns.constEnd()) {
		prep.append(i.key());
		values += i.value();
		placeh.append("?");

		++i;
	}

	query.prepare(QString("INSERT INTO %1 ( %2 ) VALUES ( %3 )").arg(_table,prep.join(", ")).arg(placeh.join(", ")));
	query.addBindValue(idvalue);
	for(const auto variant : values)
	{
		QVariant::Type t = variant.type();
		switch(t)
		{
			case QVariant::UInt:
			case QVariant::Int:
			case QVariant::Bool:
				query.addBindValue(variant.value<int>());
				break;

			default:
				query.addBindValue(variant.value<QString>());
				break;
		}
	}

	if(!query.exec())
	{
		Error(_log, "Failed to create record: '%s' in table: '%s' Error: %s", QSTRING_CSTR(idvalue), QSTRING_CSTR(_table), QSTRING_CSTR(idb.lastError().text()));
		return false;
	}
	return true;
}

bool DBManager::recordExists(const QString& column, const QString& id) const
{
	QSqlDatabase idb = getDB();
	QSqlQuery query(idb);
	if(!query.exec(QString("SELECT %1 FROM %2").arg(column,_table)))
	{
		Error(_log, "Failed recordExists() at column: '%s' in table: '%s' Error: %s", QSTRING_CSTR(column), QSTRING_CSTR(_table), QSTRING_CSTR(idb.lastError().text()));
		return false;
	}
	while(query.next())
	{
		if(query.value(0).toString() == id)
			return true;
	}

	return false;
}

bool DBManager::updateRecord(const QString& idfield, const QString& idvalue, const QMap<QString,QVariant>& columns) const
{
	QSqlDatabase idb = getDB();
	QSqlQuery query(idb);
	QVariantList values;
	QString prep;

	QMap<QString, QVariant>::const_iterator i = columns.constBegin();
	while (i != columns.constEnd()) {
		prep += i.key()+"=? ";
		values += i.value();

		++i;
	}

	query.prepare(QString("UPDATE %1 SET %2WHERE %3=?").arg(_table,prep).arg(idfield));
	for(const auto variant : values)
	{
		QVariant::Type t = variant.type();
		switch(t)
		{
			case QVariant::UInt:
			case QVariant::Int:
			case QVariant::Bool:
				query.addBindValue(variant.value<int>());
				break;

			default:
				query.addBindValue(variant.value<QString>());
				break;
		}
	}
	query.addBindValue(idvalue);

	if(!query.exec())
	{
		Error(_log, "Failed to update record: '%s' in table: '%s' Error: %s", QSTRING_CSTR(idvalue), QSTRING_CSTR(_table), QSTRING_CSTR(idb.lastError().text()));
		return false;
	}
	return true;
}

QSqlDatabase DBManager::getDB() const
{
	return QSqlDatabase::database(_dbn);
}

bool DBManager::createTable(QStringList& columns) const
{
	if(columns.isEmpty())
	{
		Error(_log,"Empty tables aren't supported!");
		return false;
	}

	QSqlDatabase idb = getDB();
	// create table if required
	QStringList tables = idb.tables();
	QSqlQuery query(idb);
	if(!tables.contains(_table))
	{
		// empty tables aren't supported by sqlite, add one column
		QString tcolumn = columns.takeFirst();
		// default CURRENT_TIMESTAMP is not supported by ALTER TABLE
		if(!query.exec(QString("CREATE TABLE %1 ( %2 )").arg(_table,tcolumn)))
		{
			Error(_log, "Failed to create table: '%s' Error: %s", QSTRING_CSTR(_table), QSTRING_CSTR(idb.lastError().text()));
			return false;
		}
	}
	// create columns if required
	QSqlRecord rec = idb.record(_table);
	int err = 0;
	for(const auto column : columns)
	{
		QStringList id = column.split(' ');
		if(rec.indexOf(id.at(0)) == -1)
		{
			if(!createColumn(column))
			{
				err++;
			}
		}
	}
	if(err)
		return false;

	return true;
}

bool DBManager::createColumn(const QString& column) const
{
	QSqlDatabase idb = getDB();
	QSqlQuery query(idb);
	if(!query.exec(QString("ALTER TABLE %1 ADD COLUMN %2").arg(_table,column)))
	{
		Error(_log, "Failed to create column: '%s' in table: '%s' Error: %s", QSTRING_CSTR(column), QSTRING_CSTR(_table), QSTRING_CSTR(idb.lastError().text()));
		return false;
	}
	return true;
}
