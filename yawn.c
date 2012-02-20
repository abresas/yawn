#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h> // execvp
#include <string.h> // strchr

typedef struct client {
    struct client *next;
    struct client *prev;
    xcb_window_t win;
} client;

typedef struct desktop desktop;
struct desktop{
    client *head;
    client *current;
};

xcb_connection_t * xconn;
xcb_screen_t * screen;
xcb_timestamp_t timestamp;
client* head;
client* current;
xcb_key_symbols_t* symbols;
uint32_t sh;
uint32_t sw;
uint8_t default_depth = 24;
int bindingsCount = 1;

void die( const char* e );
void sigchld( int unused );
void setup();
void start();
void add_window( xcb_window_t w );
void remove_window( xcb_window_t w );
void tile();
void update();

void keypress( xcb_generic_event_t *e );
void maprequest( xcb_generic_event_t *e );
void destroynotify( xcb_generic_event_t *e );
void configurenotify( xcb_generic_event_t *e );
void configurerequest( xcb_generic_event_t *e );
void enternotify( xcb_generic_event_t *e );

void (*events[256])( xcb_generic_event_t * ) = {
    [XCB_KEY_PRESS] = keypress,
    [XCB_CONFIGURE_REQUEST] = configurerequest,
    [XCB_DESTROY_NOTIFY] = destroynotify,
    [XCB_CONFIGURE_NOTIFY] = configurenotify,
    [XCB_MAP_REQUEST] = maprequest,
    [XCB_ENTER_NOTIFY] = enternotify
};

static int current_desktop;
static desktop desktops[10];

void save_desktop( int i );
void select_desktop( int i );

int change_desktop( int argc, char ** argv ) {
    int i;
    client *c;

    sscanf( argv[ 0 ], "%s", &i );

    if ( i == current_desktop ) {
        return 0;
    }

    if ( head != NULL ) {
        for( c = head; c; c= c->next ) {
            xcb_unmap_window( xconn, c->win );
        }
    }

    save_desktop( current_desktop );
    select_desktop( i );

    if ( head != NULL ) {
        for( c = head; c; c = c->next ) {
            xcb_map_window( xconn, c->win );
        }
    }

    tile();
    update();

    return 0;
}

void save_desktop( int i ) {
    desktops[ i ].head = head;
    desktops[ i ].current = current;
}

void select_desktop( int i ) {
    head = desktops[ i ].head;
    current = desktops[ i ].current;
    current_desktop = i;
}

void die( const char* e ) {
    fprintf( stderr, "yawn: %s\n", e );
    exit( 1 );
}

int quit( int argc, char ** argv ) {
    exit( 0 );
}

