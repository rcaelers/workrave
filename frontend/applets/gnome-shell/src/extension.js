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
               }
             ],
    signals: [ {  name: 'Update',
                  inSignature: '(siuuuuuu)(siuuuuuu)(siuuuuuu)'
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
 
        this._workraveMenu = new PopupMenu.PopupMenuItem(_('Workrave MenuItem'));
        this.menu.addMenuItem(this._workraveMenu);
        
	this._proxy = new IndicatorProxy(DBus.session, 'org.workrave.Workrave', '/org/workrave/Workrave/UI');
	this._id = this._proxy.connect("Update", Lang.bind(this, this._onIndicatorUpdate));

	this._timer_running = true;
	Mainloop.timeout_add(3000, Lang.bind(this, this._onTimer));

    	global.log('workrave-applet: init ');
    },
 
    _onTimer: function() {
	
	if (! this._alive)
	{
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

    _onIndicatorUpdate : function(result, microbreak, restbreak, daily) {
	this._alive = true;

	global.log('workrave-applet: update ');

	if (! this._timer_running)
	{
	    Mainloop.timeout_add(3000, Lang.bind(this, this._onTimer));
	}

	this._timerbox.set_slot(0, microbreak[1]);
	this._timerbox.set_slot(1, restbreak[1]);
	this._timerbox.set_slot(2, daily[1]);

	global.log('workrave-applet: ' + microbreak[0]);

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
    }
};


function main(extensionMeta) {

    let userExtensionLocalePath = extensionMeta.path + '/locale';
    Gettext.bindtextdomain("workrave", userExtensionLocalePath);
    Gettext.textdomain("workrave");
 
    let _workravePanelButton = new _workraveButton();
}
