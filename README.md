# Workrave 1.10

Please visit http://www.workrave.org for more information.

## Install

- Windows: download from http://www.workrave.org/download
- Ubuntu: install with the "Ubuntu Software" application.
- Linux latest version:  
  Add this PPA to your Software Sources  
  `ppa:rob-caelers/workrave`  
  either in the Ubuntu Software application, or from the terminal:
  ```
  sudo add-apt-repository ppa:rob-caelers/workrave  
  sudo apt-get update
  ```


## Build

This document only discusses compilation on Unix (like OSes).  
Information on how to compile Workrave on Windows can be found in
- `build/cmake/README` (native compilation on windows) and
- `build/win32/README` (cross-compilation on Linux)

Read the 'INSTALL' file for more detailed directions on compilation on
Unix and OSX.

Workrave requires that development packages of at least the following
software are installed. The version numbers mentioned have been tested
during development.

- GLib (2.16.0)
- GLibmm (2.19.3)
- Gtk (2.16.0)
- Gtkmm (2.16.0)
- Atk (1.20)
- Pango (1.22.0)
- Pangomm (2.14.0)
- Cairo (1.2.4)
- Cairomm (1.6.4)
- DBus (1.0.2)
- DBus-Glib (0.78)
- GConf (2.13.5)
- GConfmm (2.22.0)
- GDome
- GStreamer (0.10.10)
- Libsigc++ (2.0.2)
- Autoconf with Autoconf Macro Archive (2012.04.04)

Optionally, the following packages are required for gnome support.

- ORbit (2.14.10)
- Bonobo (2.15.0)
- panel-applet (2.19.3)

For OS X, the following steps will install sufficient packages
1. Install [Homebrew](https://brew.sh/)
2. `brew install gettext intltool gobject-introspection autoconf-archive gtk+ gtk-mac-integration gtkmm`
3. `brew link --force gettext`
