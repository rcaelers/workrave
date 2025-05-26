import Clutter from "gi://Clutter";
import Gio from "gi://Gio";
import GLib from "gi://GLib";
import GObject from "gi://GObject";
import St from "gi://St";
import * as Main from "resource:///org/gnome/shell/ui/main.js";
import * as PanelMenu from "resource:///org/gnome/shell/ui/panelMenu.js";
import * as PopupMenu from "resource:///org/gnome/shell/ui/popupMenu.js";
import {
  Extension,
  gettext as _,
} from "resource:///org/gnome/shell/extensions/extension.js";

import Workrave from "gi://Workrave?version=2.0";
import * as Prelude from "./prelude.js";

let start = GLib.get_monotonic_time();
console.log("workrave-applet: start @ " + start);

const IndicatorIface =
  '<node>\
    <interface name="org.workrave.AppletInterface"> \
    <method name="Embed"> \
        <arg type="b" name="enabled" direction="in" /> \
        <arg type="s" name="sender" direction="in" /> \
    </method> \
    <method name="Command"> \
        <arg type="i" name="command" direction="in" /> \
    </method> \
    <method name="GetMenu"> \
        <arg type="a(sssuyy)" name="menuitems" direction="out" /> \
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
        <arg type="a(sssuyy)" /> \
    </signal> \
    <signal name="MenuItemUpdated"> \
        <arg type="(sssuyy)" /> \
    </signal> \
    <signal name="TrayIconUpdated"> \
        <arg type="b" /> \
    </signal> \
    </interface> \
</node>';

let IndicatorProxy = Gio.DBusProxy.makeProxyWrapper(IndicatorIface);

const CoreIface =
  '<node>\
  <interface name="org.workrave.CoreInterface"> \
    <method name="SetOperationMode"> \
      <arg type="s" name="mode" direction="in"> \
      </arg> \
    </method> \
    <method name="GetOperationMode"> \
      <arg type="s" name="mode" direction="out"> \
      </arg> \
    </method> \
    <method name="SetUsageMode"> \
      <arg type="s" name="mode" direction="in"> \
      </arg> \
    </method> \
    <method name="GetUsageMode"> \
      <arg type="s" name="mode" direction="out"> \
      </arg> \
    </method> \
    <method name="ReportActivity"> \
      <arg type="s" name="who" direction="in"> \
      </arg> \
      <arg type="b" name="act" direction="in"> \
      </arg> \
    </method> \
    <method name="IsTimerRunning"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
      <arg type="b" name="value" direction="out"> \
      </arg> \
    </method> \
    <method name="GetTimerIdle"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
      <arg type="i" name="value" direction="out"> \
      </arg> \
    </method> \
    <method name="GetTimerElapsed"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
      <arg type="i" name="value" direction="out"> \
      </arg> \
    </method> \
    <method name="GetTimerRemaining"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
      <arg type="i" name="value" direction="out"> \
      </arg> \
    </method> \
    <method name="GetTimerOverdue"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
      <arg type="i" name="value" direction="out"> \
      </arg> \
    </method> \
    <method name="GetTime"> \
      <arg type="i" name="value" direction="out"> \
      </arg> \
    </method> \
    <method name="GetBreakState"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
      <arg type="s" name="stage" direction="out"> \
      </arg> \
    </method> \
    <method name="IsActive"> \
      <arg type="b" name="value" direction="out"> \
      </arg> \
    </method> \
    <method name="PostponeBreak"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
    </method> \
    <method name="SkipBreak"> \
      <arg type="s" name="timer_id" direction="in"> \
      </arg> \
    </method> \
    <signal name="MicrobreakChanged"> \
      <arg type="s" name="progress"> \
      </arg> \
    </signal> \
    <signal name="RestbreakChanged"> \
      <arg type="s" name="progress"> \
      </arg> \
    </signal> \
    <signal name="DailylimitChanged"> \
      <arg type="s" name="progress"> \
      </arg> \
    </signal> \
    <signal name="OperationModeChanged"> \
      <arg type="s" name="mode"> \
      </arg> \
    </signal> \
    <signal name="UsageModeChanged"> \
      <arg type="s" name="mode"> \
      </arg> \
    </signal> \
    <signal name="BreakPostponed"> \
      <arg type="s" name="timer_id"> \
      </arg> \
    </signal> \
    <signal name="BreakSkipped"> \
      <arg type="s" name="timer_id"> \
      </arg> \
    </signal> \
  </interface> \
</node>';

