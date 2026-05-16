# Qt6 Port TODO

This tracks functional differences and missing features found by comparing the Qt6 implementation with the Gtk+3 implementation in `ui/app/toolkits/gtkmm`.

## User-Visible Functionality Lost or Regressed

These are the differences a user is likely to notice when switching from the Gtk+3 UI to the Qt6 UI.

### High Impact

- [x] Status window visibility behaves differently.
  - Gtk lets the user hide/close the status window while keeping Workrave alive through the tray icon or applet.
  - Qt now honors `gui/main_window/enabled`, supports open/close from the toolkit, and keeps Workrave alive when the status window is hidden.

- [x] System tray/status icon actions are incomplete.
  - Gtk reacts to tray icon activation and balloon/notification activation.
  - Qt now wires tray icon activation and notification clicks through the toolkit signals/notification callbacks.

- [x] Break blocking is less robust on Windows/macOS.
  - Gtk has mature fullscreen/input blocking behavior, platform-specific focus handling, and topmost handling.
  - Qt now uses per-screen fullscreen topmost blocker windows, re-raises the break window above them, reasserts Windows topmost/focus, and preserves the pre-break foreground app on Windows/macOS.
  - Windows standard locking now installs its low-level keyboard hook, uses the screensaver-running guard, and restores the pre-break foreground window.

- [ ] Break blocking is less robust on Unix/Wayland.
  - Gtk has mature X11 input grabbing and Wayland layer-shell behavior.
  - Qt still has incomplete Unix/Wayland input locking and platform-specific foreground behavior.
  - User impact: during breaks on Unix/Wayland, the user may be able to interact with other windows, move focus away, or see less reliable full-screen blocking than in Gtk.

- [x] Windows startup preference does not work in Qt.
  - Gtk can enable/disable "Start Workrave on logon" by writing the Windows Run registry key.
  - Qt now initializes the checkbox from the same Windows Run registry key and updates it when toggled.

- [x] Statistics history deletion is missing.
  - Gtk has a "Delete all statistics history" button with confirmation and retry handling.
  - Qt now shows the delete button and implements confirmation, retry, and operation-mode override behavior.

- [x] Activity statistics are missing.
  - Gtk shows an Activity tab with mouse usage, mouse movement, effective mouse movement, clicks, and keystrokes on non-macOS platforms.
  - Qt now adds the same Activity tab on non-macOS platforms and keeps macOS without detailed activity statistics.

### Medium Impact

- [x] Statistics do not refresh the same way.
  - Gtk updates statistics before opening the dialog and starts a periodic refresh.
  - Qt now updates statistics before initial display and starts the refresh timer when the toolkit opens the dialog.

- [x] "Rest break now" from a micro-break is missing.
  - Gtk can show a Rest Break button during a micro-break when the rest-break timer is enabled.
  - Qt now shows the Rest Break button when the rest-break timer is enabled and starts a rest break through the core.
  - User impact: users cannot turn a micro-break into a rest break from that prompt.

- [x] Closing a non-blocking break prompt does not postpone it.
  - Gtk treats closing a non-blocking break window as postponing the break.
  - Qt now handles close events the same way for non-blocking break windows and keeps blocking break windows open.
  - User impact: closing a non-blocking break prompt may do nothing useful or behave inconsistently.

- [x] Sound preferences are less capable.
  - Gtk disables the sound event controls when sounds are off, filters selectable files to wave files, and has preview playback in the file chooser.
  - Qt now disables the event controls when sounds are off, filters the chooser to wave files, and adds a preview play button.
  - User impact: sound setup is easier to misuse and has fewer controls.

- [x] Custom sound selection does not track the selected event as well.
  - Gtk updates the chooser to show the file for the selected sound event.
  - Qt now shows the selected event's sound file and updates it when the event, theme, or custom file changes.
  - User impact: it is less clear which sound file is assigned to each event.

- [x] Windows light/dark appearance preference is missing.
  - Gtk exposes Light, Dark, and Auto on Windows and responds to Windows theme changes.
  - Qt now exposes Light, Dark, and Auto on Windows, applies the selected color scheme, and updates Auto when Windows reports theme changes.
  - User impact: users lose explicit control over Workrave's light/dark mode.

- [x] Icon theme selection is missing.
  - Gtk can select an icon theme from installed image theme directories.
  - Qt now scans image theme directories and writes `GUIConfig::icon_theme()` from General preferences.
  - User impact: users cannot switch Workrave icon themes from preferences.

- [x] Plugin/commonui preference pages may miss text-entry controls.
  - Gtk supports commonui `Entry` widgets.
  - Qt now implements the commonui `Entry` widget and wires it into the prefwidgets builder.

### Lower Impact or Polish, But Still User-Visible

