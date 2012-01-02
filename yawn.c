#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct client {
    // Prev and next client
    struct client *next;
    struct client *prev;

    // The window
    Window win;
} client;

Display* display;

// desktop
int master_size;
int mode;
client* head;
client* current;

int sh;
int sw;
int screen;
Window root;
int bool_quit;

void die( const char* e );
void sigchld( int unused );
void setup();
void start();
void add_window( Window w );
void tile();
void update();

// XEvent handlers
void keypress( XEvent *e );
void maprequest( XEvent *e );
void destroynotify( XEvent *e );
void configurenotify( XEvent *e );
void configurerequest( XEvent *e );

void (*events[LASTEvent])(XEvent *e) = {
    [KeyPress] = keypress,
    [MapRequest] = maprequest,
    [DestroyNotify] = destroynotify,
    [ConfigureNotify] = configurenotify,
    [ConfigureRequest] = configurerequest
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

void keypress( XEvent* e ) {
    printf( "yawn: keypress\n" );
}

void maprequest( XEvent* e ) {
    printf( "yawn: maprequest\n" );

    XMapRequestEvent* ev = &e->xmaprequest;
    add_window( ev->window );
    XMapWindow( display, ev->window );
    tile();
    update();
}

void destroynotify( XEvent* e ) {
    printf( "yawn: destroynotify\n" );
}

void configurenotify( XEvent* e ) {
    printf( "yawn: configurenotify\n" );
}

void configurerequest( XEvent* e ) {
    printf( "yawn: configurerequest\n" );

    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;
    wc.x = ev->x;
    wc.y = ev->y;
    wc.width = ev->width;
    wc.height = ev->height;
    wc.border_width = ev->border_width;
    wc.sibling = ev->above;
    wc.stack_mode = ev->detail;
    XConfigureWindow( display, ev->window, ev->value_mask, &wc );
}

void add_window( Window w ) {
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
        printf( "yawn: resizing current\n" );
        XMoveResizeWindow( display, current->win, 0, 0, sw-2, sh-2 );
    }
}

void update() {
    if ( current != NULL ) {
        printf( "yawn: raising current\n" );
        XSetInputFocus( display, current->win, RevertToParent, CurrentTime );
        XRaiseWindow( display, current->win );
    }
}

#define MASTER_SIZE     0.6

void setup() {
    // Install a signal
    sigchld( 0 );

    screen = DefaultScreen( display );
    root = RootWindow( display, screen );

    // Screen width and height
    sw = XDisplayWidth( display, screen );
    sh = XDisplayHeight( display, screen );

    // For exiting
    bool_quit = 0;

    // List of client
    head = NULL;
    current = NULL;
    mode = 0;
    master_size = sw * MASTER_SIZE;

    XSelectInput( display, root, SubstructureNotifyMask | SubstructureRedirectMask );
}

void start() {
    XEvent ev;

    // main loop, just dispatch events
    while ( !bool_quit && !XNextEvent( display, &ev ) ) {
        if ( events[ ev.type ] ) {
            events[ ev.type ]( &ev );
        }
    }
}

int main( int argc, char** argv ) {
    if ( !( display = XOpenDisplay( NULL ) ) ) {
        die( "Cannot open display!" );
    }

    setup();
    start();

    XCloseDisplay( display );

    return 0;
}
