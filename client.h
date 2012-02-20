#ifndef CLIENT_H
#define CLIENT_H

#include <xcb/xcb.h>

#define CLIENT_SELECT_INPUT_EVENT_MASK ( XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_KEY_PRESS )

struct client_t {
    struct client_t *next;
    struct client_t *prev;
    xcb_window_t win;
};

void client_manage( xcb_window_t w );
struct client_t *client_get_by_window( xcb_window_t w );
int client_kill( int argc, char ** argv );

#endif
