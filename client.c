#include "client.h"
#include "globals.h"
#include "keyboard.h"
#include "window.h"

void client_manage( xcb_window_t w ) {
    const uint32_t select_input_val[] = { CLIENT_SELECT_INPUT_EVENT_MASK };
    xcb_map_window( xconn, w );
    xcb_change_window_attributes( xconn, w, XCB_CW_EVENT_MASK, select_input_val );
    xcb_configure_window(xconn, w, XCB_CONFIG_WINDOW_STACK_MODE, (uint32_t[]) { XCB_STACK_MODE_ABOVE } );
    window_grabkeys( w, keybindings );
}

int client_kill( int argc, char ** argv ) {
    xcb_destroy_window( xconn, current->win );

    /* Remove this window from the save set since this shouldn't be made visible
     * after a restart anymore. */
    xcb_change_save_set( xconn, XCB_SET_MODE_DELETE, current->win );

    /* Do this last to avoid races with clients. According to ICCCM, clients
     * arent allowed to re-use the window until after this. */
    // xwindow_set_state(c->window, XCB_ICCCM_WM_STATE_WITHDRAWN);

    current->win = XCB_NONE;
    
    return 0;
}
