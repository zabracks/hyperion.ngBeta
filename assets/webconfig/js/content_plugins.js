$(document).ready(function() {
	performTranslation();

	function getHostnameURL(url)
	{
		var link = document.createElement('a');
		link.setAttribute('href', url);
		return link.protocol+"//"+link.hostname;
	}

	function PluginClick()
	{
		var splitted = this.id.split(':');
		var currID = splitted[1];
		if(this.id.startsWith("au"))
		{
			if(this.classList.contains('btn-success'))
			{
				requestPluginAutoUpdate(currID,false);
				this.classList.replace('btn-success', 'btn-danger');
			}
			else
			{
				requestPluginAutoUpdate(currID,true);
				this.classList.replace('btn-danger', 'btn-success');
			}
		}
		else if(this.id.startsWith("co"))
		{
			// settings
			$('#plugin_header').html('<i class="fa fa-fw fa-cube"></i>'+getPluginNameById(currID)+' - '+$.i18n('plugins_confit'));
			$('#plugin_footer').html('<button id="plugin_save" class="btn btn-primary" data-dismiss="modal"><i class="fa fa-fw fa-save"></i>'+$.i18n('general_btn_save')+'</button>\
										<button class="btn btn-danger" data-dismiss="modal"><i class="fa fa-fw fa-close"></i>'+$.i18n('general_btn_cancel')+'</button>');


			var p_schema = getPluginSchemaById(currID);
			var plugin_editor = createJsonEditor('plugin_body', {
				data: p_schema
			}, false, true);

			plugin_editor.on('change',function() {
				plugin_editor.validate().length ? $('#plugin_save').attr('disabled', true) : $('#plugin_save').attr('disabled', false);
			});

			var settings = getSettingsById(currID);
			// set values per entry to prevent schema overwrites on schema changes
			for(key in settings)
			{
				var ed = plugin_editor.getEditor('root.data.'+key);
				if(ed)
					ed.setValue(settings[key]);
			}

			$('#plugin_save').off().on('click', function(e){
				requestPluginSave(currID, plugin_editor.getValue().data);
				if(plugin_editor != null)
					plugin_editor.destroy();
			});

			$('#plugin_modal').modal({
				backdrop : "static",
				keyboard: false,
				show: true
			});
		}
	}


	function PluginInstall()
	{
		requestPluginInstall(this.id.split(":")[1]);
	}

	function isPluginInstalled(id)
	{
		for(key in plugins_installed)
		{
			if(key == id)
				return true;
		}
		return false;
	}

	function getInstDataById(id)
	{
		for(key in plugins_installed) {
			if(key == id){
		        return plugins_installed[key];
		    }
		}
	}

	function getPluginNameById(id)
	{
		for(key in plugins_installed) {
			if(key == id){
		        return plugins_installed[key].name;
		    }
		}
		for(key in plugins_available) {
			if(key == id){
				return plugins_available[key].name;
			}
		}
	}

	function getSettingsById(id)
	{
		for(key in plugins_installed) {
			if(key == id){
				return plugins_installed[key].settings;
			}
		}
	}

	function getPluginSchemaById(id)
	{
		for(key in plugins_installed) {
			if(key == id){
				return plugins_installed[key].settingsSchema;
			}
		}
	}

	function setSettings(id, data)
	{
		plugins_installed[id]["settings"] = data;
	}

	function modifyBtn(id, success)
	{
		var el = document.getElementById(id);
		if(el == null)
			console.log("ModifyBtn is null");
		if(success)
			el.classList.replace('btn-danger','btn-success');
		else
			el.classList.replace('btn-success','btn-danger');
	}

	function elEnable(id, state)
	{
		var el = document.getElementById(id);
		if(el == null)
			return;
		if(state && el.hasAttribute("disabled"))
			el.removeAttribute("disabled");
		else if(!state && !el.hasAttribute("disabled"))
			el.setAttribute("disabled", "disabled");
	}

	function removeEntry(id)
	{
		var el = document.getElementById(id);
		if(el == null)
			return;
		el.parentNode.removeChild(el);
	}

	function PluginDelete()
	{
		var id = this.id.split(":")[1];
		showInfoDialog('delplug',$.i18n('plugins_delreq_h'),$.i18n('plugins_delreq_t',getPluginNameById(id)));
		$("#id_btn_yes").off().on('click', function(){
			requestPluginDelete(id);
		});
	}

	function updateRepo()
	{
		requestPluginRepoUpdate();
	}

	function createPIWrap(id, data = "")
	{
		// return for module. plugins
		if(id.startsWith("module."))
			return;

		// data empty, get them
		if(data == "")
			data = getInstDataById(id);
		stop_btn_disabled = data.running ? '' : 'disabled="disabled"';
		start_btn_class = data.running ? "btn-success" : "btn-danger";
		autoupd_btn_class = data.autoupdate ? "btn-success" : "btn-danger";
		start_btn_text = $.i18n('plugins_start');
		instCont.append('\
		<div class="col-lg-6" id="pinst_cont:'+id+'" >\
			<div class="panel panel-default">\
				<div class="panel-heading"><i class="fa fa-cube fa-fw"></i>'+data.name+'</div>\
				<div class="panel-body">\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_desc')+':</span> '+ data.description +'</p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_vers')+':</span> '+data.version+'</p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_provider')+':</span> '+data.provider+'</p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_source')+':</span> <a href="'+data.source+'" target="_blank">'+getHostnameURL(data.source)+'</a></p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_support')+':</span> <a href="'+data.support+'" target="_blank">'+getHostnameURL(data.support)+'</a></p>\
					<hr>\
					<button class="btn '+start_btn_class+'" id="pstart:'+id+'" onclick="requestPluginStart(\''+id+'\')"><i class="fa fa-play fa-fw"></i><span style="font-weight:bold;">'+start_btn_text+'</span></button>\
					<button class="btn btn-danger" id="pstop:'+id+'" onclick="requestPluginStop(\''+id+'\')" '+stop_btn_disabled+'><i class="fa fa-stop fa-fw"></i>'+$.i18n('plugins_stop')+'</button>\
					<button class="btn btn-primary" id="co:'+id+'"><i class="fa fa-cog fa-fw"></i>'+$.i18n('plugins_confit')+'</button>\
					<button class="btn '+autoupd_btn_class+'" id="au:'+id+'"><i class="fa fa-magic fa-fw"></i>'+$.i18n('plugins_autoupd')+'</button>\
					<button class="btn btn-default" id="del:'+id+'"><i class="fa fa-trash fa-fw"></i>'+$.i18n('plugins_delete')+'</button>\
				</div>\
			</div>\
		</div>');

		document.getElementById("au:"+id).onclick = PluginClick;
		document.getElementById("co:"+id).onclick = PluginClick;
		document.getElementById("del:"+id).onclick = PluginDelete;
	}

	function createPAWrap(id, data)
	{
		// return for module. plugins
		if(id.startsWith("module."))
			return;
		install_btn_disabled = isPluginInstalled(id) ? 'disabled="disabled"' : '' ;
		availCont.append('\
		<div class="col-lg-6" id="pavail_cont:'+id+'" >\
			<div class="panel panel-default">\
				<div class="panel-heading"><i class="fa fa-list fa-fw"></i>'+data.name+'</div>\
				<div class="panel-body">\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_desc')+':</span> '+ data.description +'</p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_vers')+':</span> '+data.version+'</p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_provider')+':</span> '+data.provider+'</p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_source')+':</span> <a href="'+data.source+'" target="_blank">'+getHostnameURL(data.source)+'</a></p>\
					<p><span style="font-weight:bold;">'+$.i18n('plugins_support')+':</span> <a href="'+data.support+'" target="_blank">'+getHostnameURL(data.support)+'</a></p>\
					<hr>\
					<button class="btn btn-primary" id="pinst:'+id+'" '+install_btn_disabled+'><i class="fa fa-download fa-fw"></i><span style="font-weight:bold;">'+$.i18n('plugins_install')+'</span></button>\
				</div>\
			</div>\
		</div>');

		document.getElementById("pinst:"+id).onclick = PluginInstall;
	}

	//var instSel = $("#p_instSel");
	var instCont = $("#p_installed");
	var availCont = $("#p_available");

	if(showOptHelp)
		createHintH("intro", $.i18n('plugins_intro'), "intro_plugins");

	// create plugins installed
	if(Object.keys(plugins_installed).length > 0)
	{
		for(key in plugins_installed)
		{
			createPIWrap(key,plugins_installed[key])
		}
	}
	else {
		instCont.append('<div class="col-md-12">'+$.i18n("plugins_noInstalled")+'</div>');
	}

	// create plugins available
	if(Object.keys(plugins_available).length > 0)
	{
		// clear list
		availCont.html('');
		for(key in plugins_available)
		{
			createPAWrap(key,plugins_available[key])
		}
	}
	else {
		availCont.html('No Plugins in repository, you need a working internet connection!');
	}
	// connect update repo btn
	document.getElementById("plugins_upd_repo").onclick = updateRepo;

	$(hyperion).off("cmd-plugin-start cmd-plugin-stop cmd-plugin-error cmd-plugin-remove cmd-plugin-install cmd-plugin-updateavail cmd-plugin-save").on("cmd-plugin-start cmd-plugin-stop cmd-plugin-error cmd-plugin-remove cmd-plugin-install cmd-plugin-updateavail cmd-plugin-save", function(e){
		//console.log("cmd-plugin-event",e);
		var resp = e.response.info;
		if(e.type == "cmd-plugin-start")
		{
			modifyBtn("pstart:"+resp.id, resp.success);
			elEnable("pstop:"+resp.id, resp.success);
		}
		else if(e.type == "cmd-plugin-stop")
		{
			modifyBtn("pstart:"+resp.id, false);
			elEnable("pstop:"+resp.id, false);
		}
		else if(e.type == "cmd-plugin-error")
		{
			showNotification("danger",$.i18n('plugins_exception',getPluginNameById(resp.id)));
			modifyBtn("pstart:"+resp.id, false);
			elEnable("pstop:"+resp.id, false);
		}
		else if(e.type == "cmd-plugin-remove")
		{
			if(resp.success)
			{
				showNotification("success",$.i18n('plugins_delsucc',getPluginNameById(resp.id)));
				removeEntry("pinst_cont:"+resp.id);
				delete plugins_installed[resp.id];
				elEnable("pinst:"+resp.id, true);
			}
			else
			{
				showNotification("danger",$.i18n('plugins_delerr',getPluginNameById(resp.id)));
			}
		}
		else if(e.type == "cmd-plugin-install")
		{
			if(resp.success)
			{
				showNotification("success",$.i18n('plugins_instsucc',getPluginNameById(resp.id)));
				// update entry
				plugins_installed[resp.id] = resp.data;
				removeEntry("pinst_cont:"+resp.id);
				createPIWrap(resp.id);
				elEnable("pinst:"+resp.id, false);

			}
			else
			{
				showNotification("danger",$.i18n('plugins_insterr',getPluginNameById(resp.id)));
			}
		}
		else if(e.type == "cmd-plugin-updateavail")
		{
			if(resp.success)
			{
				// update plugins available
				if(Object.keys(resp.data).length == 0)
				{
					showNotification("danger","Repository list received but was empty! This should never happen");
				}
				else
				{
					// transfer new list
					plugins_available = resp.data;
					showNotification("success",$.i18n('plugins_reposucc',getPluginNameById(resp.id)));
					// clear list
					availCont.html('');
					for(key in plugins_available)
					{
						createPAWrap(key,plugins_available[key])
					}
				}
			}
			else
			{
				showNotification("danger",$.i18n('plugins_repoerr',getPluginNameById(resp.id)));
			}
		}
		else if(e.type == "cmd-plugin-save")
		{
			if(resp.success)
			{
				showNotification("success",$.i18n('plugins_savesucc',getPluginNameById(resp.id)));
				setSettings(resp.id, resp.data);
				return;
			}
			showNotification("danger",$.i18n('plugins_saveerr',getPluginNameById(resp.id)));
		}

	});

});
