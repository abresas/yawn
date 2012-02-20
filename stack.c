#include <stdlib.h>
#include "stack.h"
#include "globals.h"
#include "yawn.h"

void stack_add( xcb_window_t w ) {
    struct client_t *c, *tail;

    if ( !( c = (struct client_t*)malloc( 1 * sizeof (struct client_t) ) ) ) {
        die( "yawn: Malloc error on stack_add" );
    }

    if ( head == NULL ) {
        c->next = NULL;
        c->prev = NULL;
        c->win = w;
        head = c;
    }
    else {
        for ( tail = head; tail->next; tail = tail->next );

        c->next = NULL;
        c->prev = tail;
        c->win = w;

        tail->next = c;
    }

    current = c;
}

void stack_remove( xcb_window_t w ) {
    struct client_t * c = stack_get_client( w );
    if ( !c ) {
        return;
    }

    if(c->prev == NULL && c->next == NULL) {
        free(head);
        head = NULL;
        current = NULL;
        return;
    }

    if(c->prev == NULL) {
        head = c->next;
        c->next->prev = NULL;
        current = c->next;
    }
    else if(c->next == NULL) {
        c->prev->next = NULL;
        current = c->prev;
    }
    else {
        c->prev->next = c->next;
        c->next->prev = c->prev;
        current = c->prev;
    }

    free( c );
    return;
}

void stack_tile() {
    int n = 0, x = 0, y = 0;
    struct client_t *c;

    if ( current == NULL ) {
        return; // nothing to do here
    }
    
    for ( c = head; c; c = c->next ) {
        ++n;
    }

    printf( "size %i %i %i\n", n, sw, sh );
    int w = sw / n;
    int h = sh - 2;

    for ( c = head; c; c = c->next) {
        printf( "yawn: resizing current %i %i %i %i\n", x, y, w, h );
        uint32_t values[] = { x, y, w, h };
        xcb_configure_window( xconn, c->win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values );
        x += sw / n;
    }
    xcb_flush( xconn );
}

struct client_t * stack_get_client( xcb_window_t w ) {
    struct client_t * c;
    for ( c = head; c; c = c->next ) {
        if ( c->win == w ) {
            return c;
        }
    }
    return NULL;
}
