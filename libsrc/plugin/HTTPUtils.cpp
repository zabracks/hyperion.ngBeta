#include <plugin/HTTPUtils.h>

// qt include
#include <QNetworkReply>

HTTPUtils::HTTPUtils(Logger* log)
	:QObject()
	, _log(log)
	, _manager(new QNetworkAccessManager())
{
	networkStateChanged(_manager->networkAccessible());

	connect(_manager, &QNetworkAccessManager::finished, this, &HTTPUtils::readReply);
	connect(_manager, &QNetworkAccessManager::networkAccessibleChanged, this, &HTTPUtils::networkStateChanged);
}

HTTPUtils::~HTTPUtils()
{
	delete _manager;
}


void HTTPUtils::readReply(QNetworkReply* reply)
{
	// Remove start - Remove this with QT 5.6 and use QNetworkRequest::setAttribute(QNetworkRequest::FollowRedirectsAttribute, true); to enable auto redirects
	QString possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
	// no check for endless redirect loops, if url is not empty we got a redirect
	if(!possibleRedirectUrl.isEmpty())
	{
		Debug(_log,"Redirect to url %s",QSTRING_CSTR(possibleRedirectUrl));
		sendGet(possibleRedirectUrl);
		return;
	}
	// Remove end

	const QNetworkRequest &req = reply->request();
	QByteArray ba = reply->readAll();
	int type = static_cast<int>(reply->operation());
	bool success = true;

	if(reply->error())
	{
		success = false;
		Error(_log, "Request failed (%s) for %s", QSTRING_CSTR(reply->errorString()), QSTRING_CSTR(req.url().toString()));
	}

 	emit replyReceived(success, type, req.url().toString(), ba);
	reply->deleteLater();
}

void HTTPUtils::networkStateChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
	_networkAccessible = (accessible == QNetworkAccessManager::Accessible) ? true : false;
}


bool HTTPUtils::isValid(const QUrl& url)
{
	if(_networkAccessible)
	{
		if(url.isValid())
		{
			return true;
		}
		Error(_log, "URL isn't valid! %s", QSTRING_CSTR(url.url()));
		return false;
	}
	Error(_log, "Network is not available to send HTTP requests");
	return false;
}

bool HTTPUtils::sendGet(const QString& url)
{
	const QUrl iurl(url);
	if(isValid(iurl))
	{
		const QNetworkRequest req(iurl);
		_manager->get(req);
		return true;
	}
	Error(_log, "Failed to send GET request to %s", QSTRING_CSTR(iurl.url()));
	return false;

}

bool HTTPUtils::sendPost(const QString& url, const QByteArray& ba)
{
	const QUrl iurl(url);
	if(isValid(iurl))
	{
		const QNetworkRequest req(iurl);
		_manager->post(req, ba);
		return true;
	}
	Error(_log, "Failed to send POST request to %s", QSTRING_CSTR(iurl.url()));
	return false;
}

bool HTTPUtils::sendPut(const QString& url, const QByteArray& ba)
{
	const QUrl iurl(url);
	if(isValid(iurl))
	{
		const QNetworkRequest req(iurl);
		_manager->put(req, ba);
		return true;
	}
	Error(_log, "Failed to send PUT request to %s", QSTRING_CSTR(iurl.url()));
	return false;
}
