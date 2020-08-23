# suspend-workrave-on-screensaver-activation
This script prevents Workrave breaks when the display is
blank/sleeping/off or the screen is locked.

## Motivation
When the *Reading mode* is on, Workrave keeps asking for breaks
indefinitely even if there is nothing to read on the screen and the user
has stopped all mouse/keyboard activity some time ago. The break sounds
can distract the user from non-computer activity then.

## Requirements
*xfce4-screensaver* must be running. It should be fairly easy to adapt
this script to other screensavers that send D-Bus signals when activated
or deactivated.

The script always works for screen locking. However it requires the
*Activate screensaver when computer is idle* option to be on in
*xfce4-screensaver* settings in order to detect display
blanking/sleeping/off. With this option on the screensaver emits the
necessary `ActiveChanged` signal even if the *Enable Screensaver* option
is off.

## How to install and run
1. Make sure that the script has the executable bit set.
2. Copy the script to */usr/local/bin/*.
3. Add the script into the Autostart list.

Note that the script has to be constantly running and monitoring D-Bus.

## License
Copyright (C) 2020 Igor Kushnir &lt;igorkuo AT Google mail&gt;

License: GPL v3+ (http://www.gnu.org/copyleft/gpl.html)

## Discussion and feedback
The script can be discussed at the original Workrave issue that inspired
it: [#134](https://github.com/rcaelers/workrave/issues/134).
