const St = imports.gi.St;
const Mainloop = imports.mainloop;
const Main = imports.ui.main;
const Shell = imports.gi.Shell;
const Lang = imports.lang;
const PopupMenu = imports.ui.popupMenu;
const PanelMenu = imports.ui.panelMenu;
const Gettext = imports.gettext;
const MessageTray = imports.ui.messageTray;
const Gtk = imports.gi.Gtk;
const Gdk = imports.gi.Gdk;
const GLib = imports.gi.GLib;
const Cairo = imports.cairo;
const Clutter = imports.gi.Clutter;
const Util = imports.misc.util;
const DBus = imports.dbus;
const Workrave = imports.gi.Workrave;
 
const _ = Gettext.gettext;

let start = GLib.get_monotonic_time();
global.log('workrave-applet: start @ ' + start);

const IndicatorIface = {
    name: 'org.workrave.AppletInterface',
    methods: [ { name: 'Embed',
                 inSignature: 'bs',
                 outSignature: ''
               },
	       { name: 'GetMenu',
                 inSignature: '',
                 outSignature: 'a(siii)'
               },
	       { name: 'Command',
                 inSignature: 'i',
                 outSignature: ''
               }
             ],

    signals: [ {  name: 'TimersUpdated',
                  inSignature: '(siuuuuuu)(siuuuuuu)(siuuuuuu)'
               },
	       {  name: 'MenuUpdated',
                  inSignature: 'a(siii)'
               }
             ],
   properties: []
};

let IndicatorProxy = DBus.makeProxyClass(IndicatorIface);

function _workraveButton() {
    this._init();
}
 
