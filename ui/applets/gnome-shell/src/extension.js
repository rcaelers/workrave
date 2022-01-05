const St = imports.gi.St;
const Mainloop = imports.mainloop;
const Main = imports.ui.main;
const Lang = imports.lang;
const PopupMenu = imports.ui.popupMenu;
const PanelMenu = imports.ui.panelMenu;
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

const CoreIface = '<node>\
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

const WorkraveButton = new Lang.Class({
    Name: 'WorkraveButton',
    Extends: PanelMenu.Button,

    _init: function() {
        PanelMenu.Button.prototype._init.call(this, 0.0);

        this._timerbox = new Workrave.Timerbox();
        this._force_icon = false;
        this._height = 24;
        this._padding = 0;
        this._bus_name = 'org.workrave.GnomeShellApplet';
        this._bus_id = 0;
        this._menu_entries = {};
        this._watchid = 0;

        this._area = new St.DrawingArea({ style_class: 'workrave-area', reactive: true } );
        this._area.set_width(this._width=24);
        this._area.set_height(this._height=24);
        this._area.connect('repaint', Lang.bind(this, this._draw));

        this._box = new St.Bin();
        this._box.add_actor(this._area);

        if (typeof this.add_actor === "function")
        {
            this.add_actor(this._box);
            this.show();
        }
        else
        {
            this.actor.add_actor(this._box);
            this.actor.show();
        }

        this.connect('destroy', Lang.bind(this, this._onDestroy));

        this._ui_proxy = new IndicatorProxy(Gio.DBus.session, 'org.workrave.Workrave', '/org/workrave/Workrave/UI', Lang.bind(this, this._connectUI));
        this._timers_updated_id = this._ui_proxy.connectSignal("TimersUpdated", Lang.bind(this, this._onTimersUpdated));
        this._menu_updated_id = this._ui_proxy.connectSignal("MenuUpdated", Lang.bind(this, this._onMenuUpdated));
        this._menu_item_updated_id = this._ui_proxy.connectSignal("MenuItemUpdated", Lang.bind(this, this._onMenuItemUpdated));
        this._trayicon_updated_id = this._ui_proxy.connectSignal("TrayIconUpdated", Lang.bind(this, this._onTrayIconUpdated));

        this._core_proxy = new CoreProxy(Gio.DBus.session, 'org.workrave.Workrave', '/org/workrave/Workrave/Core', Lang.bind(this, this._connectCore));
        this._operation_mode_changed_id = this._core_proxy.connectSignal("OperationModeChanged", Lang.bind(this, this._onOperationModeChanged));

        this.menu._setOpenedSubMenu = Lang.bind(
            this,
            function (submenu) {
            this._openedSubMenu = submenu;
            }
        );

        this._updateMenu(null);
    },

    _connectUI: function() {
        try
        {
            this._watchid = Gio.DBus.session.watch_name('org.workrave.Workrave',
                                                        Gio.BusNameWatcherFlags.NONE, // no auto launch
                                                        Lang.bind(this, this._onWorkraveAppeared),
                                                        Lang.bind(this, this._onWorkraveVanished));
            return false;
        }
        catch(err)
        {
            return true;
        }
    },

    _connectCore: function() {
    },

    _onDestroy: function() {
        if (this._ui_proxy != null)
        {
            this._ui_proxy.EmbedRemote(false, this._bus_name);
        }
        this._stop();
        this._destroy();
    },

    _destroy: function() {
        if (this._watchid > 0)
        {
            Gio.DBus.session.unwatch_name(this._watchid);
            this._watchid = 0;
        }
        if (this._ui_proxy != null)
        {
            this._ui_proxy.disconnectSignal(this._timers_updated_id);
            this._ui_proxy.disconnectSignal(this._menu_updated_id);
            this._ui_proxy.disconnectSignal(this._menu_item_updated_id);
            this._ui_proxy.disconnectSignal(this._trayicon_updated_id);
            this._ui_proxy = null;
        }
        if (this._core_proxy != null)
        {
            this._core_proxy.disconnectSignal(this._operation_mode_changed_id);
            this._core_proxy = null;
        }
    },

    _start: function() {
        if (! this._alive)
        {
            this._bus_id = Gio.DBus.session.own_name(this._bus_name, Gio.BusNameOwnerFlags.NONE, null, null);
            this._ui_proxy.GetMenuRemote(Lang.bind(this, this._onGetMenuReply));
            this._ui_proxy.GetTrayIconEnabledRemote(Lang.bind(this, this._onGetTrayIconEnabledReply));
            this._ui_proxy.EmbedRemote(true, this._bus_name);
            this._core_proxy.GetOperationModeRemote(Lang.bind(this, this._onGetOperationModeReply));
            this._timeoutId = Mainloop.timeout_add(5000, Lang.bind(this, this._onTimer));
            this._alive = true;
            this._update_count = 0;
        }
    },

    _stop_dbus: function() {
        if (this._alive)
        {
            Mainloop.source_remove(this._timeoutId);
            Gio.DBus.session.unown_name(this._bus_id);
            this._timeoutId = 0;
            this._bus_id = 0;
        }
    },

    _stop: function() {
        if (this._alive)
        {
            this._stop_dbus();
            this._timerbox.set_enabled(false);
            this._timerbox.set_force_icon(false);
            this._alive = false;
            this._updateMenu(null);
            this._area.queue_repaint();
            this._area.set_width(this._width=24);
        }
    },

    _draw: function(area) {
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

        let timerbox_width = this._timerbox.get_width();
        let timerbox_height = this._timerbox.get_height();

        let height = 24;
        if (typeof this.get_height === "function")
        {
            height = this.get_height();
        }
        else
        {
            // Fallback for older Gnome Shell versions
            height = this.actor.height;
        }

        let padding = Math.floor((height - timerbox_height) / 2);

        this._box.style = "padding-top: " + padding + "px;";
        this._padding = padding;

        this._area.set_width(this._width=timerbox_width);
        this._area.queue_repaint();
    },

    _onGetMenuReply : function([menuitems], excp) {
        this._updateMenu(menuitems);
    },

    _onGetTrayIconEnabledReply : function([enabled], excp) {
        this._updateTrayIcon(enabled);
    },

    _onGetOperationModeReply : function([mode], excp) {
        this._timerbox.set_operation_mode(mode);
    },

    _onMenuUpdated : function(emitter, senderName, [menuitems]) {
        this._updateMenu(menuitems);
    },

    _onMenuItemUpdated : function(emitter, senderName, [menuitem]) {
        this._updateItemMenu(menuitem);
    },

    _onTrayIconUpdated : function(emitter, senderName, [enabled]) {
        this._updateTrayIcon(enabled);
    },

    _onOperationModeChanged : function(emitter, senderName, [mode]) {
        this._timerbox.set_operation_mode(mode);
    },

    _onCommandReply : function(menuitems) {
    },

    _onMenuCommand : function(item, event, command) {
        this._ui_proxy.CommandRemote(command, Lang.bind(this, this._onCommandReply));
    },

    _onMenuOpenCommand: function(item, event) {
        this._ui_proxy.GetMenuRemote(); // A dummy method call to re-activate the service
    },

    _updateTrayIcon : function(enabled) {
        this._force_icon = enabled;
        this._timerbox.set_force_icon(this._force_icon);
    },

    _updateMenu : function(menuitems) {
        this.menu.removeAll();
        this._menu_entries = {}

        let current_menu = this.menu;
        let indent = "";

        if (menuitems == null || menuitems.length == 0)
        {
            let popup = new PopupMenu.PopupMenuItem(_("Open Workrave"));
            popup.connect('activate', Lang.bind(this, this._onMenuOpenCommand));
            current_menu.addMenuItem(popup);
        }
        else
        {
            let submenus = []
            for (var item in menuitems)
            {
                let text = indent + menuitems[item][0];
                let dynamic_text = indent + menuitems[item][1];
                let action = menuitems[item][2];
                let id = menuitems[item][3];
                let type = menuitems[item][4];
                let flags = menuitems[item][5];

                let active = ((flags & MENU_ITEM_FLAG_ACTIVE) != 0);
                let visible = ((flags & MENU_ITEM_FLAG_VISIBLE) != 0);
                let popup;

                dynamic_text = dynamic_text.replace("_", "");

                if (type == MENU_ITEM_TYPE_SUBMENU_BEGIN)
                {
                    let popup = new PopupMenu.PopupSubMenuMenuItem(dynamic_text);
                    submenus.push(current_menu);
                    current_menu.addMenuItem(popup);
                    current_menu = popup.menu;
                    indent = " ".repeat(4 * submenus.length); // Look at CSS??
                }
                else if (type == MENU_ITEM_TYPE_SUBMENU_END)
                {
                    current_menu = submenus.pop();
                    indent = " ".repeat(4 * submenus.length); // Look at CSS??
                }
                else if (type == MENU_ITEM_TYPE_SEPARATOR)
                {
                    popup = new PopupMenu.PopupSeparatorMenuItem(dynamic_text);
                }
                else if (type == MENU_ITEM_TYPE_CHECK)
                {
                    popup = new PopupMenu.PopupSwitchMenuItem(dynamic_text, active);
                }
                else if (type == MENU_ITEM_TYPE_RADIO)
                {
                    popup = new PopupMenu.PopupMenuItem(dynamic_text);

                    // Gnome 3.6 & 3.8
                    if (typeof popup.setShowDot === "function")
                    {
                        popup.setShowDot(active);
                    }

                    // Gnome 3.10 & newer
                    else if (typeof popup.setOrnament === "function")
                    {
                        popup.setOrnament(active ? PopupMenu.Ornament.DOT : PopupMenu.Ornament.NONE);
                    }
                }
                else if (type == MENU_ITEM_TYPE_ACTION)
                {
                    popup = new PopupMenu.PopupMenuItem(dynamic_text);
                        popup.setOrnament(
                          active
                            ? PopupMenu.Ornament.DOT
                            : PopupMenu.Ornament.NONE
                        );
                }

                if (popup)
                {
                    popup.setSensitive(visible);
                    popup.connect('activate', Lang.bind(this, this._onMenuCommand, id));
                    current_menu.addMenuItem(popup);
                    this._menu_entries[id] = popup;
                }
            }
        }
    },

    _updateItemMenu : function(menuitem) {
        let id = menuitem[2];
        let type = menuitem[3];
        let flags = menuitem[4];

        let active = ((flags & MENU_ITEM_FLAG_ACTIVE) != 0);
        let visible = ((flags & MENU_ITEM_FLAG_VISIBLE) != 0);
        let popup = this._menu_entries[id];

        global.log('workrave-applet: menu2 ' +  id + ' ' + type +' ' + flags + ' ' + active + ' ' + visible);

        popup.setSensitive(visible);

        if (type == MENU_ITEM_TYPE_CHECK)
        {
            popup.setToggleState(active);
        }
        else if (type == MENU_ITEM_TYPE_RADIO)
        {
            // Gnome 3.6 & 3.8
            if (typeof popup.setShowDot === "function")
            {
                popup.setShowDot(active);
            }

            // Gnome 3.10 & newer
            else if (typeof popup.setOrnament === "function")
            {
                popup.setOrnament(active ? PopupMenu.Ornament.DOT : PopupMenu.Ornament.NONE);
            }
        }
    }
});

let workravePanelButton;
let workraveUserExtensionLocalePath;

function init(extensionMeta) {
    /* do nothing */
    workraveUserExtensionLocalePath = extensionMeta.path + '/locale';
}

function disable() {
    workravePanelButton.destroy();
    workravePanelButton = null;
}

function enable() {
    Gettext.bindtextdomain("workrave", workraveUserExtensionLocalePath);

    workravePanelButton = new WorkraveButton();
    Main.panel.addToStatusArea('workrave-applet', workravePanelButton);
}
