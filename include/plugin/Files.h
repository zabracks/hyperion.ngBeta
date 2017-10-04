#pragma once

#include <utils/Logger.h>
#include <plugin/PluginDefinition.h>



// forward decl
class QTimer;
class HTTPUtils;
///
/// @brief Handle all file system related tasks and repo sync
///
class Files : public QObject
{
	Q_OBJECT

public:
	///
	/// @brief Constructor
	/// @param[in]  log      The Logger of the caller
	/// @param[in]  rootPath rootPath of hyperion user data
	///
	Files(Logger* log, const QString& rootPath);
	~Files();

	bool installPlugin(const QString& id);
	bool removePlugin(const QString& id);

private:
	/// Logger instance
	Logger* _log;
	/// HTTP Utils instance
	HTTPUtils* _http;

	/// plugins, package, config dir
	QString _pluginsDir;
	QString _packageDir;
	QString _configDir;

	/// available plugins url
	const QString _pUrl = "https://raw.githubusercontent.com/brindosch/plugins/master/plugins.json";

	/// List of installed plugins <id,definition>
	QMap<QString, PluginDefinition> _installedPlugins;
	/// List of available plugins <id,QJsonObject>
	QMap<QString, QJsonObject> _availablePlugins;

	/// Timer to refresh available plugins
	QTimer* _rTimer;
private slots:
	void updateInstalledPlugins(void);
	void updateAvailablePlugins(void);
	/// replys from http utils
	void replyReceived(bool success, int type, QString url, QByteArray data);
};
