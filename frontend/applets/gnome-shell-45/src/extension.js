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
      this._watchid = 0;
      this._alive = false;

      this._area = new St.DrawingArea({
        style_class: "workrave-area",
        reactive: true,
        y_align: Clutter.ActorAlign.CENTER,
      });
      this._area.set_width((this._width = 24));
      this._area.set_height((this._height = 24));
      this._area.connect("repaint", this._draw.bind(this));

      this._box = new St.Bin();
      this._box.add_actor(this._area);

      if (typeof this.add_actor === "function") {
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
      if (this._watchid > 0) {
        Gio.DBus.session.unwatch_name(this._watchid);
        this._watchid = 0;
      }
      if (this._ui_proxy != null) {
        this._ui_proxy.disconnectSignal(this._timers_updated_id);
        this._ui_proxy.disconnectSignal(this._menu_updated_id);
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

      let current_menu = this.menu;
      let indent = "";

      if (menuitems == null || menuitems.length == 0) {
        let popup = new PopupMenu.PopupMenuItem(_("Open Workrave"));
        popup.connect("activate", this._onMenuOpenCommand.bind(this));
        current_menu.addMenuItem(popup);
      } else {
        for (var item in menuitems) {
          let text = indent + menuitems[item][0];
          let command = menuitems[item][1];
          let flags = menuitems[item][2];

          if ((flags & 1) != 0) {
            let popup = new PopupMenu.PopupSubMenuMenuItem(text);
            this.menu.addMenuItem(popup);
            current_menu = popup.menu;
            indent = "   "; // Look at CSS??
          } else if ((flags & 2) != 0) {
            current_menu = this.menu;
            indent = "";
          } else {
            let active = (flags & 16) != 0;
            let popup;

            if (text == "") {
              popup = new PopupSub.PopupSeparatorMenuItem();
            } else if ((flags & 4) != 0) {
              popup = new PopupMenu.PopupSwitchMenuItem(text, active);
            } else if ((flags & 8) != 0) {
              popup = new PopupMenu.PopupMenuItem(text);

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
            } else {
              popup = new PopupMenu.PopupMenuItem(text);
            }

            popup.connect("activate", (item, event) => this._onMenuCommand(item, event, command));

            current_menu.addMenuItem(popup);
          }
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
