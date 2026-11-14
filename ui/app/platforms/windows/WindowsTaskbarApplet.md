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
shutdown. The existing GTK and Qt Widgets Applet panels bind their controls to
the applet settings.

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

## Native errors and resource ownership

`WindowsImaging.hh` and `WindowsAutomation.hh` provide owning C++ wrappers for
WIC images and cached UI Automation elements. Application code loads images,
reads sizes/bounds and copies pixels using ordinary methods with returned values.
HRESULT checks remain inside these wrappers at the native API boundary;
`std::system_error` preserves the failed operation and original HRESULT.
`WindowsResources.hh` supplies owning factories such as `create_compatible_dc`,
`create_compatible_bitmap`, `create_font` and `create_popup_menu`. These free
functions return RAII handles and throw on failure. `create_dib_section` returns
both the owned bitmap and its pixel buffer. `append_submenu` takes an owned menu,
transfers it to the parent on success and destroys it on failure. The underlying
error checks remain at this API boundary.
`ComApartment` balances successful initialization, including
`S_FALSE`. Icon loading may borrow an existing apartment in another mode without
uninitializing it; the UI Automation worker requires MTA initialization.

COM interfaces use `ComPtr`. Bitmaps, brushes, fonts, icons, drawing contexts and
popup menus have scoped owners. GDI selections are restored before their owned
objects are freed, and `PaintScope` balances `BeginPaint`/`EndPaint` during
unwinding. Parent menus take ownership of submenus only after insertion succeeds.

Icon failures are caught per image and logged; a failed reload retains the
previous usable icon, and other timers still load. A timer without an icon uses
its letter fallback. Scan failures are caught in the worker and publish an
unavailable snapshot; setup errors stop the worker safely. Native event handlers
catch drawing/menu errors after temporary resources have been released.

`windows-resources-test` deliberately throws after allocating native bitmaps and
icons and verifies their release, submenu ownership transfer, GDI selection
restoration and balanced nested COM initialization, including borrowed/different-mode
apartments. The imaging checks decode an actual PNG through the wrappers, exercise a failed pixel copy, and verify that the image
remains usable after failure.

## Current scope

This implementation targets the primary horizontal Windows 10/11 taskbar.
Secondary taskbars, vertical taskbars, dragging and private XAML space
reservation are not implemented. The native view follows the classic main
window's icon-and-bar layout and default palette;
custom GTK CSS colours are not read by the taskbar applet.
The palette follows the Windows system appearance, read from
`SystemUsesLightTheme`, independently of Workrave's appearance and Windows app
mode. Light mode uses beige and black; dark mode uses a dark background and light
text, keeping black text over coloured progress fills. Native theme notifications
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

1. Enable or disable **Show timers in taskbar** in **User interface > Applet**,
   restart Workrave, and verify persistence.
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
