#include <stdio.h>
#include "globals.h"
#include "desktop.h"
#include "client.h"
#include "stack.h"
#include "window.h"

int desktop_change( int argc, char ** argv ) {
    int i;
    struct client_t *c;

    sscanf( argv[ 0 ], "%s", &i );

    if ( i == current_desktop ) {
        return 0;
    }

    if ( head != NULL ) {
        for( c = head; c; c= c->next ) {
            xcb_unmap_window( xconn, c->win );
        }
    }

    desktop_save( current_desktop );
    desktop_select( i );

    if ( head != NULL ) {
        for( c = head; c; c = c->next ) {
            xcb_map_window( xconn, c->win );
        }
    }

    stack_tile();
    window_current_update();

    return 0;
}

void desktop_save( int i ) {
    desktops[ i ].head = head;
    desktops[ i ].current = current;
}

void desktop_select( int i ) {
    head = desktops[ i ].head;
    current = desktops[ i ].current;
    current_desktop = i;
}
