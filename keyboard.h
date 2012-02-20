#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include "action.h"

struct keybinding_t {
    uint16_t state;
    xcb_keysym_t key;
    int (*callback)(int argc, char ** argv);
    int argc;
    char ** argv;
};

struct keybinding_t keybindings[ 100 ];

xcb_keysym_t keyboard_get_keysym( char * key );
uint16_t keyboard_combination_get_state( char ** key_combination_pointer );
int keyboard_bind( int argCount, char ** words );

#endif