- [x] The About dialog has less information.
  - Gtk shows authors, translators, website metadata, logo, copyright, and version.
  - Qt now shows authors, translator credits, website metadata, logo, copyright, and version.
  - User impact: credits and translator information are missing.

- [x] Tray/status tooltip is missing.
  - Gtk updates the status icon tooltip.
  - Qt now forwards toolkit tooltip updates to the system tray icon.

- [x] Status-window position restore differs on multi-monitor setups.
  - Gtk stores position relative to the selected monitor and handles negative offsets.
  - Qt now stores the main window position relative to the selected screen, including right/bottom negative offsets.
  - User impact: the status window previously could reappear in a different or awkward place after monitor changes.

- [x] The status window context menu can appear at inappropriate times.
  - Gtk suppresses the menu while a blocking break is active, depending on block mode.
  - Qt now updates the menu model and suppresses the context menu during input/all blocking breaks.

- [x] Timer-box icon behavior is incomplete.
  - Gtk changes the sheep/status icon for normal, quiet, and suspended modes and reacts to icon-theme changes.
  - Qt now implements `TimerBoxView::set_icon()` and refreshes timer/status icons when the icon theme changes.

- [x] Rest-break exercises may appear on the wrong screen.
  - Gtk only shows the exercise player on the primary monitor and shows the info panel elsewhere.
  - Qt now shows exercises only on the primary screen and uses the info panel elsewhere.
  - User impact: multi-monitor users may see duplicate or misplaced exercise panels.

- [x] Language selection is less polished.
  - Gtk shows both current-locale and native-language names, sorts with locale collation, and disables languages whose native name cannot be rendered.
  - Qt now uses the shared locale language maps, shows current/native names, sorts with locale collation, and disables entries with unsupported glyphs.
  - User impact: the language list previously was harder to scan or could include entries that rendered poorly.

- [x] Crash reporter text is not localized in Qt.
  - Gtk uses translated strings and shows detailed crash info/attachment controls.
  - Qt now wraps user-visible crash reporter strings in a translation context and mirrors the detailed crash report view.
  - User impact: translated builds previously still showed English crash reporter text, with fewer crash details.

## Implementation Notes

The rest of this file keeps lower-level notes that explain where the user-visible gaps come from.

## Toolkit and Application Integration

- [x] Wire Qt `MainWindow` close events into `Toolkit::signal_main_window_closed()`.
  - Gtk: `MainWindow::signal_closed()` is connected in `Toolkit::init()`.
  - Qt: `MainWindow::signal_closed()` is connected and close events are ignored after the toolkit handles hide/quit semantics.
- [x] Wire Qt status icon activation and notification activation into toolkit signals.
  - Gtk: `StatusIcon::signal_activated()` and `signal_balloon_activated()` are connected.
  - Qt: tray activation is connected and `QSystemTrayIcon::messageClicked` maps back to `notify_confirm(id)`.
- [x] Implement `Toolkit::show_tooltip()` for the Qt tray icon.
  - Qt forwards tooltip text to `StatusIcon::set_tooltip()`.
- [x] Return a useful display name from `Toolkit::get_display_name()`.
  - Qt returns the generic platform name, with `ToolkitUnix` returning Wayland/X11 display names when available.
- [x] Match Gtk one-shot timer behavior for `ms == 0`.
  - Qt now uses `QTimer::singleShot`, including zero-delay callbacks.
- [x] Review Qt dialog presentation semantics.
  - Gtk uses `present()` for existing windows and calls `run()` where needed.
  - Qt now calls `StatisticsDialog::run()` on creation and raises/activates the existing statistics dialog.

## Main Status Window and Timer Box

- [x] Implement Qt main-window open/close behavior equivalent to Gtk.
  - Gtk supports `open_window()`, `close_window()`, `set_can_close()`, and updates `GUIConfig::timerbox_enabled("main_window")`.
  - Qt now has matching open/close/can-close methods and no longer unconditionally shows the status window at startup.
- [x] React to `GUIConfig::timerbox_enabled("main_window")` changes.
  - Gtk opens or hides/iconifies the status window when the config changes.
  - Qt now tracks the main-window timerbox setting and updates visibility when it changes.
- [x] Implement close handling for the Qt status window.
  - Gtk prevents destruction, hides or iconifies based on `can_close`, and emits a closed signal.
- [x] Match Gtk positioning semantics.
  - Gtk stores window coordinates relative to the selected monitor and supports negative offsets from the right/bottom edge.
  - Qt stores relative screen coordinates and restores them against the current screen geometry.
- [x] Update and gate the context menu before showing it.
  - Gtk suppresses the menu during blocking modes that should not allow interaction and calls `menu_model->update()` before popup.
  - Qt now applies the same block-mode guard and updates the menu model before popup.
