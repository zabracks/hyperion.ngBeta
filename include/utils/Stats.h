// qt includes
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

// hyperion includes
#include <utils/Logger.h>
#include <hyperion/Hyperion.h>

class MetaTable;

class Stats : public QObject
{
	Q_OBJECT

public:
	static Stats* getInstance() { return instance; };
	static Stats* instance;

	const QString & getID() { return _uuid; };

	void handleDataUpdate(const QJsonObject& config);

private:
	friend class HyperionDaemon;
	Stats(const QJsonObject& config, QObject* parent = nullptr);
	~Stats();

private:
	Logger* _log;
	MetaTable* _metaTable;
	QString _uuid;
	QByteArray _ba;
	QNetworkRequest _req;
	QNetworkAccessManager _mgr;

	bool trigger(bool set = false);

private slots:
	void initialExec();
	void sendHTTP();
	void sendHTTPp();
	void resolveReply(QNetworkReply *reply);

};
