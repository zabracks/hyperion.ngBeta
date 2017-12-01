#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QString>
#include <hyperion/Hyperion.h>

class StaticFileServing;

class WebServer : public QObject {
	Q_OBJECT

public:
	WebServer (QObject * parent = 0);

	virtual ~WebServer (void);

	void start();
	void stop();

	quint16 getPort() { return _port; };

private:
	Hyperion*            _hyperion;
	QString              _baseUrl;
	quint16              _port;
	StaticFileServing*   _server;

	const QString        WEBSERVER_DEFAULT_PATH = ":/webconfig";
	const quint16        WEBSERVER_DEFAULT_PORT = 8090;
};

#endif // WEBSERVER_H
