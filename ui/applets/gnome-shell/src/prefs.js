const GLib = imports.gi.GLib;
const Gtk = imports.gi.Gtk;

const ExtensionUtils = imports.misc.extensionUtils;
const Me = ExtensionUtils.getCurrentExtension();

function init()
{
    log(`initializing ${Me.metadata.name} Preferences`);
}


function buildPrefsWidget()
{
    let prefsWidget = new Gtk.Label({
        label: `${Me.metadata.name} version ${Me.metadata.version}`,
        visible: true
    });

    //GLib.timeout_add(GLib.G_PRIORITY_DEFAULT, 0, () => {
    //    let window = prefsWidget.get_toplevel();
    //    let headerBar = window.get_titlebar();
    //    headerbar.title = `${Me.metadata.name} Preferences`;
    //});

    return prefsWidget;
}
