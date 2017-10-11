#pragma once

#include <utils/Logger.h>

#include <QNetworkAccessManager>

///
/// @brief Util class for HTTP GET/POST/PUT requests with error resolution
///
class HTTPUtils: public QObject
{
	Q_OBJECT

public:
	///
	/// @brief Constructor with log instance of caller
	///
	HTTPUtils(Logger* log);
	~HTTPUtils();

	///
	/// @brief send a GET request to given url
	/// @param[in]   url   The target url
	/// @param[in]   id    id which is part of reply emit
	/// @return            true on success or false when network isn't ready or url is invalid
	///
	bool sendGet(const QString& url, const QString& id="unknown");

	///
	/// @brief send a POST request to given url
	/// @param[in]   url   The target url
	/// @param[in]   ba    POST body data
	/// @param[in]   id    id which is part of reply emit
	/// @return            true on success or false when network isn't ready or url is invalid
	///
	bool sendPost(const QString& url, const QByteArray& ba, const QString& id="unknown");

	///
	/// @brief send a PUT request to given url
	/// @param[in]   url   The target url
	/// @param[in]   ba    PUT body data
	/// @param[in]   id    id which is part of reply emit
	/// @return            true on success or false when network isn't ready or url is invalid
	///
	bool sendPut(const QString& url, const QByteArray& ba, const QString& id="unknown");

private:
	/// Logger instance of creator
	Logger* _log;
	/// QNAM for this instance
	QNetworkAccessManager* _manager;
	/// current state of network
	bool _networkAccessible;
	/// contains pending replies with id association
	QMap<QNetworkReply*,QString> _currReplies;

	/// check if url is valid and network ready
	bool isValid(const QUrl& url);

signals:
	///
	/// @brief Emits whenever a request returns a reply
	/// @param[out]   success   true when reply contains no error, else false
	/// @param[out]   type      operation code of this reply (HEAD=1,GET=2,PUT,POST,...)
	/// @param[out]   id        id given on send
	/// @param[out]   data      Data of the reply (if available)
	///
	void replyReceived(bool success, int type, QString id, QByteArray data);

private slots:
	/// read reply from QNAM
	void readReply(QNetworkReply* reply);
	/// update _networkAccessible according to current network state
	void networkStateChanged(QNetworkAccessManager::NetworkAccessibility accessible);
};
