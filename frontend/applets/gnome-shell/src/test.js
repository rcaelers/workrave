const Mainloop = imports.mainloop;
const Lang = imports.lang;
const Gettext = imports.gettext;
const GLib = imports.gi.GLib;
const Gtk = imports.gi.Gtk;
const Gdk = imports.gi.Gdk;
const Cairo = imports.cairo;
const Clutter = imports.gi.Clutter;
const DBus = imports.dbus;
const Workrave = imports.gi.Workrave;
 
const _ = Gettext.gettext;

let start = GLib.get_monotonic_time();
//global.log('system-monitor-applet: start @ ' + start);

let alive = false;

function onIndicatorUpdate (result, microbreak, restbreak, daily) {

    alive = true;

    timerbox.set_slot(0, microbreak[1]);
    timerbox.set_slot(1, restbreak[1]);
    timerbox.set_slot(2, daily[1]);
    
    var timebar = timerbox.get_time_bar(0);
    if (timebar != null)
    {
	timerbox.set_enabled(true);
	timebar.set_progress(microbreak[6], microbreak[7], microbreak[5]);
	timebar.set_secondary_progress(microbreak[3], microbreak[4], microbreak[2]);
	timebar.set_text(microbreak[0]);
    }

    timebar = timerbox.get_time_bar(1);
    if (timebar != null)
    {
	timerbox.set_enabled(true);
	timebar.set_progress(restbreak[6], restbreak[7], restbreak[5]);
	timebar.set_secondary_progress(restbreak[3], restbreak[4], restbreak[2]);
	timebar.set_text(restbreak[0]);
    }

    timebar = timerbox.get_time_bar(2);
    if (timebar != null)
    {
	timerbox.set_enabled(true);
	timebar.set_progress(daily[6], daily[7], daily[5]);
	timebar.set_secondary_progress(daily[3], daily[4], daily[2]);
	timebar.set_text(daily[0]);
    }

    // timerbox.draw(timerbox, );

}

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

    Gtk.init(0, null);

    let timerbox = new Workrave.Timerbox();

    let IndicatorProxy = DBus.makeProxyClass(IndicatorIface);
    let proxy = new IndicatorProxy(DBus.session, 'org.workrave.Workrave', '/org/workrave/Workrave/UI');

    let id = proxy.connect("Update", onIndicatorUpdate);

    Mainloop.run('');

