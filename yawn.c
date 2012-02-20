#include <unistd.h> // execvp
#include "yawn.h"
#include "action.h"
#include "client.h"
#include "keyboard.h"
#include "window.h"
#include "configuration.h"
#include "globals.h"

int current_desktop;
struct desktop_t desktops[10];
xcb_connection_t * xconn;
xcb_screen_t * screen;
xcb_timestamp_t timestamp;
struct client_t *head;
struct client_t *current;
xcb_key_symbols_t *symbols;
uint32_t sh;
uint32_t sw;
uint8_t default_depth;
int bindingsCount;
int default_screen;
xcb_query_tree_cookie_t tree_c;

uint8_t default_depth = 24;
int bindingsCount = 1;

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

    symbols = xcb_key_symbols_alloc( xconn );

    sh = screen->height_in_pixels;
    sw = screen->width_in_pixels;
    
    head = NULL;
    current = NULL;
   
    xcb_change_window_attributes( xconn, screen->root, XCB_CW_EVENT_MASK, (const uint32_t []) { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_KEY_PRESS } );

    xcb_flush( xconn );

    // Set up all desktop
    int i;
    for ( i = 0; i < 10; ++i ) {
        desktops[ i ].head = head;
        desktops[ i ].current = current;
    }

    char * argv[] = { "0" };
    desktop_change( 1, argv );
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

int quit( int argc, char ** argv ) {
    exit( 0 );
}

int spawn( int argc, char ** argv ) {
    printf( "spawning %i %s %s\n", argc, argv[ 0 ], argv[ 1 ] );
    argv[ argc ] = NULL;
    if ( fork() == 0 ) {
        execvp( argv[ 0 ], argv + 1 );
        exit( 0 );
    }
    return 0;
}

int main( int argc, char** argv ) {
    setup();
    configuration_read( "yawn.conf" );

    start();
    teardown();

    return 0;
}
