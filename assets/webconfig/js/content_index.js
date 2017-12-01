$(document).ready( function() {
	var uiLock = false;

	loadContentTo("#container_connection_lost","connection_lost");
	loadContentTo("#container_restart","restart");
	initWebSocket();

	$(hyperion).on("cmd-serverinfo",function(event){
		serverInfo = event.response.info;
		// protect components from serverinfo updates
		if(!compsInited)
		{
			comps = event.response.info.components
			compsInited = true;
		}
		$(hyperion).trigger("ready");

		if (serverInfo.hyperion.config_modified)
			$("#hyperion_reload_notify").fadeIn("fast");
		else
			$("#hyperion_reload_notify").fadeOut("fast");

		if (serverInfo.hyperion.enabled)
			$("#hyperion_disabled_notify").fadeOut("fast");
		else
			$("#hyperion_disabled_notify").fadeIn("fast");

		if (!serverInfo.hyperion.config_writeable)
		{
			showInfoDialog('uilock',$.i18n('InfoDialog_nowrite_title'),$.i18n('InfoDialog_nowrite_text'));
			$('#wrapper').toggle(false);
			uiLock = true;
		}
		else if (uiLock)
		{
			$("#modal_dialog").modal('hide');
			$('#wrapper').toggle(true);
			uiLock = false;
		}

		updateSessions();
	}); // end cmd-serverinfo

	$(hyperion).on("cmd-sessions-update", function(event) {
		serverInfo.sessions = event.response.data;
		updateSessions();
	});

	$(hyperion).one("cmd-management-plugins", function(event) {
		requestPluginsInitData()
	});

	$(hyperion).one("cmd-plugin-getInitData", function(event) {
		plugins_installed = event.response.info.installedPlugins;
		plugins_available = event.response.info.availablePlugins;
		requestServerInfo();
	});

	$(hyperion).one("cmd-sysinfo", function(event) {
		sysInfo = event.response.info;

		currentVersion = sysInfo.hyperion.version;

		requestPluginsManagement()
	});

	$(hyperion).one("cmd-config-getschema", function(event) {
		serverSchema = event.response.info;
		requestServerConfig();

		schema = serverSchema.properties;
	});

	$(hyperion).one("cmd-config-getconfig", function(event) {
		serverConfig = event.response.info;
		requestSysInfo();

		showOptHelp = serverConfig.general.showOptHelp;
	});

	$(hyperion).on("error",function(event){
		showInfoDialog("error","Error", event.reason);
	});

	$(hyperion).on("open",function(event){
		requestServerConfigSchema();
	});

	$(hyperion).one("ready", function(event) {
		loadContent();
	});

	$(hyperion).on("cmd-components-update", function(event) {
		let obj = event.response.data
		comps.forEach((entry, index) => {
			if (entry.name === obj.name){
				comps[index] = obj;
			}
		});
		// notify the update
		$(hyperion).trigger("components-updated");
	});

	$("#btn_hyperion_reload").on("click", function(){
		initRestart();
	});

	$(".mnava").bind('click.menu', function(e){
		loadContent(e);
		window.scrollTo(0, 0);
	});

});

$(function(){
	var sidebar = $('#side-menu');  // cache sidebar to a variable for performance
	sidebar.delegate('a.inactive','click',function(){
		sidebar.find('.active').toggleClass('active inactive');
		$(this).toggleClass('active inactive');
	});
});
