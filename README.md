[![Awesome Humane Tech](https://raw.githubusercontent.com/humanetech-community/awesome-humane-tech/main/humane-tech-badge.svg?sanitize=true)](https://github.com/humanetech-community/awesome-humane-tech)
[![Build Status](https://github.com/rcaelers/workrave/workflows/CI/badge.svg?branch=main)](https://github.com/rcaelers/workrave/actions)

# Workrave 1.11

Please visit https://workrave.org for more information.

## Install

- Windows: download from https://workrave.org/download
- Ubuntu: install with the "Ubuntu Software" application.
- Ubuntu (and derivatives) Linux latest version:
  Add this PPA to your Software Sources
  `ppa:rob-caelers/workrave`
  either in the Ubuntu Software application, or from the terminal:
  ```
  sudo add-apt-repository ppa:rob-caelers/workrave
  sudo apt-get update
  ```
- Arch Linux users: use `sudo pacman -S workrave` to install from the community repository
- Ubuntu and Debian users: `sudo apt-get install workrave`
- FreeBSD users: `pkg install workrave`

## Troubleshooting

### Show timers applet in Cinnamon

To make timers visible you need to explicitly add Workrave applet to a panel. In the other case only workrave icon is shown.

## Technical Information

Have a look at the [contrib](./contrib) directory to get a little insight into the different scripting possibilities!

### Unix/Linux

- Workrave uses [dconf](https://wiki.gnome.org/Projects/dconf) to store its configuration.
  `dconf-editor` can be used to explore and manipulate the values - be careful!
- Workrave can receive [dbus](https://www.freedesktop.org/wiki/Software/dbus/) signals.
  Explore by having a look at the [example python script](./backend/src/dbus-example.py) or viewing in `qdbusviewer` (part of the `qttools5-dev-tools` package in Ubuntu).

In both of these, workrave is found under the `org.workrave` key.

### Changelog

Workrave keeps an overview of user-visible changes under [NEWS](NEWS).
