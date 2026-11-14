# Windows taskbar applet implementation

For setup and usage, see the [user guide](../../../../docs/windows-taskbar-applet.md).

Both the Qt and GTK Windows frontends use the same native Win32 implementation.
The plugin reuses the existing `gui/applet` settings for enablement, grouping,
cycle time and timer visibility. Saved applet preferences apply automatically.
The defaults are enabled, with all timers cycling in one slot.

## Hosting and placement

`WindowsTaskbarApplet` is a compiled-in `Plugin<WindowsTaskbarApplet>` with ID
`workrave.WindowsTaskbarApplet`. Its implementation is private to
`ui/app/platforms/windows`; neither toolkit nor the application instantiates or
references it. Static registration makes `PluginRegistry::build()` create it
using `IPluginContext`, and `PluginRegistry::deinit()` destroys it before toolkit
shutdown. The plugin registers and removes the entire Windows Applet preference
panel through `IPreferencesRegistry`, using `PanelDef("ui", "applet", "Applet")`.
Its prefwidgets provide taskbar enablement, timer grouping, cycle time and
per-timer visibility, bound to the existing applet settings. The built-in Applet
panel remains unconditional on Linux. On Windows, the registered panel supplies
the tab, without legacy applet, status-icon or toolkit-specific style controls.
The definitions include [generic presentation hints](../../../libs/prefwidgets/Presentation.md)
for illustrated timer grouping, segmented visibility choices and the cycle-time
editor. The application supplies named timer grouping previews and colors; the
prefwidget library contains no timer-specific presentation. Its Qt Quick backend renders the hints,
while GTK currently retains the original controls and labels.

See [PreferencesRegistry](../../PreferencesRegistry.md) for panel contributions,
named insertion sections, error handling and shared-panel ordering.

It implements `ITimerBoxView` and uses `TimerBoxControl`.
Its child window belongs to Workrave and is reparented into `Shell_TrayWnd`
using `SetParent`. Creation uses Explorer's window DPI awareness context;
Workrave's thread DPI context is restored before toolkit calls.
The applet is an opaque layered child window so its GDI drawing has its own
composited surface alongside the Windows 11 taskbar's DirectComposition content.

`TaskbarLayout` queries UI Automation on a windowless COM MTA worker. Each scan
caches control bounds, visibility, types and process IDs in one operation. The
UI receives only geometry, without sharing COM objects across threads. Buttons,
task list items, search controls and the notification area are obstacles. The
applet ignores its own windows during discovery.

The placement algorithm selects the rightmost gap that fits the display and its
clearance. It never moves Start, app buttons or the notification area. If
discovery fails, geometry is stale, or no gap fits, the applet hides and retries.
It polls once per second and recreates its child after Explorer restarts. It
does not fall back to a floating or topmost window.

## Current scope

This implementation targets the primary horizontal Windows 10/11 taskbar.
Secondary taskbars, vertical taskbars, dragging and private XAML space
reservation are not implemented. The applet uses its own compact text/bar view;
the Qt status window's rings, focus and other styles do not apply. The native
view follows the classic main window's icon-and-bar layout and default palette;
custom GTK CSS colours are not read by the taskbar applet.
The palette follows `GUIConfig::light_dark_mode()`. Light mode uses beige and
black; Dark mode uses a dark background and light text, keeping black text over
coloured progress fills. Auto reads the Windows `AppsUseLightTheme` preference,
independently of the taskbar theme. Setting changes and native theme notifications
invalidate the applet immediately. Windows high contrast mode uses the system
colours.

Taskbar hosting is dependent on Explorer's implementation. UI Automation can
miss custom-drawn controls from other taskbar modifications. Occupancy can change
between scans, so an overlap can persist until the next geometry update. No
Windows build is considered supported solely because compilation succeeds.

## Verification

`taskbar-placement-test` is registered with CTest when `WITH_TESTS=ON`. It checks
known layouts and compares 20,000 deterministic randomized layouts against an
independent pixel-by-pixel placement oracle. It is portable and can run under
sanitizers without Windows.

Before shipping, test on a real Windows desktop:

1. Enable/disable through **User interface > Applet > Taskbar**, restart Workrave,
   and verify persistence. Check that the menu no longer has an applet toggle.
2. Check all three timers, overdue values, idle-reset progress, grouping,
   cycling, hidden/disabled breaks, quiet mode and suspended mode.
3. Exercise left/middle/right clicks, hover, menu dismissal and Quit.
4. Switch taskbar alignment, light/dark themes, high contrast and taskbar sizes.
5. Open enough applications to crowd the bar; check hiding and recovery without
   covering taskbar controls. Also test the Widgets button and taskbar auto-hide.
6. Restart Explorer and verify recovery without restarting Workrave.
7. Change primary monitor, resolution and DPI (100%, 150%, 200%), including
   monitors with negative coordinates. Confirm the main and break windows keep
   their original DPI behaviour.
8. Quit with the applet both enabled and disabled, including during a provider
   timeout. Check that no worker, tooltip or child window remains.

The Qt host registers preview assets in `PreferencePresentations.qml`. Grouping
geometry lives in `TimerGroupingPreview.qml`; display-style geometry lives in
`TimerStylePreview.qml`. Both built-in pages and prefwidget cards resolve these
assets through the library's generic `PreviewRegistry`. For example, a choice
can reference `workrave.timer-style.rings` to reuse the status-window preview.
This does not add support for that style to the native taskbar renderer.

The entire grouping control is registered as `workrave.timer-grouping` and
implemented by `TimerPlacementPicker.qml`. The status-window page instantiates
that component directly; the applet's `ChoicePresentation::name` loads the same
component through the generic library registry. The C++ definition contains only
the native choice labels, preference binding and registered presentation name.
