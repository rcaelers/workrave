import Clutter from "gi://Clutter";
import Gio from "gi://Gio";
import GLib from "gi://GLib";
import GObject from "gi://GObject";
import St from "gi://St";
import {
  Extension,
  gettext as _,
} from "resource:///org/gnome/shell/extensions/extension.js";
import Workrave from "gi://Workrave?version=2.0";

const ICON_DIR = ""; // Extension.lookupByURL(import.meta.url).dir.get_child('images');

var TimeBarConstraint = GObject.registerClass(
  class TimeBarConstraint extends Clutter.Constraint {
    _init(props) {
      super._init(props);
    }

    vfunc_update_allocation(actor, actorBox) {
      let [width, height] = actorBox.get_size();
      console.log(
        "workrave-applet: timebar4: " +
          width +
          " " +
          height +
          " " +
          actor.margin_bottom
      );
      actor.set_size(Math.max(100, width), Math.max(40, height));
    }
  }
);

class TimeBar extends St.Widget {
  static {
    GObject.registerClass({ GTypeName: "WorkraveTimeBar" }, this);
  }

  _init() {
    super._init({
      style_class: "timebar-window",
      x_expand: true,
      y_expand: true,
      x_align: Clutter.ActorAlign.FILL,
      y_align: Clutter.ActorAlign.FILL,
    });

    this._area = new St.DrawingArea({
      x_expand: true,
      y_expand: true,
      x_align: Clutter.ActorAlign.FILL,
      y_align: Clutter.ActorAlign.FILL,
    });
    this._area.add_style_class_name("timebar-area");
    this._area.connect("repaint", () => {
      this._onDraw();
    });
    this.add_child(this._area);

    this._timebar = new Workrave.Timebar();
  }

  set_progress(value, max_value, color) {
    this._timebar.set_progress(value, max_value, color);
  }

  set_secondary_progress(value, max_value, color) {
    this._timebar.set_secondary_progress(value, max_value, color);
  }

  set_text(text) {
    this._timebar.set_text(text);
  }

  set_text_alignment(align) {
    this._timebar.set_text_alignment(align);
  }

  _onDraw() {
    let cr = this._area.get_context();

    let [width, height] = this._area.get_surface_size();
    this._timebar.set_dimensions(width, height);

    this._timebar.draw(cr);
    cr.$dispose();
  }

  vfunc_get_preferred_width(forHeight) {
    let themeNode = this.get_theme_node();
    return themeNode.adjust_preferred_width(160, 160);
  }

  vfunc_get_preferred_height(_forWidth) {
    let themeNode = this.get_theme_node();
    return themeNode.adjust_preferred_height(24, 24);
  }

  vfunc_allocate(box) {
    console.log(
      "workrave-applet: timebar5: " +
        box.x1 +
        " " +
        box.y1 +
        " " +
        box.x2 +
        " " +
        box.y2
    );
    this.set_allocation(box);

    let contentBox = this.get_theme_node().get_content_box(box);
    console.log(
      "workrave-applet: timebar5: " +
        contentBox.x1 +
        " " +
        contentBox.y1 +
        " " +
        contentBox.x2 +
        " " +
        contentBox.y2
    );
    this._area.allocate(contentBox);
  }
}

export class PreludeWindow extends St.Widget {
  static {
    GObject.registerClass({ GTypeName: "Workrave-PreludeWindow" }, this);
  }

  _init(monitor) {
    super._init({
      reactive: true,
      track_hover: true,
    });

    this._monitor = monitor;
    this._blink_timer = null;
    this._blink_stage = "";
    this._blink_on = false;

    this._init_icons();
    this._init_ui();

    this._timebar.set_progress(1, 30, Workrave.ColorId.active);
    this._timebar.set_secondary_progress(0, 0, Workrave.ColorId.active);
    this._timebar.set_text("Hello World");
    this._timebar.set_text_alignment(0);
    this.set_stage("initial");
  }

  set_progress(value, max_value, color) {
    this._timebar.set_progress(value, max_value, color);
  }

  set_progress_text(text) {
    this._timebar.set_text(text);
  }

  set_text(text) {
    let label_text = `<span weight="bold" size="larger">${text}</span>`;
    this._label.set_text(label_text);
    this._label.get_clutter_text().set_use_markup(true);
  }

  refresh(text) {
    this._timebar.queue_redraw();
  }

