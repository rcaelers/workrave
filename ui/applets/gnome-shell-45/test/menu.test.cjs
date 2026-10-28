// Run with: node --test ui/applets/gnome-shell-45/test/menu.test.cjs
const assert = require("node:assert/strict");
const { readFileSync } = require("node:fs");
const { test } = require("node:test");
const vm = require("node:vm");

// Exercise the extension's menu methods without starting GNOME Shell.
const source = readFileSync(`${__dirname}/../src/extension.js`, "utf8");
const constants = source.slice(
  source.indexOf("const MENU_ITEM_TYPE_SUBMENU_BEGIN"),
  source.indexOf("let CoreProxy"),
);
const button = source.slice(
  source.indexOf("const WorkraveButton = GObject.registerClass("),
  source.indexOf("let workravePanelButton;"),
);

class MenuItem {
  constructor(text, active = false) {
    this.state = active;
    this.handlers = new Map();
  }

  connect(signal, callback) {
    const id = this.handlers.size + 1;
    this.handlers.set(id, { signal, callback, blocked: false });
    return id;
  }

  emit(signal, ...args) {
    for (const handler of this.handlers.values()) {
      if (handler.signal === signal && !handler.blocked)
        handler.callback(this, ...args);
    }
  }

  setSensitive() {}
  setOrnament() {}

  setToggleState(state) {
    if (this.state === state) return;
    this.state = state;
    // GNOME Shell 48+ emits toggled for programmatic changes, too.
    this.emit("toggled", state);
  }

  activate(event) {
    this.setToggleState(!this.state);
    this.emit("activate", event);
  }
}

const context = vm.createContext({
  GObject: {
    registerClass: (klass) => klass,
    signal_handler_block: (item, id) => {
      item.handlers.get(id).blocked = true;
    },
    signal_handler_unblock: (item, id) => {
      item.handlers.get(id).blocked = false;
    },
  },
  PanelMenu: { Button: class {} },
  PopupMenu: {
    PopupSwitchMenuItem: MenuItem,
    PopupMenuItem: MenuItem,
    Ornament: { DOT: 1, NONE: 0 },
  },
});
vm.runInContext(`${constants}\n${button}\nthis.Button = WorkraveButton;`, context);

const readingItem = (active) => [
  "Reading mode", "Reading mode", "workrave.mode_reading", 42, 6,
  2 | (active ? 1 : 0),
];

function fixture(active = false) {
  const applet = Object.create(context.Button.prototype);
  const commands = [];
  applet._ui_proxy = {
    CommandRemote: (command) => commands.push(command),
  };
  applet.menu = { removeAll() {}, addMenuItem() {} };
  applet._updateMenu([readingItem(active)]);
  return {
    applet,
    commands,
    popup: applet._menu_entries["workrave.mode_reading"],
  };
}

test("incoming reading-mode changes update the switch without sending commands", () => {
  const { applet, commands, popup } = fixture();
  for (const active of [true, false, true, true, false]) {
    applet._updateItemMenu(readingItem(active));
    assert.equal(popup.state, active);
    assert.deepEqual(commands, []);
  }
});

test("initial reading mode does not send a toggle command", () => {
  const { commands, popup } = fixture(true);
  assert.equal(popup.state, true);
  assert.deepEqual(commands, []);
});

test("row activation and direct switch gestures each send one command", () => {
  const { applet, commands, popup } = fixture();
  popup.activate({});
  assert.deepEqual(commands, [42]);
  applet._updateItemMenu(readingItem(true));
  assert.deepEqual(commands, [42]);
  popup.setToggleState(false); // A direct switch gesture emits no activate.
  assert.deepEqual(commands, [42, 42]);
  applet._updateItemMenu(readingItem(false));
  assert.deepEqual(commands, [42, 42]);
});

test("the user callback is restored even if updating the switch throws", () => {
  const { applet, commands, popup } = fixture();
  const setToggleState = popup.setToggleState;
  popup.setToggleState = () => { throw new Error("update failed"); };
  assert.throws(() => applet._updateItemMenu(readingItem(true)), /update failed/);
  popup.setToggleState = setToggleState;
  popup.activate({});
  assert.deepEqual(commands, [42]);
});
