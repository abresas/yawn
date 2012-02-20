#include "keyboard.h"
#include "string.h"
#include "globals.h"

int modifierAsciiToMask[ 256 ] = {
    [ 'A' ] = XCB_MOD_MASK_1,
    [ 'C' ] = XCB_MOD_MASK_CONTROL,
    [ 'M' ] = XCB_MOD_MASK_4
};

xcb_keysym_t keyboard_get_keysym( char * key ) {
    if ( strlen( key ) == 1 ) {
        return key[ 0 ];
    }
    if ( strcmp( key, "return" ) == 0 ) {
        return XK_Return;
    }
    return 0;
}

uint16_t keyboard_combination_get_state( char ** key_combination_pointer ) {
    int i;
    uint16_t state = 0;

    char * keyCombination = *key_combination_pointer;

    for ( i = 0; i < strlen( keyCombination ); i += 2 ) {
        if ( keyCombination[ i + 1 ] != '-' ) {
            break;
        }
        state |= modifierAsciiToMask[ (int)keyCombination[ i ] ];
    }

    if ( i == 0 ) {
        state = XCB_MOD_MASK_ANY;
    }

    *key_combination_pointer = keyCombination + i;

    return state;
}

int keyboard_bind( int argCount, char ** words ) {
    int argc = argCount - 2;

    char * keyCombination = words[ 0 ],
         * command = words[ 1 ],
         ** argv = ( char** )malloc( argc * sizeof( char * ) );

    string_array_ncopy( argv, words + 2, argc );

    struct keybinding_t k = {
        keyboard_combination_get_state( &keyCombination ),
        keyboard_get_keysym( keyCombination ),
        get_action( command ),
        argc,
        argv
    };

    keybindings[ bindingsCount++ ] = k;

    return 0;
}

