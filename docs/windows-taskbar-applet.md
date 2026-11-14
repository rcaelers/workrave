# Windows taskbar applet

The taskbar applet shows Workrave timers in unused space on the primary Windows
taskbar.

## Enable and configure

Open **Preferences > User interface > Applet > Taskbar** and enable
**Show timers in taskbar**.

The **Applet** tab also lets you choose how timers are grouped, how often grouped
timers cycle, and which timers are shown. Existing applet settings are retained.

## Display and interaction

- Timers are stacked vertically, with the same break icons as the main window.
- The display follows Workrave’s Light, Dark or Auto appearance setting.
  Progress bars use blue for active time, green for idle time and orange for
  overdue time.
- Suspended and quiet modes have markers beside the times and labels in the tooltip.
- Left-click opens the status window.
- Middle-click cycles grouped timers.
- Right-click opens Workrave's menu.
- Hover over the applet to see timer names and the current mode.

Light mode uses black text on a beige background. Dark mode uses a dark
background with light text; text over coloured progress bars stays black.
Auto follows the Windows app theme. Changes take effect without restarting
Workrave. Windows high contrast mode uses the system colours.

## Limitations

The applet supports the primary horizontal Windows 10/11 taskbar. Secondary and
vertical taskbars are not supported, and the applet cannot be dragged.

If there is not enough free space, the applet hides and reappears when space
becomes available. It also reconnects after Windows Explorer restarts.

Custom GTK CSS colours and Qt status-window styles do not apply to the taskbar
applet.
