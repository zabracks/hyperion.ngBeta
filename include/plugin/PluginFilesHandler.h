#pragma once

#include <utils/Logger.h>
#include <plugin/PluginDefinition.h>

// forward decl
class QTimer;
class HTTPUtils;
class PluginTable;
class Plugins;
///
/// @brief Handle all file system related tasks, init downloads and repo sync
///
class PluginFilesHandler : public QObject
{
	Q_OBJECT
private:
	friend class HyperionDaemon;
	PluginFilesHandler(const QString& rootPath, QObject* parent = nullptr);

public:
	static PluginFilesHandler* pfhInstance;
	static PluginFilesHandler* getInstance() { return pfhInstance; };

public:
	~PluginFilesHandler();

	/// init files
	void init(void);

	QMap<QString, PluginDefinition> getInstalledPlugins(void) const { return _installedPlugins; };
	QMap<QString, PluginDefinition> getAvailablePlugins(void) const { return _availablePlugins; };

	void registerMe(Plugins* instance);

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
	/// PluginTable instance
	PluginTable* _PDB;
	/// HTTP Utils instance
	HTTPUtils* _http;

	/// plugins, package, config dir
	QString _pluginsDir;
	QString _packageDir;

	/// repo url
	const QString _rUrl = "https://api.hyperion-project.org/redir.php?plugin_repo=true";
	/// download dir url
	const QString _dUrl = "https://api.hyperion-project.org/redir.php?plugin=";

	/// List of installed plugins <id,definition>
	QMap<QString, PluginDefinition> _installedPlugins;
	/// List of available plugins <id,definition>
	QMap<QString, PluginDefinition> _availablePlugins;

	/// Timer to refresh available plugins
	QTimer* _rTimer;

	/// store current plugins instance pointer
	QVector<Plugins*> _registeredInstances;

	/// returs int representation of a version string
	int getIntV(QString str);

	/// compare installed plugins with available to trigger updates
	void doUpdateCheck(void);

	/// install or update the given plugin
	void installPlugin(const QString& id);

	///
	/// @brief remove a plugin by id from filesystem, all registered Plugins instances will stop this plugin (blocking) before
	/// @pram id  The plugin id to delete
	///
	void removePlugin(const QString& id);

	/// Create plugin definition from plugin id, return true on success; push to _installedPlugins;
	bool updateInstalledPlugin(const QString& tid);

	/// initial creation of _installedPlugins definitions, call just once on startup
	void readInstalledPlugins(void);

public slots:
	/// is called from Plugins
	void doPluginAction(PluginAction action, QString id, bool success = true, PluginDefinition def = PluginDefinition());

	/// update available plugins
	void updateAvailablePlugins(void);

private slots:

	/// replys from http utils
	void replyReceived(bool success, int type, QString id, QByteArray data);
};
