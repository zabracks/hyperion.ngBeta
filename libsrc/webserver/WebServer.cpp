#include "webserver/WebServer.h"
#include "StaticFileServing.h"

#include <QFileInfo>

WebServer::WebServer(QObject * parent)
	:  QObject(parent)
	, _hyperion(Hyperion::getInstance())
	, _server(nullptr)
{
	Logger* log = Logger::getInstance("WEBSERVER");
	_port       = WEBSERVER_DEFAULT_PORT;
	_baseUrl    = WEBSERVER_DEFAULT_PATH;
	const QJsonObject config = _hyperion->getQJsonConfig();

	bool webconfigEnable = true;

	if (config.contains("webConfig"))
	{
		const QJsonObject webconfigConfig = config["webConfig"].toObject();
		webconfigEnable = webconfigConfig["enable"].toBool(true);
		_port    = webconfigConfig["port"].toInt(_port);
		_baseUrl = webconfigConfig["document_root"].toString(_baseUrl);
	}

	if ( (_baseUrl != ":/webconfig") && !_baseUrl.trimmed().isEmpty())
	{
		QFileInfo info(_baseUrl);
		if (!info.exists() || !info.isDir())
		{
			Error(log, "document_root '%s' is invalid, set to default '%s'", _baseUrl.toUtf8().constData(), WEBSERVER_DEFAULT_PATH.toUtf8().constData());
			_baseUrl = WEBSERVER_DEFAULT_PATH;
		}
	}
	else
		_baseUrl = WEBSERVER_DEFAULT_PATH;

	Debug(log, "WebUI initialized, document root: %s", _baseUrl.toUtf8().constData());
	if ( webconfigEnable )
	{
		start();
	}
}


WebServer::~WebServer()
{
	stop();
}


void WebServer::start()
{
	if ( _server == nullptr )
		_server = new StaticFileServing (_hyperion, _baseUrl, _port, this);
}

void WebServer::stop()
{
	if ( _server != nullptr )
	{
		delete _server;
		_server = nullptr;
	}
}
