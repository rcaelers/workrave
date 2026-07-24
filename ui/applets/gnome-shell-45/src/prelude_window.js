import Clutter from "gi://Clutter";
import Gio from "gi://Gio";
import GLib from "gi://GLib";
import GObject from "gi://GObject";
import St from "gi://St";
import { gettext as _ } from "resource:///org/gnome/shell/extensions/extension.js";
import Workrave from "gi://Workrave?version=2.0";

var TimeBarConstraint = GObject.registerClass(
  class TimeBarConstraint extends Clutter.Constraint {
    _init(props) {
      super._init(props);
    }

    vfunc_update_allocation(actor, actorBox) {
      let [width, height] = actorBox.get_size();
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

  refresh() {
    this._area.queue_repaint();
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
    this.set_allocation(box);
    let contentBox = this.get_theme_node().get_content_box(box);
    this._area.allocate(contentBox);
  }
}

class SanctuaryProgressBar extends St.Widget {
  static {
    GObject.registerClass({ GTypeName: "WorkraveSanctuaryProgressBar" }, this);
  }

  _init() {
    super._init({
      style_class: "sanctuary-prelude-progress",
      x_expand: true,
      x_align: Clutter.ActorAlign.FILL,
    });

    this._track = new St.Widget({
      style_class: "sanctuary-prelude-progress-track",
      x_expand: true,
    });
    this._fill = new St.Widget({
      style_class: "sanctuary-prelude-progress-fill",
    });
    this.add_child(this._track);
    this.add_child(this._fill);
    this._fraction = 1.0;
  }

  set_progress(value, max_value, _color) {
    this._fraction =
      max_value > 0 ? Math.max(0, Math.min(1, (max_value - value) / max_value)) : 1.0;
    this.queue_relayout();
  }

  set_secondary_progress(_value, _max_value, _color) {}

  set_text(_text) {}

  set_text_alignment(_align) {}

  refresh() {}

  vfunc_get_preferred_width(_forHeight) {
    let themeNode = this.get_theme_node();
    return themeNode.adjust_preferred_width(280, 360);
  }

  vfunc_get_preferred_height(_forWidth) {
    let themeNode = this.get_theme_node();
    return themeNode.adjust_preferred_height(3, 3);
  }

  vfunc_allocate(box) {
    this.set_allocation(box);
    let contentBox = this.get_theme_node().get_content_box(box);
    this._track.allocate(contentBox);

    let fillBox = new Clutter.ActorBox();
    fillBox.x1 = contentBox.x1;
    fillBox.y1 = contentBox.y1;
    fillBox.x2 =
      contentBox.x1 + (contentBox.x2 - contentBox.x1) * this._fraction;
    fillBox.y2 = contentBox.y2;
    this._fill.allocate(fillBox);
  }
}

export class PreludeWindow extends St.Widget {
  static {
    GObject.registerClass({ GTypeName: "Workrave-PreludeWindow" }, this);
  }

  _init(monitor, icon, sad_icon, warn_color, alert_color, sanctuary) {
    super._init({
      reactive: true,
      track_hover: true,
    });

    this._monitor = monitor;
    this._warn_color = warn_color;
    this._alert_color = alert_color;
    this._sanctuary = sanctuary;

    this._blink_timer = null;
    this._blink_stage = "";
    this._blink_on = false;
    this._did_avoid = "";

    this._init_icons(icon, sad_icon);
    this._init_ui();

    this._timebar.set_progress(1, 30, Workrave.ColorId.active);
    this._timebar.set_secondary_progress(0, 0, Workrave.ColorId.active);
    this._timebar.set_text("");
    this._timebar.set_text_alignment(0);
    this.set_stage("initial");
  }

  set_progress(value, max_value, color) {
    this._timebar.set_progress(value, max_value, color);
  }

  set_progress_text(text) {
    if (this._sanctuary) {
      this._countdown_label.set_text(text);
    } else {
      this._timebar.set_text(text);
    }
  }

  set_text(text) {
    if (this._sanctuary) {
      this._label.set_text(text);
    } else {
      let label_text = `<span weight="bold" size="larger">${text}</span>`;
      this._label.set_text(label_text);
      this._label.get_clutter_text().set_use_markup(true);
    }
  }

  refresh(text) {
    this._timebar.refresh();
    this.queue_redraw();
  }

  set_stage(stage) {
    if (stage == "initial") {
      this._normal_icon.show();
      this._sad_icon.hide();
      if (this._blink_timer != null) {
        GLib.source_remove(this._blink_timer);
        this._blink_timer = null;
      }
    } else if (stage == "warn" || stage == "alert") {
      this._normal_icon.hide();
      this._sad_icon.show();
      this._blink_on = false;
      this._blink_stage = stage;
      if (this._blink_timer == null) {
        this._blink_update();
        this._blink_timer = GLib.timeout_add(GLib.PRIORITY_DEFAULT, 500, () => {
          return this._on_blink_timer();
        });
      }
    } else if (stage == "move-out") {
      if (this._did_avoid == "") {
        this._blink_stage = "";
        this._blink_update();
        this._did_avoid = "top";
      }
    }
  }

  _on_blink_timer() {
    this._blink_on = !this._blink_on;
    this._blink_update();
    this.sync_hover();
    return true;
  }

  _blink_update() {
    if (this._blink_on) {
      let color =
        this._blink_stage === "alert" ? this._alert_color : this._warn_color;
      this._frame.set_style(`border-color: ${color};`);
    } else {
      this._frame.set_style("");
    }
  }

  _get_icon(filename) {
    let gicon = new Gio.FileIcon({ file: Gio.File.new_for_path(filename) });
    return new St.Icon({ gicon: gicon, icon_size: 48 });
  }

  _init_icons(icon, sad_icon) {
    this._normal_icon = this._get_icon(icon);
    this._sad_icon = this._get_icon(sad_icon);
    this._sad_icon.hide();
  }

  _init_ui() {
    let default_text = _("Time for a microbreak?");
    let label = new St.Label({
      text: this._sanctuary
        ? default_text
        : `<span weight="bold" size="larger">${default_text}</span>`,
    });
    if (this._sanctuary) {
      label.add_style_class_name("sanctuary-prelude-heading");
    } else {
      label.get_clutter_text().set_use_markup(true);
    }

    let timebar = this._sanctuary ? new SanctuaryProgressBar() : new TimeBar();
    let countdown_label = new St.Label({
      style_class: "sanctuary-prelude-countdown",
      text: "",
    });

    let vbox = new St.BoxLayout({
      style_class: this._sanctuary ? "sanctuary-prelude-info" : "prelude-info",
      vertical: true,
      reactive: true,
    });
    vbox.add_child(label);
    if (this._sanctuary) {
      vbox.add_child(countdown_label);
    }
    vbox.add_child(timebar);

    let hbox = new St.BoxLayout({
      style_class: this._sanctuary
        ? "sanctuary-prelude-content"
        : "prelude-content",
      reactive: true,
      vertical: false,
    });
    if (this._sanctuary) {
      let icon_badge = new St.BoxLayout({
        style_class: "sanctuary-prelude-icon-badge",
      });
      icon_badge.add_child(this._normal_icon);
      icon_badge.add_child(this._sad_icon);
      hbox.add_child(icon_badge);
    } else {
      hbox.add_child(this._normal_icon);
      hbox.add_child(this._sad_icon);
    }
    hbox.add_child(vbox);

    let inner_frame = new St.BoxLayout({
      style_class: this._sanctuary
        ? "sanctuary-prelude-frame"
        : "prelude-frame",
      reactive: true,
    });
    inner_frame.add_child(hbox);

    let outer_frame = new St.BoxLayout({
      style_class: this._sanctuary
        ? "sanctuary-prelude-window"
        : "prelude-window",
      reactive: true,
    });
    outer_frame.add_child(inner_frame);

    this.add_child(outer_frame);
    this.set_track_hover(true);

    let enterId = this.connect("enter-event", () => {
      let [x, y] = global.get_pointer();
      if (y < this._monitor.y + this._monitor.height / 2) {
        this._did_avoid = "top";
      } else {
        this._did_avoid = "bottom";
      }
      this.queue_relayout();

      return Clutter.EVENT_PROPAGATE;
    });

    this._frame = inner_frame;
    this._timebar = timebar;
    this._label = label;
    this._countdown_label = countdown_label;
  }

  avoid(window_height) {
    let screen_height = this._monitor.height;
    let top_y = this._monitor.y + 20;
    let bottom_y = this._monitor.y + screen_height - window_height - 20;

    if (this._did_avoid == "top") {
      return bottom_y;
    } else if (this._did_avoid == "bottom") {
      return top_y;
    }
    return (screen_height - window_height) / 2 + this._monitor.y;
  }

  vfunc_allocate(box) {
    this.set_allocation(box);

    let contentBox = this.get_theme_node().get_content_box(box);

    this.get_first_child().allocate(contentBox);

    let [winWidth, winHeight] = [
      contentBox.x2 - contentBox.x1,
      contentBox.y2 - contentBox.y1,
    ];

    let centerX =
      this._monitor.x + Math.floor((this._monitor.width - winWidth) / 2);
    let centerY =
      this._monitor.y + Math.floor((this._monitor.height - winHeight) / 2);

    if (this._did_avoid != "") {
      centerY = this.avoid(winHeight);
    }

    this.set_position(centerX, centerY);
  }
}
