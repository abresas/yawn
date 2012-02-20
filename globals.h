#ifndef GLOBALS_H
#define GLOBALS_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include "client.h"
#include "desktop.h"

extern int current_desktop;
extern struct desktop_t desktops[10];
extern xcb_connection_t * xconn;
extern xcb_screen_t * screen;
extern xcb_timestamp_t timestamp;
extern struct client_t *head;
extern struct client_t *current;
extern xcb_key_symbols_t *symbols;
extern uint32_t sh;
extern uint32_t sw;
extern uint8_t default_depth;
extern int bindingsCount;
extern int default_screen;
extern xcb_query_tree_cookie_t tree_c;
extern void (*events[256])( xcb_generic_event_t * );
extern int modifierAsciiToMask[ 256 ];


#endif