  set_stage(stage) {
    if (stage == "initial") {
      this._normal_icon.show();
      this._sad_icon.hide();
      if (this._blink_timer != null) {
        GLib.source_remove(this._blink_timer);
      }
    } else if (stage == "warn" || stage == "alert") {
      this._normal_icon.hide();
      this._sad_icon.show();
      this._blink_on = false;
      if (this._blink_stage != "") {
        this._frame.remove_style_class_name(
          "prelude-frame-" + this._blink_stage
        );
        this._blink_stage = stage;
      }
      this._blink_update();
      if (this._blink_timer == null) {
        this._blink_timer = GLib.timeout_add(GLib.PRIORITY_DEFAULT, 500, () => {
          return this._on_blink_timer();
        });
      }
    } else if (stage == "move-out") {
      if (!this.did_avoid) {
        this._blink_stage = "move-out";
        this._blink_update();
        this.did_avoid = true;
        this.queue_relayout();
      }
    }
  }

  _on_blink_timer() {
    this._blink_update();
    this.sync_hover();
    return true;
  }

  _blink_update() {
    this._blink_on = !this._blink_on;

    if (this._blink_on) {
      this._frame.add_style_class_name("prelude-frame-" + this._blink_stage);
    } else {
      this._frame.remove_style_class_name("prelude-frame-" + this._blink_stage);
    }
  }

  _get_icon(filename) {
    let path = ""; // ICON_DIR.get_child(filename).get_path();
    let gicon = new Gio.FileIcon({ file: Gio.File.new_for_path(path) });
    return new St.Icon({ gicon: gicon, icon_size: 48 });
  }

  _init_icons() {
    this._normal_icon = this._get_icon("prelude-hint.png");
    this._sad_icon = this._get_icon("prelude-hint-sad.png");
    this._sad_icon.hide();
  }

  _init_ui() {
    let text = `<span weight="bold" size="larger">${_(
      "Time for a microbreak?"
    )}</span>`;
    let label = new St.Label({ text: text });
    label.get_clutter_text().set_use_markup(true);

    let timebar = new TimeBar();

    let vbox = new St.BoxLayout({
      vertical: true,
      reactive: true,
      style: "spacing: 4px;",
    });
    vbox.add_child(label);
    vbox.add_child(timebar);

    let hbox = new St.BoxLayout({
      style_class: "prelude-content",
      reactive: true,
      vertical: false,
    });
    hbox.add_child(this._normal_icon);
    hbox.add_child(this._sad_icon);
    hbox.add_child(vbox);

    let inner_frame = new St.BoxLayout({
      style_class: "prelude-frame",
      reactive: true,
    });
    inner_frame.add_child(hbox);

    let outer_frame = new St.BoxLayout({
      style_class: "prelude-window",
      reactive: true,
    });
    outer_frame.add_child(inner_frame);

    this.add_child(outer_frame);

    this.set_track_hover(true);

    this.connect("notify::hover", () => {
      console.log("workrave-applet: hover");
    });

    let enterId = this.connect("enter-event", () => {
      console.log("workrave-applet: enter");
    });

    outer_frame.connect("enter-event", () => {
      console.log("workrave-applet: enter o");
    });
    inner_frame.connect("enter-event", () => {
      console.log("workrave-applet: enter i");
    });
    vbox.connect("enter-event", () => {
      console.log("workrave-applet: enter c");
    });
    hbox.connect("enter-event", () => {
      console.log("workrave-applet: enter h");
    });

    this._frame = inner_frame;
    this._timebar = timebar;
    this._label = label;
  }

  vfunc_allocate(box) {
    console.log(
      "workrave-applet: vfunc prelude allocate1: " +
        box.x1 +
        " " +
        box.y1 +
        " " +
        box.x2 +
        " " +
        box.y2
    );
    this.set_allocation(box);

    let contentBox = this.get_theme_node().get_content_box(box);
    console.log(
      "workrave-applet: vfunc_prelude allocate2: " +
        contentBox.x1 +
        " " +
        contentBox.y1 +
        " " +
        contentBox.x2 +
        " " +
        contentBox.y2
    );

    this.get_first_child().allocate(contentBox);

    let [winWidth, winHeight] = [
      contentBox.x2 - contentBox.x1,
      contentBox.y2 - contentBox.y1,
    ];

    let centerX =
      this._monitor.x + Math.floor((this._monitor.width - winWidth) / 2);
    let centerY =
      this._monitor.y + Math.floor((this._monitor.height - winHeight) / 2);

    if (this.did_avoid) {
      centerY = this._monitor.y + 50;
    }

    this.set_position(centerX, centerY);
  }
}

//  add(*frame);
//
//  switch (break_id)
//    {
//    case BREAK_ID_MICRO_BREAK:
//      label->set_markup(HigUtil::create_alert_text(_("Time for a micro-break?"), NULL));
//      break;
//
//    case BREAK_ID_REST_BREAK:
//      label->set_markup(HigUtil::create_alert_text(_("You need a rest break..."), NULL));
//      break;
//
//    case BREAK_ID_DAILY_LIMIT:
//      label->set_markup(HigUtil::create_alert_text(_("You should stop for today..."), NULL));
//      break;
//
//    default:
//      break;
//    }
//
