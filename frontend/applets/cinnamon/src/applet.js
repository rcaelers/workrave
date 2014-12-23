// TODO: text color in timebar
// TODO: vertically center timebar
// TODO: after cinnamon restart, workrave internal applet is not removed.

const Applet = imports.ui.applet;
const St = imports.gi.St;
const Mainloop = imports.mainloop;
const Lang = imports.lang;
const PopupMenu = imports.ui.popupMenu;
const Gettext = imports.gettext;
const GLib = imports.gi.GLib;
const Gio = imports.gi.Gio;
const Workrave = imports.gi.Workrave;
 
const _ = Gettext.gettext;

let start = GLib.get_monotonic_time();
global.log('workrave-applet: start @ ' + start);

const IndicatorIface = '<node>\
    <interface name="org.workrave.AppletInterface"> \
    <method name="Embed"> \
        <arg type="b" name="enabled" direction="in" /> \
        <arg type="s" name="sender" direction="in" /> \
    </method> \
    <method name="Command"> \
        <arg type="i" name="command" direction="in" /> \
    </method> \
    <method name="GetMenu"> \
        <arg type="a(sii)" name="menuitems" direction="out" /> \
    </method> \
    <method name="GetTrayIconEnabled"> \
        <arg type="b" name="enabled" direction="out" /> \
    </method> \
    <signal name="TimersUpdated"> \
        <arg type="(siuuuuuu)" /> \
        <arg type="(siuuuuuu)" /> \
        <arg type="(siuuuuuu)" /> \
    </signal> \
    <signal name="MenuUpdated"> \
        <arg type="a(sii)" /> \
    </signal> \
    <signal name="TrayIconUpdated"> \
        <arg type="b" /> \
    </signal> \
    </interface> \
</node>';

let IndicatorProxy = Gio.DBusProxy.makeProxyWrapper(IndicatorIface);

function MyApplet(metadata, orientation, panel_height, instanceId) {
    this._init(metadata, orientation, panel_height, instanceId);
}

MyApplet.prototype = {
    __proto__: Applet.Applet.prototype,

    _init: function(metadata, orientation, panel_height) {
        Applet.Applet.prototype._init.call(this, orientation, panel_height);

        this.menuManager = new PopupMenu.PopupMenuManager(this);
        this.menu = new Applet.AppletPopupMenu(this, orientation);
        this.menuManager.addMenu(this.menu);
        
        this._timerbox = new Workrave.Timerbox();
        this._force_icon = false;
	this._height = panel_height;
	this._bus_name = 'org.workrave.CinnamonApplet';
	this._bus_id = 0;

      	this._area = new St.DrawingArea(); // { style_class: "workrave", reactive: false});
	this._area.set_width(this.width=24);
        this._area.set_height(this.height=this.panel_height);
        this._area.connect('repaint', Lang.bind(this, this._draw));

        this.actor.add_actor(this._area, { y_align: St.Align.MIDDLE, y_fill: false });
	this.actor.show();
	this.actor.connect('destroy', Lang.bind(this, this._onDestroy));

	this._proxy = new IndicatorProxy(Gio.DBus.session, 'org.workrave.Workrave', '/org/workrave/Workrave/UI');
	this._timers_updated_id = this._proxy.connectSignal("TimersUpdated", Lang.bind(this, this._onTimersUpdated));
	this._menu_updated_id = this._proxy.connectSignal("MenuUpdated", Lang.bind(this, this._onMenuUpdated));
	this._trayicon_updated_id = this._proxy.connectSignal("TrayIconUpdated", Lang.bind(this, this._onTrayIconUpdated));

	this._updateMenu(null);

        Gio.DBus.session.watch_name('org.workrave.Workrave',
                                    Gio.BusNameWatcherFlags.NONE, // no auto launch
                                    Lang.bind(this, this._onWorkraveAppeared),
                                    Lang.bind(this, this._onWorkraveVanished));
    },

    on_applet_removed_from_panel: function() {
        this._onDestroy();
    },

    on_applet_clicked: function(event) {
        this.menu.toggle();
    },
    
    _onDestroy: function() 
    {
    	this._proxy.EmbedRemote(false, 'CinnamonApplet');
	this._stop();
	this._destroy();
    },

    _destroy: function() {
	this._proxy.disconnectSignal(this._timers_updated_id);
	this._proxy.disconnectSignal(this._menu_updated_id);
	this._proxy.disconnectSignal(this._trayicon_updated_id);
	this._proxy = null;
        this.actor.destroy();
    },

    _start: function()
    {
	if (! this._alive)
	{
	    this._bus_id = Gio.DBus.session.own_name(this._bus_name, Gio.BusNameOwnerFlags.NONE, null, null);
	    this._proxy.GetMenuRemote(Lang.bind(this, this._onGetMenuReply));
	    this._proxy.GetTrayIconEnabledRemote(Lang.bind(this, this._onGetTrayIconEnabledReply));
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
	     Mainloop.source_remove(this._timeoutId);
	     Gio.DBus.session.unown_name(this._bus_id);
	     this._bus_id = 0;
	     this._timerbox.set_enabled(false);
             this._timerbox.set_force_icon(false);
	     this._area.queue_repaint();
	     this._alive = false;
	     this._updateMenu(null);
	     this._area.set_width(this.width=24);

	 }
     },

     _draw: function(area) {
	 let [width, height] = area.get_surface_size();
	 let cr = area.get_context();

         let bar_height = this._timerbox.get_height();
         this._area.set_height(this.height = bar_height);
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
            this._timerbox.set_force_icon(false);
	    this._area.queue_repaint();
	}
	this._update_count = 0;

	return this._alive;
    },

    _onWorkraveAppeared: function(owner) {
	this._start();
    },

    _onWorkraveVanished: function(oldOwner) {
	this._stop();
    },

    _onTimersUpdated : function(emitter, senderName, [microbreak, restbreak, daily]) {

	if (! this._alive)
	{
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

    _onGetMenuReply : function([menuitems], excp) {
	this._updateMenu(menuitems);
    },

    _onGetTrayIconEnabledReply : function([enabled], excp) {
	this._updateTrayIcon(enabled);
    },

    _onMenuUpdated : function(emitter, senderName, [menuitems]) {
	this._updateMenu(menuitems);
    },

    _onTrayIconUpdated : function(emitter, senderName, [enabled]) {
	this._updateTrayIcon(enabled);
    },

    _onCommandReply : function(menuitems) {
    },
    
    _onMenuCommand : function(item, event, dummy, command) {
	this._proxy.CommandRemote(command, Lang.bind(this, this._onCommandReply));
    },

    _onMenuOpenCommand: function(item, event) {
	this._proxy.GetMenuRemote(); // A dummy method call to re-activate the service
    },

    _functionExists: function(func) {
        return (typeof(func) == typeof(Function));
    },

    _updateTrayIcon : function(enabled) {
        this._force_icon = enabled;
        //Never force icon, cinnamon shows tray icon
        //this._timerbox.set_force_icon(this._force_icon);
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

                //global.log('workrave-applet: _updateMenu ' + text + ' ' + command + ' ' + flags);
		
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
                        
                        if (this._functionExists(popup.setShowDot))
                        {
			    popup.setShowDot(active);
                        }
                        else if (this._functionExists(popup.setOrnament))
                        {
                            popup.setOrnament(active ? PopupMenu.Ornament.DOT : PopupMenu.Ornament.NONE);
                        }
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

function main(metadata, orientation, panel_height, instanceId) {
    let myApplet = new MyApplet(metadata, orientation, panel_height, instanceId);
    return myApplet;
}
