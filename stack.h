#ifndef STACK_H
#define STACK_H

#include <xcb/xcb.h>
#include "client.h"

void stack_add( xcb_window_t w );
void stack_remove( xcb_window_t w );
void stack_tile();

#endif

