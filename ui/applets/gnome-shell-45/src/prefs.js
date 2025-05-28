const GLib = imports.gi.GLib;
const Gtk = imports.gi.Gtk;

const ExtensionUtils = imports.misc.extensionUtils;
const Me = ExtensionUtils.getCurrentExtension();

function init() {
  log(`initializing ${Me.metadata.name} Preferences`);
}

function buildPrefsWidget() {
  let prefsWidget = new Gtk.Label({
    label: `${Me.metadata.name} version ${Me.metadata.version}`,
    visible: true,
  });

  return prefsWidget;
}
