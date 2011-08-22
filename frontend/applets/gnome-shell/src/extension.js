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
    methods: [ { name: 'SetEnabled',
                 inSignature: 'b',
                 outSignature: ''
               },
	       { name: 'GetMenuItems',
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

      	this._area = new St.DrawingArea({ style_class: "workrave", reactive: false});
	this._area.set_width(this.width=100);
        this._area.set_height(this.height=24);
        this._area.connect('repaint', Lang.bind(this, this._draw));
        this.actor.set_child(this._area);
        Main.panel._centerBox.add(this.actor, { y_fill: true });
 
	this._proxy = new IndicatorProxy(DBus.session, 'org.workrave.Workrave', '/org/workrave/Workrave/UI');
	this._timers_updated_id = this._proxy.connect("TimersUpdated", Lang.bind(this, this._onTimersUpdated));
	this._menu_updated_id = this._proxy.connect("MenuUpdated", Lang.bind(this, this._onMenuUpdated));

	this._timer_running = true;
	Mainloop.timeout_add(3000, Lang.bind(this, this._onTimer));

	this._proxy.GetMenuItemsRemote(Lang.bind(this, this._onGetMenuItemsReply));
    },
 
    _onTimer: function() {
	
	if (! this._alive)
	{
	    global.log('workrave-applet: now dead');
	    this._timerbox.set_enabled(false);
	    this._timer_running = false;
	    return false;
	}

	this._alive = false;
	return true;
    },

    _onDestroy: function() {},

    _draw: function(area) {
        let [width, height] = area.get_surface_size();
        let themeNode = area.get_theme_node();
        //let gradientHeight = themeNode.get_length('-gradient-height');
        //let startColor = themeNode.get_color('-gradient-start');
        let cr = area.get_context();
	
	this._timerbox.draw(cr);
    },

    _onTimersUpdated : function(result, microbreak, restbreak, daily) {

	if (! this._alive)
	{
	    this._alive = true;
	}

	if (! this._timer_running)
	{
	    global.log('workrave-applet: now alive');
	    this._timer_running = true;
	    Mainloop.timeout_add(3000, Lang.bind(this, this._onTimer));
	    this._proxy.GetMenuItemsRemote(Lang.bind(this, this._onGetMenuItemsReply));
	}

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

    _onGetMenuItemsReply : function(menuitems) {
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

    _updateMenu : function(menuitems) {
	this.menu.removeAll();

	let current_menu = this.menu;
	let indent = "";

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
};


function main(extensionMeta) {

    let userExtensionLocalePath = extensionMeta.path + '/locale';
    Gettext.bindtextdomain("workrave", userExtensionLocalePath);
    Gettext.textdomain("workrave");
 
    let _workravePanelButton = new _workraveButton();
}