- [x] Complete `TimerBoxView::set_icon()` and icon theme refresh.
  - Gtk updates the sheep/status icon for normal, quiet, and suspended modes and reacts to `GUIConfig::icon_theme()`.
  - Qt now updates the sheep/status icon and refreshes timer icons on icon-theme changes.
- [x] Add the rest-break-now action to the timer box.
  - Gtk makes the rest-break timer icon clickable and calls `core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated)`.
  - Qt now makes the rest-break timer icon clickable and calls the same core action.
- [ ] Revisit timer-box geometry/orientation support.
  - Gtk supports horizontal/vertical geometry and explicit sizing; Qt currently uses a fixed two-column grid.

## Break and Prelude Windows

- [ ] Complete blocking/fullscreen behavior for Qt break windows.
  - Gtk has fullscreen-grab handling, transparent overlays, desktop-background windows, skip-taskbar/pager hints, and platform-specific focus handling.
  - Qt creates a per-screen fullscreen `block_window` and improves Windows/macOS locking/focus, but Unix/Wayland input locking and some foreground behavior remain TODOs.
- [x] Improve Windows/macOS break locking and focus behavior.
  - Windows now reasserts topmost/focus on refresh, installs the standard locker hook correctly, applies the screensaver-running guard, and restores the pre-break foreground window.
  - macOS now captures the pre-break foreground application in `prepare_lock()` before break windows are shown, then restores it on unlock.
- [x] Re-lock after system operations from break windows.
  - Gtk unlocks, schedules a relock after 5 seconds, then executes suspend/lock/shutdown.
  - Qt now unlocks, schedules a relock after 5 seconds, then executes the selected operation.
- [x] Implement close handling for non-blocking Qt break windows.
  - Gtk treats closing a non-blocking break window as postpone.
  - Qt now maps close events in non-blocking mode to postpone and ignores the close event.
- [x] Fix locked postpone tooltip text.
  - Qt uses the skip wording for the locked postpone button; Gtk distinguishes skip and postpone text.
  - Qt now uses postpone-specific wording for the locked postpone button.
- [x] Add micro-break "Rest break" button.
  - Gtk can start a rest break from the micro-break window when the rest-break timer is enabled.
  - Qt now adds the button and calls `force_break(BREAK_ID_REST_BREAK, BreakHint::Normal)`.
- [x] Keep micro-break labels stable after the first layout.
  - Gtk fixes the label size after first update to avoid resize/recenter churn.
  - Qt now fixes the label minimum size after the first update and recenters once.
- [x] Restrict rest-break exercises to the primary screen and honor `BREAK_FLAGS_NO_EXERCISES`.
  - Gtk shows exercises only on the primary head and falls back to the info panel otherwise.
  - Qt now checks both `BREAK_FLAGS_NO_EXERCISES` and primary-screen status.
- [x] Match rest-break recentering behavior when switching from exercises to the info panel.
  - Gtk preserves position in non-blocking primary-screen mode by moving relative to size delta.
  - Qt now adjusts by the size delta in non-blocking primary-screen mode and centers otherwise.
- [x] Review prelude window behavior on platforms where windows cannot be positioned.
  - Gtk has an alignment/input-shape fallback.
  - Qt now uses a transparent full-screen fallback, aligns the visible frame, and masks input to the frame region.

## Preferences and Common UI

- [x] Add the Qt commonui `Entry` preference widget.
  - Gtk has `EntryWidget` and `Builder` handles `ui::prefwidgets::Entry`.
  - Qt now has `EntryWidget` and `Builder` handles `ui::prefwidgets::Entry`.
- [x] Fix focus quiet-mode override in `PreferencesDialog`.
  - Qt implements `eventFilter()` but does not install it on the dialog or application.
  - Qt now installs the filter on the dialog and handles focus/window activation changes like Gtk focus in/out.
- [x] Match platform-specific preference pages.
  - Gtk hides General and Applet pages on macOS.
  - Qt now hides Applet page on macOS and keeps the Status Window page.
- [x] Implement Windows autostart in Qt preferences.
  - Gtk writes/removes the Run registry value in `GeneralPreferencePanel::on_autostart_toggled()`.
  - Qt now writes/removes the same Run registry value in `GeneralUiPreferencesPanel::on_autostart_toggled()`.
- [x] Add Windows light/dark mode preference to Qt.
  - Gtk exposes `GUIConfig::light_dark_mode()` on Windows.
  - Qt now wires the same setting into General preferences and `ToolkitWindows`.
- [x] Add icon theme selection to Qt preferences.
  - Gtk scans image theme directories and writes `GUIConfig::icon_theme()`.
  - Qt now implements the same behavior with `std::filesystem`.
