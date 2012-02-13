yawn - not yawm

Intro
-----

This is an ongoing rewrite of http://github.com/pyknite/catwm into using the XCB library and other tweaks based on my needs.

The XCB code is largely based on the code of awesome window manager, which I higly recommend as your regular window manager.

Goals
-----

* Tiling window manager
* Desktops for grouping windows
* Full screen available for user programs
* Customizable shortcuts
* Minimal and clean code

Changelog
------

Basic code for window management with XCB is done. Windows are always tiled horizontally, getting full height and equal width. 

You can spawn programs with custom shortcuts. Added sample configuration file.

Installation
------------

    $ make
    $ sudo make install

Then edit the file that executes your existing window manager (usually ~/.xinitrc) and set it to execute yawn instead:

    exec /usr/bin/yawn


Test run
--------

You can have two X displays open simultaneously, one managed by your regular window manager, and one managed by yawn. To open a second X display run:

    $ startx -- :2

which will open a display with id :2.

To open a window on that display from within another display, you usually just pass --display=:2, or else you can set the environmental variable DISPLAY

    $ export DISPLAY=:2
