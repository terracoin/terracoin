
Debian
====================
This directory contains files used to package terracoind/terracoin-qt
for Debian-based Linux systems. If you compile terracoind/terracoin-qt yourself, there are some useful files here.

## terracoin: URI support ##


terracoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install terracoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your terracoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/terracoin128.png` to `/usr/share/pixmaps`

terracoin-qt.protocol (KDE)

