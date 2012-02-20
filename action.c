#include <string.h>
#include "action.h"
#include "yawn.h"
#include "keyboard.h"
#include "client.h"
#include "desktop.h"

int (*get_action(char * name ))(int , char **) {
    if ( !strcmp( name, "bind" ) ) {
        return &keyboard_bind;
    }
    else if ( !strcmp( name, "change_desktop" ) ) {
        return &desktop_change;
    }
    else if ( !strcmp( name, "quit" ) ) {
        return &quit;
    }
    else if ( !strcmp( name, "kill_client" ) ) {
        return &client_kill;
    }
    else {
        return &spawn;
    }
}
