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
uint8_t default_depth = 64;

void die( const char* e );
void sigchld( int unused );
void setup();
void start();
void add_window( xcb_window_t w );
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
#define FRAME_SELECT_INPUT_EVENT_MASK ( XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT )

void
client_manage(xcb_window_t w )
{
    xcb_get_geometry_cookie_t geom_c = xcb_get_geometry_unchecked( xconn, w );
    xcb_get_geometry_reply_t * wgeom = xcb_get_geometry_reply(xconn, geom_c, NULL);
    const uint32_t select_input_val[] = { CLIENT_SELECT_INPUT_EVENT_MASK };

    /* Make sure the window is automatically mapped on exit. */
    xcb_change_save_set( xconn, XCB_SET_MODE_INSERT, w );

    uint32_t frameid = xcb_generate_id( xconn );
    xcb_create_window( xconn, default_depth, frameid, screen->root,
                      wgeom->x, wgeom->y, wgeom->width, wgeom->height,
                      wgeom->border_width, XCB_COPY_FROM_PARENT, screen->root_visual,
                      XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_BIT_GRAVITY
                      | XCB_CW_WIN_GRAVITY | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK
                      | XCB_CW_COLORMAP,
                      (const uint32_t [])
                      {
                          screen->black_pixel,
                          screen->black_pixel,
                          XCB_GRAVITY_NORTH_WEST,
                          XCB_GRAVITY_NORTH_WEST,
                          1,
                          FRAME_SELECT_INPUT_EVENT_MASK,
                          screen->default_colormap
                      });

    xcb_reparent_window( xconn, w, frameid, 0,  0);
    xcb_map_window( xconn, w );
    xcb_map_window( xconn, frameid );

    /* Do this now so that we don't get any events for the above
     * (Else, reparent could cause an UnmapNotify) */
    xcb_change_window_attributes( xconn, w, XCB_CW_EVENT_MASK, select_input_val );

    /* The frame window gets the border, not the real client window */
    xcb_configure_window( xconn, w,
                         XCB_CONFIG_WINDOW_BORDER_WIDTH,
                         (uint32_t[]) { 0 });

    /* Move this window to the bottom of the stack. Without this we would force
     * other windows which will be above this one to redraw themselves because
     * this window occludes them for a tiny moment. The next stack_refresh()
     * will fix this up and move the window to its correct place. */

    xcb_configure_window(xconn, w,
                         XCB_CONFIG_WINDOW_STACK_MODE,
                         (uint32_t[]) { XCB_STACK_MODE_ABOVE });
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
    client * c;

    if ( !( c = (client*)malloc( 1 * sizeof( client ) ) ) ) {
        die( "yawn: Malloc error on add_window" );
    }

    c->next = NULL;
    c->prev = NULL;
    c->win = w;
    head = c;

    current = c;
}

void tile() {
    if ( current != NULL ) {
        printf( "yawn: resizing current %i %i\n", sw - 2, sh - 2 );
        uint32_t values[] = { 0, 0, sw - 2, sh - 2 };
        // uint32_t values[] = { 0, 0, 600, 800 };
        xcb_configure_window( xconn, current->win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values );
        xcb_flush( xconn );
    }
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


xcb_screen_t *screen_of_display (xcb_connection_t *c,
                                 int               screen)
{
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator (xcb_get_setup (c));
  for (; iter.rem; --screen, xcb_screen_next (&iter))
    if (screen == 0)
      return iter.data;

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