- [x] Improve language selection parity.
  - Gtk shows current-locale and native-language names, sorts with locale collation, and disables rows when fonts are unavailable.
  - Qt now uses the same shared locale data, current/native columns, locale-aware sorting, and font support checks.
- [x] Add the tray-icon explanatory tooltip in Qt preferences.
  - Gtk warns that some desktop environments do not show tray icons.
  - Qt now shows the same warning as a tooltip on the tray icon checkbox.
- [x] Review `TimerBoxPreferencesPanel` sensitivity behavior.
  - Gtk explicitly unchecks and disables the main-window checkbox when all timers are hidden.
  - Qt now unchecks and disables the main-window checkbox when all timers are hidden.
- [x] Polish sound preferences behavior.
  - Gtk has a "No sounds / Play sounds" combo that disables the sound-event UI.
  - Qt now disables the event group, volume, and mute controls when sounds are disabled.
- [x] Add sound file chooser filters and preview playback in Qt.
  - Gtk filters wave files and provides a play button inside the file chooser.
- [x] Sync selected sound file state in Qt when the selected event/theme changes.
  - Gtk updates the chooser to the current event's filename; Qt opens a new file dialog only from the current stored file path and does not update visible filename state.

## Statistics

- [x] Start the statistics refresh timer when opening the Qt Statistics dialog.
  - Qt `Toolkit::show_statistics()` now calls `StatisticsDialog::run()` when the dialog is created.
- [x] Call `statistics->update()` before initial display.
  - Gtk does this in the constructor.
- [x] Add the Activity statistics tab to Qt.
  - Gtk shows mouse usage, mouse movement, effective movement, clicks, and keystrokes on non-macOS platforms.
  - Qt now has a real activity page wired to `activity_labels`.
- [x] Add "Delete all statistics history" to the Qt UI and implement it.
  - Qt now adds the delete button, removes the duplicate calendar insertion, and implements the handler.
- [x] Implement Qt `stream_distance()`.
  - Gtk converts pixels to meters using primary monitor dimensions.
  - Qt now converts using the primary screen physical width and geometry.

## Windows-Specific Qt Gaps

- [ ] Register for Windows session and device notifications.
  - Gtk calls `WTSRegisterSessionNotification()` and `RegisterDeviceNotification()` on the main window handle.
  - Qt installs a native event filter but does not register an event window, so some messages may never arrive.
- [ ] Implement `ToolkitWindows::get_event_hwnd()`.
  - Qt currently returns `0`.
- [x] Handle `WM_SETTINGCHANGE` for automatic light/dark mode.
  - Gtk updates dark mode when Windows reports `ImmersiveColorSet`.
  - Qt now reapplies Auto mode when Windows reports `ImmersiveColorSet`.
- [ ] Handle `WM_DEVICECHANGE` display notifications.
  - Gtk forwards display changes to GTK's display-change window; Qt currently logs `WM_DISPLAYCHANGE` only.
- [ ] Restore tray icon fallback on release.
  - Gtk enables the tray icon if the main window is not visible when a hold is released.
  - Qt has this logic inside `#if 0`.
- [ ] Implement desktop snapshot support for `BlockMode::All`.
  - Qt `ToolkitWindows::get_desktop_image()` returns an empty pixmap, so the block window cannot use the desktop background.
- [ ] Port Windows break-window focus/topmost refresh behavior.
  - Gtk repeatedly enforces topmost, handles force-focus settings, and avoids covering tooltips.

## Dialogs and Secondary UI

- [x] Complete About dialog metadata.
  - Gtk shows authors, translator credits, website, logo, copyright, and version.
  - Qt now uses the shared credits metadata and shows the website link.
- [x] Review crash reporter localization and focus/default-button behavior.
  - Gtk uses translated strings, focuses the text entry, and lets users inspect/select report attachments.
  - Qt now translates visible strings, adds the missing comments label/hint, focuses the comments text entry, and exposes crash summary/attachment selection.
- [x] Review Exercises dialog/panel sizing and action placement.
  - Gtk constrains exercise image/description sizes and uses the dialog action area in standalone mode.
  - Qt now constrains the exercise image/description widgets and moves standalone player controls into the dialog button box.

## Packaging and Build Follow-Up

- [x] Compare Qt and Gtk distribution scripts for missing macOS bundle setup.
  - Gtk macOS packaging has helper scripts and environment setup for bundled resources.
  - Qt now configures its `Info.plist` and uses the correct DMG resource paths before `macdeployqt` runs.
- [x] Compare Windows installer helper coverage.
  - Gtk has additional helper sources such as `ChangeAutorun.c` and SBOM generation; Qt has a different installer path that should be checked before release.
  - Qt now carries the SBOM install script, passes Crashpad/DBus/SBOM/MSVC install flags, resolves Qt plugin dependencies, and emits Crashpad/SBOM installer entries from the generated install paths.
