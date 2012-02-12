#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct client {
    // Prev and next client
    struct client *next;
    struct client *prev;

    // The window
    xcb_window_t win;
} client;

xcb_connection_t * xconn;
xcb_screen_t * screen;
client* head;
client* current;
uint32_t sh;
uint32_t sw;
uint8_t default_depth = 24;

void die( const char* e );
void sigchld( int unused );
void setup();
void start();
void add_window( xcb_window_t w );
void remove_window( xcb_window_t w );
void tile();
void update();

// xcb_generic_event_t handlers
void keypress( xcb_generic_event_t *e );
void maprequest( xcb_generic_event_t *e );
void destroynotify( xcb_generic_event_t *e );
void configurenotify( xcb_generic_event_t *e );
void configurerequest( xcb_generic_event_t *e );

void (*events[256])( xcb_generic_event_t * ) = {
    [XCB_KEY_PRESS] = keypress,
    [XCB_CONFIGURE_REQUEST] = configurerequest,
    [XCB_DESTROY_NOTIFY] = destroynotify,
    [XCB_CONFIGURE_NOTIFY] = configurenotify,
    [XCB_MAP_REQUEST] = maprequest
};

void die( const char* e ) {
    fprintf( stderr, "yawn: %s\n", e );
    exit( 1 );
}

void sigchld( int unused ) {
	if( signal( SIGCHLD, sigchld ) == SIG_ERR ) { // re-register for next time
		die( "Can't install SIGCHLD handler" );
    }
    
	while( 0 < waitpid( -1, NULL, WNOHANG ) );
}

void keypress( xcb_generic_event_t* e ) {
    printf( "yawn: keypress\n" );
}

#define CLIENT_SELECT_INPUT_EVENT_MASK ( XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_FOCUS_CHANGE )

void client_manage( xcb_window_t w ) {
    const uint32_t select_input_val[] = { CLIENT_SELECT_INPUT_EVENT_MASK };
    xcb_map_window( xconn, w );
    xcb_change_window_attributes( xconn, w, XCB_CW_EVENT_MASK, select_input_val );
    xcb_configure_window(xconn, w, XCB_CONFIG_WINDOW_STACK_MODE, (uint32_t[]) { XCB_STACK_MODE_ABOVE } );
}

void maprequest( xcb_generic_event_t* e ) {
    printf( "yawn: maprequest\n" );

    xcb_map_request_event_t* ev = (xcb_map_request_event_t*)e;
    add_window( ev->window );
    client_manage( ev->window );
    tile();
    update();
}

void destroynotify( xcb_generic_event_t* e ) {
    printf( "yawn: destroynotify\n" );
    xcb_destroy_notify_event_t* ev = (xcb_destroy_notify_event_t*)e;
    remove_window( ev->window );
    tile();
    update();
}

void configurenotify( xcb_generic_event_t* e ) {
    printf( "yawn: configurenotify\n" );
}

void configurerequest( xcb_generic_event_t * e ) {
    xcb_configure_request_event_t * ev = (xcb_configure_request_event_t*)e;

    printf( "yawn: configurerequest\n" );

    uint16_t config_win_mask = 0;
    uint32_t config_win_vals[ 7 ];

    uint16_t masks[ 7 ] = { XCB_CONFIG_WINDOW_X, XCB_CONFIG_WINDOW_Y, XCB_CONFIG_WINDOW_WIDTH, XCB_CONFIG_WINDOW_HEIGHT, XCB_CONFIG_WINDOW_BORDER_WIDTH, XCB_CONFIG_WINDOW_SIBLING, XCB_CONFIG_WINDOW_STACK_MODE };

    uint32_t values[ 7 ] = { ev->x, ev->y, ev->width, ev->height, ev->border_width, ev->sibling, ev->stack_mode };

    unsigned int i;
    unsigned int count = 0;
    for ( i = 0; i < 7; ++i ) {
        uint16_t mask = masks[ i ];
        if ( ev->value_mask & mask ) {
            config_win_mask |= mask;
            config_win_vals[ count++ ] = values[ i ];
        }
    }

    // printf( "value mask: %i %i %i %i %i\n", ev->value_mask, ev->x, ev->y, ev->width, ev->height );

    // xcb_configure_window( xconn, ev->window, ev->value_mask, values );
    xcb_configure_window( xconn, ev->window, config_win_mask, config_win_vals );
    xcb_flush( xconn );
}

void add_window( xcb_window_t w ) {
    client * c, * tail;

    if ( !( c = (client*)malloc( 1 * sizeof( client ) ) ) ) {
        die( "yawn: Malloc error on add_window" );
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

void remove_window( xcb_window_t w ) {
    client *c;

    for(c=head;c;c=c->next) {
        if(c->win == w) {
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

            free(c);
            return;
        }
    }
}

void tile() {
    int n = 0, x = 0, y = 0;
    client* c;

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
        // XMoveResizeWindow(dis,c->win,master_size,y,sw-master_size-2,(sh/n)-2);
        printf( "yawn: resizing current %i %i %i %i\n", x, y, w, h );
        uint32_t values[] = { x, y, w, h };
        // uint32_t values[] = { 0, 0, 600, 800 };
        xcb_configure_window( xconn, c->win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values );
        x += sw / n;
    }
    xcb_flush( xconn );
}

void update() {
    if ( current != NULL ) {
        printf( "yawn: raising current\n" );
        const static uint32_t values[] = { XCB_STACK_MODE_ABOVE };
        // XSetInputFocus( display, current->win, RevertToParent, CurrentTime );
        xcb_configure_window( xconn, current->win, XCB_CONFIG_WINDOW_STACK_MODE, values );
        xcb_flush( xconn );
    }
}


int default_screen;
xcb_query_tree_cookie_t tree_c;


xcb_screen_t *screen_of_display( xcb_connection_t *c, int screen ) {
    xcb_screen_iterator_t iter;

    iter = xcb_setup_roots_iterator (xcb_get_setup (c) );
    for ( ; iter.rem; --screen, xcb_screen_next (&iter) ) {
        if ( screen == 0 ) {
            return iter.data;
        }
    }

    return NULL;
}

void setup() {
    printf( "yawn: setup\n" );

    // Install a signal
    sigchld( 0 );

    xconn = xcb_connect( NULL, &default_screen );
    screen = screen_of_display( xconn, default_screen );

    sh = screen->height_in_pixels;
    sw = screen->width_in_pixels;
    
    // List of client
    head = NULL;
    current = NULL;
    
    xcb_change_window_attributes( xconn, screen->root, XCB_CW_EVENT_MASK, (const uint32_t []) { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY } );

    xcb_flush( xconn );
}

void start() {
    printf( "yawn: start\n" );
    xcb_generic_event_t* event;
    while( ( event = xcb_wait_for_event( xconn ) ) != NULL ) {
        uint32_t type = event->response_type & ~0x80;
        printf( "yawn: event %i\n", type );
        if ( events[ type ] != NULL ) {
            events[ type ]( event );
        }
        free( event );
        xcb_flush( xconn );
    }
}

void teardown() {
    printf( "yawn: tear down\n" );
    xcb_flush( xconn );
    xcb_disconnect( xconn );
}

int main( int argc, char** argv ) {
    setup();
    start();
    teardown();

    return 0;
}
