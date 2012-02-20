#include <stdio.h>
#include "event.h"
#include "globals.h"

void (*events[256])( xcb_generic_event_t * ) = {
    [XCB_KEY_PRESS] = event_keypress,
    [XCB_CONFIGURE_REQUEST] = event_configurerequest,
    [XCB_DESTROY_NOTIFY] = event_destroynotify,
    [XCB_CONFIGURE_NOTIFY] = event_configurenotify,
    [XCB_MAP_REQUEST] = event_maprequest,
    [XCB_ENTER_NOTIFY] = event_enternotify
};

void event_keypress( xcb_generic_event_t* e ) {
    xcb_key_press_event_t * ev = (xcb_key_press_event_t*)e;
    timestamp = ev->time;
    xcb_keysym_t keysym = xcb_key_symbols_get_keysym( symbols, ev->detail, 0 );
    printf( "yawn: keypress %i %i %i\n", ev->detail, keysym, ev->state );
    int i = 0;
    printf( "keybinding count: %i\n", bindingsCount );
    for ( i = 0; i < bindingsCount; ++i ) {
        struct keybinding_t k = keybindings[ i ];
        printf( "key sym: %i state: %i\n", k.key, k.state );
        if ( k.key == keysym && ev->state & k.state ) {
            printf( "found key binding! calling callback! %i %s\n", k.argc, k.argv[ 0 ] );
            k.callback( k.argc, k.argv );
        }
    }
}

void event_maprequest( xcb_generic_event_t* e ) {
    printf( "yawn: maprequest\n" );

    xcb_map_request_event_t* ev = (xcb_map_request_event_t*)e;
    stack_add( ev->window );
    client_manage( ev->window );
    stack_tile();
    window_current_update();
}

void event_destroynotify( xcb_generic_event_t* e ) {
    printf( "yawn: destroynotify\n" );
    xcb_destroy_notify_event_t* ev = (xcb_destroy_notify_event_t*)e;
    stack_remove( ev->window );
    stack_tile();
    window_current_update();
}

void event_configurenotify( xcb_generic_event_t* e ) {
    printf( "yawn: configurenotify\n" );
}

void event_configurerequest( xcb_generic_event_t * e ) {
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

    xcb_configure_window( xconn, ev->window, config_win_mask, config_win_vals );
    xcb_flush( xconn );
}

void event_enternotify( xcb_generic_event_t * e ) {
    printf( "yawn: enternotify\n" );
    xcb_enter_notify_event_t * ev = (xcb_enter_notify_event_t*)e;
    timestamp = ev->time;
}