_workraveButton.prototype = {
    __proto__: PanelMenu.Button.prototype,
 
    _init: function() {
        PanelMenu.Button.prototype._init.call(this, 0.0);
  
        this._timerbox = new Workrave.Timerbox();
	this._height = 24;
	this._bus_name = 'org.workrave.GnomeShellApplet';
	this._bus_id = 0;

      	this._area = new St.DrawingArea({ style_class: "workrave", reactive: false});
	this._area.set_width(this.width=24);
        this._area.set_height(this.height=24);
        this._area.connect('repaint', Lang.bind(this, this._draw));
        this.actor.add_actor(this._area);
	this.actor.show();

	this.actor.connect('destroy', Lang.bind(this, this._onDestroy));

	this._proxy = new IndicatorProxy(DBus.session, 'org.workrave.Workrave', '/org/workrave/Workrave/UI');
	this._timers_updated_id = this._proxy.connect("TimersUpdated", Lang.bind(this, this._onTimersUpdated));
	this._menu_updated_id = this._proxy.connect("MenuUpdated", Lang.bind(this, this._onMenuUpdated));

	this._updateMenu(null);

        DBus.session.watch_name('org.workrave.Workrave',
                                false, // no auto launch
                                Lang.bind(this, this._onWorkraveAppeared),
                                Lang.bind(this, this._onWorkraveVanished));
        DBus.session.start_service('org.workrave.Workrave');
    },
 
    _onDestroy: function() 
    {
	global.log('workrave-applet: onDestroy');
    	this._proxy.EmbedRemote(false, 'GnomeShellApplet');
	this._stop();
	this._destroy();
    },

    _destroy: function() {
	global.log('workrave-applet: _destroy');
	this._proxy.disconnect(this._timers_updated_id);
	this._proxy.disconnect(this._menu_updated_id);
	this._proxy = null;

        this.actor.destroy();
    },

    _start: function()
    {
	global.log('workrave-applet: starting');
	if (! this._alive)
	{
	    this._bus_id = DBus.session.acquire_name(this._bus_name, 0, null, null);
	    this._proxy.GetMenuRemote(Lang.bind(this, this._onGetMenuReply));
    	    this._proxy.EmbedRemote(true, this._bus_name);
	    this._timeoutId = Mainloop.timeout_add(5000, Lang.bind(this, this._onTimer));
	    this._alive = true;
	    this._update_count = 0;
	}
    },

    _stop: function()
    {
	 if (this._alive)
	 {
	     global.log('workrave-applet: stopping');
	     Mainloop.source_remove(this._timeoutId);
	     DBus.session.release_name_by_id(this._bus_id);
	     this._bus_id = 0;
	     this._timerbox.set_enabled(false);
	     this._area.queue_repaint();
	     this._alive = false;
	     this._updateMenu(null);
	     this._area.set_width(this.width=24);

	 }
     },

     _draw: function(area) {
	 let [width, height] = area.get_surface_size();
	 let cr = area.get_context();
	 this._timerbox.draw(cr);
    },

    _onTimer: function() {
	if (! this._alive)
	{
	    return false;
	}

	if (this._update_count == 0)
	{
	    this._timerbox.set_enabled(false);
	    this._area.queue_repaint();
	}
	this._update_count = 0;

	return this._alive;
    },

    _onWorkraveAppeared: function(owner) {
	global.log('workrave-applet: appeared');
	this._start();
    },

    _onWorkraveVanished: function(oldOwner) {
	global.log('workrave-applet: vanished');
	this._stop();
    },

    _onTimersUpdated : function(result, microbreak, restbreak, daily) {

	if (! this._alive)
	{
	    global.log('workrave-applet: now alive');
	    this._start();
	}

	this._update_count++;

	this._timerbox.set_slot(0, microbreak[1]);
	this._timerbox.set_slot(1, restbreak[1]);
	this._timerbox.set_slot(2, daily[1]);

	var timebar = this._timerbox.get_time_bar(0);
	if (timebar != null)
	{
	    this._timerbox.set_enabled(true);
	    timebar.set_progress(microbreak[6], microbreak[7], microbreak[5]);
	    timebar.set_secondary_progress(microbreak[3], microbreak[4], microbreak[2]);
	    timebar.set_text(microbreak[0]);
	}
	
	timebar = this._timerbox.get_time_bar(1);
	if (timebar != null)
	{
	    this._timerbox.set_enabled(true);
	    timebar.set_progress(restbreak[6], restbreak[7], restbreak[5]);
	    timebar.set_secondary_progress(restbreak[3], restbreak[4], restbreak[2]);
	    timebar.set_text(restbreak[0]);
	}
	
	timebar = this._timerbox.get_time_bar(2);
	if (timebar != null)
	{
	    this._timerbox.set_enabled(true);
	    timebar.set_progress(daily[6], daily[7], daily[5]);
	    timebar.set_secondary_progress(daily[3], daily[4], daily[2]);
	    timebar.set_text(daily[0]);
	}
	
	let width = this._timerbox.get_width();
	this._area.set_width(this.width=width);

	this._area.queue_repaint();
    },

    _onGetMenuReply : function(menuitems) {
	this._updateMenu(menuitems);
    },

    _onMenuUpdated : function(result, menuitems) {
	this._updateMenu(menuitems);
    },

    _onCommandReply : function(menuitems) {
    },
    
    _onMenuCommand: function(item, event, command) {
	this._proxy.CommandRemote(command, Lang.bind(this, this._onCommandReply));
    },

    _onMenuOpenCommand: function(item, event) {
	global.log('workrave-applet: open');
        DBus.session.start_service('org.workrave.Workrave');
    },

    _updateMenu : function(menuitems) {
	this.menu.removeAll();

	let current_menu = this.menu;
	let indent = "";

	if (menuitems == null || menuitems.length == 0)
	{
	    let popup = new PopupMenu.PopupMenuItem(_("Open"));
	    popup.connect('activate', Lang.bind(this, this._onMenuOpenCommand));
	    current_menu.addMenuItem(popup);
	}
	else
	{
	    for (item in menuitems)
	    {
		let text = indent + menuitems[item][0];
		let command = menuitems[item][1];
		let flags = menuitems[item][2];
		
		if ((flags & 1) != 0)
		{
		    let popup = new PopupMenu.PopupSubMenuMenuItem(text);
		    this.menu.addMenuItem(popup);
		    current_menu = popup.menu;
		    indent = "   "; // Look at CSS??
		}
		else if ((flags & 2) != 0)
		{
		    current_menu = this.menu;
		    indent = "";
		}
		else
		{
		    let active = ((flags & 16) != 0);
		    let popup;
		    
		    if (text == "")
		    {
			popup = new PopupSub.PopupSeparatorMenuItem();
		    }
		    else if ((flags & 4) != 0)
		    {
			popup = new PopupMenu.PopupSwitchMenuItem(text);
			popup.setToggleState(active);
		    }
		    else if ((flags & 8) != 0)
		    {
			popup = new PopupMenu.PopupMenuItem(text);
			popup.setShowDot(active);
		    }
		    else 
		    {
			popup = new PopupMenu.PopupMenuItem(text);
		    }
		    
		    popup.connect('activate', Lang.bind(this, this._onMenuCommand, command));
		    current_menu.addMenuItem(popup);
		}
	    }
	}
    }
};

let workravePanelButton;
let workraveUserExtensionLocalePath;

function init(extensionMeta) {
    /* do nothing */
    workraveUserExtensionLocalePath = extensionMeta.path + '/locale';
}

function disable() {
    global.log('workrave-applet: disable');
    workravePanelButton.destroy();
    workravePanelButton = null;
}

function enable() {
    global.log('workrave-applet: enable');
    Gettext.bindtextdomain("workrave", workraveUserExtensionLocalePath);
    Gettext.textdomain("workrave");
 
    workravePanelButton = new _workraveButton();
    Main.panel.addToStatusArea('workrave-applet', workravePanelButton);
}
