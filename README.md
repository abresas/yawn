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

v0.1.0
Basic code for window management with XCB is done. Windows are always tiled horizontally, getting full height and equal width. 

v0.1.1
You can spawn programs with custom shortcuts. Added sample configuration file.

v0.1.2
Desktops for grouping windows.

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

License
-------

Copyright (C) 2012 Aleksis Brezas

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
