#ifndef WINDOW_H
#define WINDOW_H

#include <xcb/xcb.h>
#include "keyboard.h"

void window_grabkey( xcb_window_t win, struct keybinding_t kb );
void window_grabkeys(xcb_window_t win, struct keybinding_t * keybindings );
void window_current_update();

#endif

