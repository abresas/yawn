#include <stdio.h>
#include "window.h"
#include "globals.h"

void window_grabkey( xcb_window_t win, struct keybinding_t kb ) {
    if ( !kb.key ) {
        return; // TODO: temporary fix
    }
    xcb_keycode_t * keycodes = xcb_key_symbols_get_keycode( symbols, kb.key );
    if ( keycodes ) {
        for( xcb_keycode_t *kc = keycodes; *kc; kc++ ) {
            printf( "grabbing key: %i %i\n", *kc, kb.state );
            xcb_grab_key( xconn, 1, win, kb.state, *kc, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
        }
        // p_delete(&keycodes);
    }
}

void window_grabkeys(xcb_window_t win, struct keybinding_t * keybindings ) {
    xcb_ungrab_key( xconn, XCB_GRAB_ANY, win, XCB_BUTTON_MASK_ANY);

    for ( int i = 0; i < bindingsCount; ++i ) {
        struct keybinding_t kb = keybindings[ i ];
        window_grabkey( win, kb );
    }
}

void window_current_update() {
    if ( current != NULL ) {
        printf( "yawn: raising current\n" );
        const static uint32_t values[] = { XCB_STACK_MODE_ABOVE };
        xcb_set_input_focus( xconn, XCB_INPUT_FOCUS_PARENT, current->win, timestamp );
        xcb_configure_window( xconn, current->win, XCB_CONFIG_WINDOW_STACK_MODE, values );
        xcb_flush( xconn );
    }
}
