import Gio from "gi://Gio";
import GLib from "gi://GLib";
import * as Main from "resource:///org/gnome/shell/ui/main.js";
import { gettext as _ } from "resource:///org/gnome/shell/extensions/extension.js";

import * as Prelude from "./prelude_window.js";

const PreludesIface = `
<node>
  <interface name="org.workrave.Preludes">
    <method name="Init">
      <arg type="s" name="prelude_icon" direction="in"/>
      <arg type="s" name="prelude_sad_icon" direction="in"/>
      <arg type="s" name="warn_color" direction="in"/>
      <arg type="s" name="alert_color" direction="in"/>
    </method>
    <method name="Terminate"/>
    <method name="Start">
      <arg type="s" name="text" direction="in"/>
    </method>
    <method name="Stop"/>
    <method name="Refresh"/>
    <method name="SetProgress">
      <arg type="i" name="value" direction="in"/>
      <arg type="i" name="max_value" direction="in"/>
    </method>
    <method name="SetStage">
      <arg type="s" name="stage" direction="in"/>
    </method>
    <method name="SetProgressText">
      <arg type="s" name="text" direction="in"/>
    </method>
  </interface>
</node>
`;

export class PreludeManager {
  constructor() {
    this._areas = [];
    this._dbus = null;

    this._icon = null;
    this._sad_icon = null;
    this._warn_color = null;
    this._alert_color = null;

    this._initialized = false;
    this._started = false;
  }

  init() {
    if (this._dbus == null) {
      this._dbus = Gio.DBusExportedObject.wrapJSObject(PreludesIface, this);
      this._dbus.export(Gio.DBus.session, "/org/workrave/Workrave/Preludes");
    }
  }

  terminate() {
    if (this._dbus != null) {
      this._dbus.unexport();
      this._dbus = null;
    }
    this._disableAreas();
    this._removeAreas();
  }

  _updateAreas() {
    if (this.activeArea) {
      this.toggleDrawing();
    }
    this._removeAreas();

    this._monitors = Main.layoutManager.monitors;

    for (let i = 0; i < this._monitors.length; i++) {
      let monitor = this._monitors[i];
      let area = new Prelude.PreludeWindow(
        monitor,
        this._icon,
        this._sad_icon,
        this._warn_color,
        this._alert_color
      );

      this._areas.push(area);
    }
  }

  _enableAreas() {
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      Main.uiGroup.add_child(area);
    }
  }

  _disableAreas() {
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      Main.uiGroup.remove_child(area);
    }
  }

  _removeAreas() {
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      area.destroy();
    }
    this._areas = [];
  }

  Init(prelude_icon, prelude_sad_icon, warn_color, alert_color) {
    this._icon = prelude_icon;
    this._sad_icon = prelude_sad_icon;
    this._warn_color = warn_color;
    this._alert_color = alert_color;

    this._updateAreas();
    // this._monitorChangedHandler = Main.layoutManager.connect(
    //   "monitors-changed",
    //   this._updateAreas.bind(this)
    // );

    this._initialized = true;
  }

  Terminate() {
    this._removeAreas();
    this._initialized = false;
  }

  Start(text) {
    if (!this._initialized) {
      throw new Error(_("PreludeManager not initialized"));
    }
    if (this._started) {
      throw new Error(_("PreludeManager already started"));
    }
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      area.set_text(text);
    }
    this._enableAreas();
    this._started = true;
  }

  Stop() {
    if (!this._started) {
      throw new Error(_("PreludeManager not started"));
    }
    // if (this._monitorChangedHandler) {
    //   Main.layoutManager.disconnect(this._monitorChangedHandler);
    //   this._monitorChangedHandler = null;
    // }
    this._disableAreas();
    this._started = false;
  }

  Refresh() {
    if (!this._started) {
      throw new Error(_("PreludeManager not started"));
    }
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      area.refresh();
    }
  }

  SetStage(stage) {
    if (!this._initialized) {
      throw new Error(_("PreludeManager not initialized"));
    }
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      area.set_stage(stage);
    }
  }

  SetProgress(value, max_value) {
    if (!this._initialized) {
      throw new Error(_("PreludeManager not initialized"));
    }
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      area.set_progress(value, max_value);
    }
  }

  SetProgressText(text) {
    if (!this._initialized) {
      throw new Error(_("PreludeManager not initialized"));
    }
    for (let i = 0; i < this._areas.length; i++) {
      let area = this._areas[i];
      area.set_progress_text(text);
    }
  }
}
