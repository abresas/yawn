#ifndef EVENT_H
#define EVENT_H

#include <xcb/xcb.h>
#include "client.h"
#include "keyboard.h"
#include "stack.h"
#include "window.h"

void event_keypress( xcb_generic_event_t *e );
void event_maprequest( xcb_generic_event_t *e );
void event_destroynotify( xcb_generic_event_t *e );
void event_configurenotify( xcb_generic_event_t *e );
void event_configurerequest( xcb_generic_event_t *e );
void event_enternotify( xcb_generic_event_t *e );

#endif