const PreludesIface = `
<node>
  <interface name="org.workrave.Preludes">
    <method name="Init">
      <arg type="s" name="prelude_icon" direction="in"/>
      <arg type="s" name="prelude_sad_icon" direction="in"/>
      <arg type="s" name="warn_color" direction="in"/>
      <arg type="s" name="alert_color" direction="in"/>
    </method>
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

const MENU_ITEM_TYPE_SUBMENU_BEGIN = 1;
const MENU_ITEM_TYPE_SUBMENU_END = 2;
const MENU_ITEM_TYPE_RADIOGROUP_BEGIN = 3;
const MENU_ITEM_TYPE_RADIOGROUP_END = 4;
const MENU_ITEM_TYPE_ACTION = 5;
const MENU_ITEM_TYPE_CHECK = 6;
const MENU_ITEM_TYPE_RADIO = 7;
const MENU_ITEM_TYPE_SEPARATOR = 8;

const MENU_ITEM_FLAG_ACTIVE = 1;
const MENU_ITEM_FLAG_VISIBLE = 2;

let CoreProxy = Gio.DBusProxy.makeProxyWrapper(CoreIface);

const WorkraveButton = GObject.registerClass(
  class WorkraveButton extends PanelMenu.Button {
    _init() {
      super._init(0.0, null, false);

      this._timerbox = new Workrave.Timerbox();
      this._force_icon = false;
      this._height = 24;
      this._padding = 0;
      this._bus_name = "org.workrave.GnomeShellApplet";
      this._bus_id = 0;
      this._menu_entries = {};
      this._watchid = 0;
      this._alive = false;
      this._prelude_dbus = null;

      this._area = new St.DrawingArea({
        style_class: "workrave-area",
        reactive: true,
        y_align: Clutter.ActorAlign.CENTER,
      });
      this._area.set_width((this._width = 24));
      this._area.set_height((this._height = 24));
      this._area.connect("repaint", this._draw.bind(this));

      this._box = new St.Bin();
      if (typeof this._box.add_child === "function") {
        this._box.add_child(this._area);
      } else if (typeof this.add_actor === "function") {
        this._box.add_actor(this._area);
      }

      if (typeof this.actor.add_child === "function") {
        this.actor.add_child(this._box);
        this.actor.show();
      } else if (typeof this.add_actor === "function") {
        this.add_actor(this._box);
        this.show();
      } else {
        this.actor.add_actor(this._box, { y_expand: true });
        this.actor.show();
      }

      this.connect("destroy", this._onDestroy.bind(this));

      this._ui_proxy = new IndicatorProxy(
        Gio.DBus.session,
        "org.workrave.Workrave",
        "/org/workrave/Workrave/UI",
        this._connectUI.bind(this)
      );
      this._timers_updated_id = this._ui_proxy.connectSignal(
        "TimersUpdated",
        this._onTimersUpdated.bind(this)
      );
      this._menu_updated_id = this._ui_proxy.connectSignal(
        "MenuUpdated",
        this._onMenuUpdated.bind(this)
      );
      this._menu_item_updated_id = this._ui_proxy.connectSignal(
        "MenuItemUpdated",
        this._onMenuItemUpdated.bind(this)
      );
      this._trayicon_updated_id = this._ui_proxy.connectSignal(
        "TrayIconUpdated",
        this._onTrayIconUpdated.bind(this)
      );

      this._core_proxy = new CoreProxy(
        Gio.DBus.session,
        "org.workrave.Workrave",
        "/org/workrave/Workrave/Core",
        this._connectCore.bind(this)
      );
      this._operation_mode_changed_id = this._core_proxy.connectSignal(
        "OperationModeChanged",
        this._onOperationModeChanged.bind(this)
      );

      this.menu._setOpenedSubMenu = this._setOpenedSubmenu.bind(this);
      this._updateMenu(null);

      this.areas = [];
      this._updateAreas();
      this.monitorChangedHandler = Main.layoutManager.connect(
        "monitors-changed",
        this._updateAreas.bind(this)
      );
    }

    _updateAreas() {
      if (this.activeArea) {
        this.toggleDrawing();
      }
      this._removeAreas();

      this.monitors = Main.layoutManager.monitors;

      for (let i = 0; i < this.monitors.length; i++) {
        let monitor = this.monitors[i];
        let area = new Prelude.PreludeWindow(monitor);
        console.log(
          "workrave-applet: monitor " +
            i +
            " " +
            monitor.width +
            " " +
            monitor.height
        );

        //Main.layoutManager._backgroundGroup.insert_child_above(area, Main.layoutManager._bgManagers[i].backgroundActor);
        this.areas.push(area);
      }
    }

    _enableAreas() {
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        Main.uiGroup.add_child(area);
      }
    }

    _disableAreas() {
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        Main.uiGroup.remove_child(area);
      }
    }

    _removeAreas() {
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        area.destroy();
      }
      this.areas = [];
    }

    _setOpenedSubmenu(submenu) {
      this._openedSubMenu = submenu;
    }

    _connectUI() {
      try {
        this._watchid = Gio.DBus.session.watch_name(
          "org.workrave.Workrave",
          Gio.BusNameWatcherFlags.NONE, // no auto launch
          this._onWorkraveAppeared.bind(this),
          this._onWorkraveVanished.bind(this)
        );
        return false;
      } catch (err) {
        console.log("workrave-applet: failed to connect to UI (" + err + ")");
        return true;
      }
    }

    _connectCore() {}

    _onDestroy() {
      console.log("workrave-applet: onDestroy");
      if (this._ui_proxy != null) {
        this._ui_proxy.EmbedRemote(false, this._bus_name);
      }
      this._stop();
      this._destroy();
    }

    _destroy() {
      console.log("workrave-applet: destroy");
      if (this.monitorChangedHandler) {
        Main.layoutManager.disconnect(this.monitorChangedHandler);
        this.monitorChangedHandler = null;
      }

      if (this._watchid > 0) {
        Gio.DBus.session.unwatch_name(this._watchid);
        this._watchid = 0;
      }
      if (this._ui_proxy != null) {
        this._ui_proxy.disconnectSignal(this._timers_updated_id);
        this._ui_proxy.disconnectSignal(this._menu_updated_id);
        this._ui_proxy.disconnectSignal(this._menu_item_updated_id);
        this._ui_proxy.disconnectSignal(this._trayicon_updated_id);
        this._ui_proxy = null;
      }
      if (this._core_proxy != null) {
        this._core_proxy.disconnectSignal(this._operation_mode_changed_id);
        this._core_proxy = null;
      }
    }

    _start() {
      console.log("workrave-applet: starting (alive = " + this._alive + ")");
      if (!this._alive) {
        this._bus_id = Gio.DBus.session.own_name(
          this._bus_name,
          Gio.BusNameOwnerFlags.NONE,
          null,
          null
        );
        this._ui_proxy.GetMenuRemote(this._onGetMenuReply.bind(this));
        this._ui_proxy.GetTrayIconEnabledRemote(
          this._onGetTrayIconEnabledReply.bind(this)
        );
        this._ui_proxy.EmbedRemote(true, this._bus_name);
        this._core_proxy.GetOperationModeRemote(
          this._onGetOperationModeReply.bind(this)
        );
        this._prelude_dbus = this._exportPreludeWindowDBus(this);

        this._timeoutId = GLib.timeout_add(
          GLib.PRIORITY_DEFAULT,
          5000,
          this._onTimer.bind(this)
        );
        this._alive = true;
        this._update_count = 0;
      }
    }

    _stop_dbus() {
      console.log(
        "workrave-applet: stopping dbus (alive = " + this._alive + ")"
      );
      if (this._alive) {
        GLib.source_remove(this._timeoutId);
        Gio.DBus.session.unown_name(this._bus_id);
        this._timeoutId = 0;
        this._bus_id = 0;
      }
      if (this._prelude_dbus != null) {
        console.log("workrave-applet: unexporting Preludes DBus interface");
        this._prelude_dbus.flush();
        this._prelude_dbus.unexport();
        delete this._prelude_dbus;
      }
    }

    _stop() {
      console.log("workrave-applet: stopping (alive = " + this._alive + ")");
      if (this._alive) {
        this._stop_dbus();
        this._timerbox.set_enabled(false);
        this._timerbox.set_force_icon(false);
        this._alive = false;
        this._updateMenu(null);
        this._area.queue_repaint();
        this._area.set_width((this._width = 24));
      }
    }

    _draw(area) {
      let cr = area.get_context();
      this._timerbox.draw(cr);
    }

    _exportPreludeWindowDBus(applet) {
      const impl = {
        Init(prelude_icon, prelude_sad_icon, warn_color, alert_color) {
          applet.init_prelude(
            prelude_icon,
            prelude_sad_icon,
            warn_color,
            alert_color
          );
        },
        Start(text) {
          applet.start_prelude(text);
        },
        Stop() {
          applet.stop_prelude();
        },
        Refresh() {
          applet.refresh_prelude();
        },
        SetStage(stage) {
          applet.set_prelude_stage(stage);
        },
        SetProgress(value, max_value) {
          applet.set_prelude_progress(value, max_value);
        },
        SetProgressText(text) {
          applet.set_prelude_progress_text(text);
        },
      };
      console.log("workrave-applet: exporting Preludes DBus interface");
      let exported = Gio.DBusExportedObject.wrapJSObject(PreludesIface, impl);
      exported.export(Gio.DBus.session, "/org/workrave/Workrave/Preludes");
      return exported;
    }

    init_prelude(prelude_icon, prelude_sad_icon, warn_color, alert_color) {
      console.log(
        "workrave-applet: init_prelude " +
          prelude_icon +
          " " +
          prelude_sad_icon +
          " " +
          warn_color +
          " " +
          alert_color
      );
      // TODO:
    }

    start_prelude(text) {
      console.log("workrave-applet: start_prelude");
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        area.set_text(text);
      }
      this._enableAreas();
    }

    stop_prelude() {
      console.log("workrave-applet: stop_prelude");
      this._disableAreas();
    }

    refresh_prelude() {
      console.log("workrave-applet: refresh_prelude");
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        area.refresh();
      }
    }

    set_prelude_stage(stage) {
      console.log("workrave-applet: set_stage " + stage);
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        area.set_stage(stage);
      }
    }

    set_prelude_progress(value, max_value) {
      console.log("workrave-applet: set_progress " + value + " / " + max_value);
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        area.set_progress(value, max_value);
      }
    }

    set_prelude_progress_text(text) {
      console.log("workrave-applet: set_progress_text " + text);
      for (let i = 0; i < this.areas.length; i++) {
        let area = this.areas[i];
        area.set_progress_text(text);
      }
    }

    _onTimer() {
      if (!this._alive) {
        console.log("workrave-applet: not alive in timer");
        return false;
      }

      if (this._update_count == 0) {
        console.log("workrave-applet: timeout (not updated)");
        this._timerbox.set_enabled(false);
        this._area.queue_repaint();
      }
      this._update_count = 0;

      return this._alive;
    }

    _onWorkraveAppeared(owner) {
      console.log("workrave-applet: workrave appeared");
      this._start();
    }

    _onWorkraveVanished(oldOwner) {
      console.log("workrave-applet: workrave disappeared");
      this._stop();
    }

    _onTimersUpdated(emitter, senderName, [microbreak, restbreak, daily]) {
      if (!this._alive) {
        console.log("workrave-applet: not alive, but timers got updated");
        this._start();
      }

      this._update_count++;

      this._timerbox.set_slot(0, microbreak[1]);
      this._timerbox.set_slot(1, restbreak[1]);
      this._timerbox.set_slot(2, daily[1]);

      var timebar = this._timerbox.get_time_bar(0);
      if (timebar != null) {
        this._timerbox.set_enabled(true);
        timebar.set_progress(microbreak[6], microbreak[7], microbreak[5]);
        timebar.set_secondary_progress(
          microbreak[3],
          microbreak[4],
          microbreak[2]
        );
        timebar.set_text(microbreak[0]);
      }

      timebar = this._timerbox.get_time_bar(1);
      if (timebar != null) {
        this._timerbox.set_enabled(true);
        timebar.set_progress(restbreak[6], restbreak[7], restbreak[5]);
        timebar.set_secondary_progress(
          restbreak[3],
          restbreak[4],
          restbreak[2]
        );
        timebar.set_text(restbreak[0]);
      }

      timebar = this._timerbox.get_time_bar(2);
      if (timebar != null) {
        this._timerbox.set_enabled(true);
        timebar.set_progress(daily[6], daily[7], daily[5]);
        timebar.set_secondary_progress(daily[3], daily[4], daily[2]);
        timebar.set_text(daily[0]);
      }

      let timerbox_width = this._timerbox.get_width();
      let timerbox_height = this._timerbox.get_height();

      this._area.set_width((this._width = timerbox_width));
      this._area.queue_repaint();
    }

    _onGetMenuReply([menuitems], excp) {
      this._updateMenu(menuitems);
    }

    _onGetTrayIconEnabledReply([enabled], excp) {
      this._updateTrayIcon(enabled);
    }

    _onGetOperationModeReply([mode], excp) {
      this._timerbox.set_operation_mode(mode);
    }

    _onMenuUpdated(emitter, senderName, [menuitems]) {
      this._updateMenu(menuitems);
    }

    _onMenuItemUpdated(emitter, senderName, [menuitem]) {
      this._updateItemMenu(menuitem);
    }

    _onTrayIconUpdated(emitter, senderName, [enabled]) {
      this._updateTrayIcon(enabled);
    }

    _onOperationModeChanged(emitter, senderName, [mode]) {
      this._timerbox.set_operation_mode(mode);
    }

    _onCommandReply(menuitems) {}

    _onMenuCommand(item, event, command) {
      this._ui_proxy.CommandRemote(command, this._onCommandReply.bind(this));
    }

    _onMenuOpenCommand(item, event) {
      this._ui_proxy.GetMenuRemote(); // A dummy method call to re-activate the service
    }

    _updateTrayIcon(enabled) {
      this._force_icon = enabled;
      this._timerbox.set_force_icon(this._force_icon);
    }

    _updateMenu(menuitems) {
      this.menu.removeAll();
      this._menu_entries = {};

      let current_menu = this.menu;
      let indent = "";

      if (menuitems == null || menuitems.length == 0) {
        let popup = new PopupMenu.PopupMenuItem(_("Open Workrave"));
        popup.connect("activate", this._onMenuOpenCommand.bind(this));
        current_menu.addMenuItem(popup);
      } else {
        let submenus = [];
        for (var item in menuitems) {
          let text = indent + menuitems[item][0];
          let dynamic_text = indent + menuitems[item][1];
          let action = menuitems[item][2];
          let id = menuitems[item][3];
          let type = menuitems[item][4];
          let flags = menuitems[item][5];

          let active = (flags & MENU_ITEM_FLAG_ACTIVE) != 0;
          let visible = (flags & MENU_ITEM_FLAG_VISIBLE) != 0;
          let popup;

          dynamic_text = dynamic_text.replace("_", "");

          if (type == MENU_ITEM_TYPE_SUBMENU_BEGIN) {
            let popup = new PopupMenu.PopupSubMenuMenuItem(dynamic_text);
            submenus.push(current_menu);
            current_menu.addMenuItem(popup);
            current_menu = popup.menu;
            indent = " ".repeat(4 * submenus.length); // Look at CSS??
          } else if (type == MENU_ITEM_TYPE_SUBMENU_END) {
            current_menu = submenus.pop();
            indent = " ".repeat(4 * submenus.length); // Look at CSS??
          } else if (type == MENU_ITEM_TYPE_SEPARATOR) {
            popup = new PopupMenu.PopupSeparatorMenuItem(dynamic_text);
          } else if (type == MENU_ITEM_TYPE_CHECK) {
            popup = new PopupMenu.PopupSwitchMenuItem(dynamic_text, active);
          } else if (type == MENU_ITEM_TYPE_RADIO) {
            popup = new PopupMenu.PopupMenuItem(dynamic_text);

            // Gnome 3.6 & 3.8
            if (typeof popup.setShowDot === "function") {
              popup.setShowDot(active);
            }

            // Gnome 3.10 & newer
            else if (typeof popup.setOrnament === "function") {
              popup.setOrnament(
                active ? PopupMenu.Ornament.DOT : PopupMenu.Ornament.NONE
              );
            }
          } else if (type == MENU_ITEM_TYPE_ACTION) {
            popup = new PopupMenu.PopupMenuItem(dynamic_text);
            popup.setOrnament(
              active ? PopupMenu.Ornament.DOT : PopupMenu.Ornament.NONE
            );
          }

          if (popup) {
            popup.setSensitive(visible);
            popup.connect("activate", (item, event) =>
              this._onMenuCommand(item, event, id)
            );
            current_menu.addMenuItem(popup);
            this._menu_entries[action] = popup;
          }
        }
      }
    }

    _updateItemMenu(menuitem) {
      let id = menuitem[2];
      let type = menuitem[4];
      let flags = menuitem[5];

      let active = (flags & MENU_ITEM_FLAG_ACTIVE) != 0;
      let visible = (flags & MENU_ITEM_FLAG_VISIBLE) != 0;
      let popup = this._menu_entries[id];

      popup.setSensitive(visible);

      if (type == MENU_ITEM_TYPE_CHECK) {
        popup.setToggleState(active);
      } else if (type == MENU_ITEM_TYPE_RADIO) {
        // Gnome 3.6 & 3.8
        if (typeof popup.setShowDot === "function") {
          popup.setShowDot(active);
        }

        // Gnome 3.10 & newer
        else if (typeof popup.setOrnament === "function") {
          popup.setOrnament(
            active ? PopupMenu.Ornament.DOT : PopupMenu.Ornament.NONE
          );
        }
      }
    }
  }
);

let workravePanelButton;

export default class WorkraveExtension extends Extension {
  enable() {
    console.log("workrave-applet: enabling applet");
    workravePanelButton = new WorkraveButton();
    Main.panel.addToStatusArea("workrave-applet", workravePanelButton);
  }

  disable() {
    console.log("workrave-applet: disabling applet");
    workravePanelButton.destroy();
    workravePanelButton = null;
  }
}
