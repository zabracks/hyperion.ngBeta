#include <utils/Stats.h>
#include <utils/SysInfo.h>
#include <HyperionConfig.h>
#include <db/MetaTable.h>

// qt includes
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include <QUrl>

Stats* Stats::instance = nullptr;

Stats::Stats(const QJsonObject& config, QObject* parent)
	: QObject(parent)
	, _log(Logger::getInstance("STATS"))
	, _metaTable(new MetaTable(this))
{
	Stats::instance = this;

	// get uuid
	_uuid = _metaTable->getUUID();

	// prep data
	handleDataUpdate(config);

	// QNetworkRequest Header
	_req.setRawHeader("Content-Type", "application/json");
   	_req.setRawHeader("Authorization", "Basic SHlwZXJpb25YbDQ5MlZrcXA6ZDQxZDhjZDk4ZjAwYjIw");

	connect(&_mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(resolveReply(QNetworkReply*)));

	// 7 days interval
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(sendHTTP()));
	timer->start(604800000);

	// delay initial check
	QTimer::singleShot(60000, this, SLOT(initialExec()));
}

Stats::~Stats()
{

}

void Stats::handleDataUpdate(const QJsonObject& config)
{
	// prepare content
	SysInfo::HyperionSysInfo data = SysInfo::get();

	QJsonObject system;
	system["kType"    ] = data.kernelType;
	system["arch"       ] = data.architecture;
	system["pType"      ] = data.productType;
	system["pVersion"   ] = data.productVersion;
	system["pName"      ] = data.prettyName;
	system["version"    ] = QString(HYPERION_VERSION);
	system["device"     ] = config["device"].toObject().take("type");
	system["id"         ] = _uuid;
	system["ledCount"   ] = config["leds"].toArray().size();
	system["comp_sm"    ] = config["smoothing"].toObject().take("enable");
	system["comp_bb"    ] = config["blackborderdetector"].toObject().take("enable");
	system["comp_fw"    ] = config["forwarder"].toObject().take("enable");
	system["comp_udpl"  ] = config["udpListener"].toObject().take("enable");
	system["comp_bobl"  ] = config["boblightServer"].toObject().take("enable");
	system["comp_pc"    ] = config["instCapture"].toObject().take("systemEnable");
	system["comp_uc"    ] = config["instCapture"].toObject().take("v4lEnable");

	QJsonDocument doc(system);
	_ba = doc.toJson();
}

void Stats::initialExec()
{
	if(_metaTable->updateRequired(_uuid))
	{
		QTimer::singleShot(0,this, SLOT(sendHTTP()));
	}
}

void Stats::sendHTTP()
{
	_req.setUrl(QUrl("https://api.hyperion-project.org/api/stats"));
	_mgr.post(_req,_ba);
}

void Stats::sendHTTPp()
{
	_req.setUrl(QUrl("https://api.hyperion-project.org/api/stats/"+_uuid));
	_mgr.put(_req,_ba);
}

void Stats::resolveReply(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError)
	{
		// update timestamp
		_metaTable->setTimestamp(_uuid);
		// already created, update entry
		if(reply->readAll().startsWith("null"))
		{
			QTimer::singleShot(0, this, SLOT(sendHTTPp()));
		}
	}
}
