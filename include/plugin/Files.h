#pragma once

#include <utils/Logger.h>
#include <plugin/PluginDefinition.h>

// forward decl
class QTimer;
class HTTPUtils;
///
/// @brief Handle all file system related tasks, init downloads and repo sync
///
class Files : public QObject
{
	Q_OBJECT

public:
	///
	/// @brief Constructor
	/// @param[in]  rootPath rootPath of hyperion user data
	/// @param[in]  id       id of hyperion
	///
	Files(const QString& rootPath, const QString& id);
	~Files();

	QMap<QString, PluginDefinition> getInstalledPlugins(void){ return _installedPlugins; };
	QMap<QString, PluginDefinition> getAvailablePlugins(void){ return _availablePlugins; };

	bool getPluginDefinition(const QString& id, PluginDefinition& def);

signals:
	///
	/// @brief emits whenever a plugin action is ongoing
	/// @param action   action from enum
	/// @param id       plugin id
	/// @param def      PluginDefinition (optional)
	/// @param success  true if action was a success, else false
	///
	void pluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

private:
	/// Logger instance
	Logger* _log;
	/// id of hyperion
	const QString _id;
	/// HTTP Utils instance
	HTTPUtils* _http;

	/// plugins, package, config dir
	QString _pluginsDir;
	QString _packageDir;
	QString _configDir;

	/// repo url
	const QString _rUrl = "https://raw.githubusercontent.com/brindosch/plugins/master/plugins.json";
	/// download dir url
	const QString _dUrl = "https://raw.githubusercontent.com/brindosch/plugins/master/";

	/// List of installed plugins <id,definition>
	QMap<QString, PluginDefinition> _installedPlugins;
	/// List of available plugins <id,definition>
	QMap<QString, PluginDefinition> _availablePlugins;

	/// Timer to refresh available plugins
	QTimer* _rTimer;

	/// returs int representation of a version string
	int getIntV(QString str);

	/// compare installed plugins with available to trigger updates
	void doUpdateCheck(void);

	/// install or update the given plugin
	void installPlugin(const QString& id);

	/// Create plugin definition from plugin id, return true on success; push to _installedPlugins
	bool updateInstalledPlugin(const QString& tid, PluginDefinition& newDef);

	/// initial creation of _installedPlugins definitions
	void updateInstalledPlugins(void);

private slots:
	/// is called when a pluginAction is ongoing
	void doPluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

	/// update available plugins
	void updateAvailablePlugins(void);

	/// replys from http utils
	void replyReceived(bool success, int type, QString id, QByteArray data);
};