void sigchld( int unused ) {
	if( signal( SIGCHLD, sigchld ) == SIG_ERR ) { // re-register for next time
		die( "Can't install SIGCHLD handler" );
    }
    
	while( 0 < waitpid( -1, NULL, WNOHANG ) );
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

struct callback_t {
    int (*callback)(int argc, char ** argv);
    char * name;
};

struct callback_t callbacks[] = {
    { spawn, "spawn" }
};

int modifierAsciiToMask[ 256 ] = {
    [ 'A' ] = XCB_MOD_MASK_1,
    [ 'C' ] = XCB_MOD_MASK_CONTROL,
    [ 'M' ] = XCB_MOD_MASK_4
};

struct keybinding {
    uint16_t state;
    xcb_keysym_t key;
    int (*callback)(int argc, char ** argv);
    int argc;
    char ** argv;
};

struct keybinding keybindings[ 100 ];

void keypress( xcb_generic_event_t* e ) {
    xcb_key_press_event_t * ev = (xcb_key_press_event_t*)e;
    timestamp = ev->time;
    xcb_keysym_t keysym = xcb_key_symbols_get_keysym( symbols, ev->detail, 0 );
    printf( "yawn: keypress %i %i %i\n", ev->detail, keysym, ev->state );
    int i = 0;
    printf( "keybinding count: %i\n", bindingsCount );
    for ( i = 0; i < bindingsCount; ++i ) {
        struct keybinding k = keybindings[ i ];
        printf( "key sym: %i state: %i\n", k.key, k.state );
        if ( k.key == keysym && ev->state & k.state ) {
            printf( "found key binding! calling callback! %i %s\n", k.argc, k.argv[ 0 ] );
            k.callback( k.argc, k.argv );
        }
    }
}

void enternotify( xcb_generic_event_t * e ) {
    printf( "yawn: enternotify\n" );
    xcb_enter_notify_event_t * ev = (xcb_enter_notify_event_t*)e;
    timestamp = ev->time;
}

xcb_keysym_t get_keysym_from_key( char * key ) {
    printf( "get keysym for: %s\n", key );
    if ( strlen( key ) == 1 ) {
        return key[ 0 ];
    }
    if ( strcmp( key, "return" ) == 0 ) {
        return XK_Return;
    }
    return 0;
}

static void window_grabkey( xcb_window_t win, struct keybinding kb ) {
    if ( !kb.key ) {
        printf( "keycodes for %i\n", kb.key );
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

void window_grabkeys(xcb_window_t win, struct keybinding * keybindings ) {
    xcb_ungrab_key( xconn, XCB_GRAB_ANY, win, XCB_BUTTON_MASK_ANY);

    for ( int i = 0; i < bindingsCount; ++i ) {
        struct keybinding kb = keybindings[ i ];
        window_grabkey( win, kb );
    }
}

#define CLIENT_SELECT_INPUT_EVENT_MASK ( XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_KEY_PRESS )

void client_manage( xcb_window_t w ) {
    const uint32_t select_input_val[] = { CLIENT_SELECT_INPUT_EVENT_MASK };
    xcb_map_window( xconn, w );
    xcb_change_window_attributes( xconn, w, XCB_CW_EVENT_MASK, select_input_val );
    xcb_configure_window(xconn, w, XCB_CONFIG_WINDOW_STACK_MODE, (uint32_t[]) { XCB_STACK_MODE_ABOVE } );
    window_grabkeys( w, keybindings );
}


void maprequest( xcb_generic_event_t* e ) {
    printf( "yawn: maprequest\n" );

    xcb_map_request_event_t* ev = (xcb_map_request_event_t*)e;
    add_window( ev->window );
    client_manage( ev->window );
    tile();
    update();
}

client * get_client_by_window( xcb_window_t w ) {
    client * c;
    for ( c = head; c; c = c->next ) {
        if ( c->win == w ) {
            return c;
        }
    }
    return NULL;
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

int kill_client( int argc, char ** argv ) {
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
        printf( "yawn: resizing current %i %i %i %i\n", x, y, w, h );
        uint32_t values[] = { x, y, w, h };
        xcb_configure_window( xconn, c->win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values );
        x += sw / n;
    }
    xcb_flush( xconn );
}

void update() {
    if ( current != NULL ) {
        printf( "yawn: raising current\n" );
        const static uint32_t values[] = { XCB_STACK_MODE_ABOVE };
        xcb_set_input_focus( xconn, XCB_INPUT_FOCUS_PARENT, current->win, timestamp );
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
    change_desktop( 1, argv );
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

typedef enum {
    false,
    true
} bool;

uint16_t get_state_from_combination( char ** key_combination_pointer ) {
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

int bind( int argCount, char ** );

int (*get_callback(char * name ))(int , char **) {
    printf( "returning callback for %s\n", name );
    if ( !strcmp( name, "bind" ) ) {
        return &bind;
    }
    else if ( !strcmp( name, "change_desktop" ) ) {
        return &change_desktop;
    }
    else if ( !strcmp( name, "quit" ) ) {
        return &quit;
    }
    else if ( !strcmp( name, "kill_client" ) ) {
        return &kill_client;
    }
    else {
        return &spawn;
    }
}

void str_array_ncopy( char ** to, char ** from, int n ) {
    int i;
    for ( i = 0; i < n; ++i ) {
        to[ i ] = (char*)malloc( strlen( from[ i ] ) * sizeof( char ) );
        strcpy( to[ i ], from[ i ] );
    }
}

int bind( int argCount, char ** words ) {
    int argc = argCount - 2;

    char * keyCombination = words[ 0 ],
         * command = words[ 1 ],
         ** argv = ( char** )malloc( argc * sizeof( char * ) );

    str_array_ncopy( argv, words + 2, argc );

    struct keybinding k = {
        get_state_from_combination( &keyCombination ),
        get_keysym_from_key( keyCombination ),
        get_callback( command ),
        argc,
        argv
    };

    keybindings[ bindingsCount++ ] = k;

    return 0;
}

int freadline( FILE * fp, char * line ) {
    int i = 0;
    while ( ( line[ i++ ] = fgetc( fp ) ) != EOF ) {
        if ( line[ i - 1 ] == '\n' ) {
            line[ i - 1 ] = '\0';
            return 0;
        }
    }
    return EOF;
}

int str_split( char * s, char * delims, char ** parts ) {
    int count = 0;
    char * result = strtok( s, delims );
    do {
        parts[ count++ ] = (char*)malloc( strlen( result ) * sizeof( char ) );
        strcpy( parts[ count - 1 ], result );
    } while ( ( result = strtok( NULL, " " ) ) != NULL );
    return count; 
}

#define CONFIG_DIR_HOME 0
#define CONFIG_DIR_SYSTEM 1

char * xdg_config_dir( int directory_type ) {
    char * config_dir;
    if ( directory_type == CONFIG_DIR_HOME ) {
        config_dir = getenv( "XDG_CONFIG_HOME" );
        if ( config_dir == NULL ) {
            config_dir = getenv( "HOME" );
            if ( config_dir == NULL ) {
                die( "Please define HOME environment variable.\n" );
            }
        }
    }
    else {
        config_dir = getenv( "XDG_CONFIG_DIRS" );
        if ( config_dir == NULL ) {
            config_dir = (char*)malloc( 4 * sizeof( char ) );
            strcpy( config_dir, "/etc" );
        }
    }
    return config_dir;
}

char * xdg_config_path( char * filename, int directory_type ) {
    char * config_path, * basedir = xdg_config_dir( directory_type );
    struct stat st;

    config_path = (char*)malloc( ( strlen( basedir ) + strlen( "/yawn/" ) + strlen( filename ) ) * sizeof( char ) );
    sprintf( config_path, "%s/yawn/%s", basedir, filename );

    if ( stat( config_path, &st ) != 0 ) {
        if ( directory_type == CONFIG_DIR_HOME ) {
            // local configuration file not found. Try reading system configuration file instead.
            return xdg_config_path( filename, CONFIG_DIR_SYSTEM );
        }
        else {
            printf( "System configuration file %s not found. Please reinstall yawn.\n", config_path );
            exit( 1 );
        }
    }

    return config_path;
}

void read_configuration( char * filename ) {
    int argc, (*callback)( int, char ** );
    char line[ 256 ], section[ 256 ], * argv[ 128 ];
    FILE * fp = fopen( xdg_config_path( "yawn.conf", CONFIG_DIR_HOME ), "r" );

    while ( freadline( fp, line ) != EOF ) {
        int length = strlen( line );
        if ( !length || line[ 0 ] == '#' ) {
            continue;
        }
        if ( line[ 0 ] == '[' && line[ length - 1 ] == ']' ) {
            line[ length - 1 ] = '\0';
            strcpy( section, line + 1 );
            continue;
        }
        argc = str_split( line, " ", argv );
        callback = get_callback( section );
        callback( argc, argv );
    }
}

int main( int argc, char** argv ) {
    setup();
    read_configuration( "yawn.conf" );

    start();
    teardown();

    return 0;
}
