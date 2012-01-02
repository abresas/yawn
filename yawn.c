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

static Display* display;

// desktop
int master_size;
int mode;
client* head;
client* current;

static int sh;
static int sw;
static int screen;
static Window root;
static int bool_quit;

static void die( const char* e );
static void sigchld( int unused );
static void setup();
static void start();

// XEvent handlers
static void keypress( XEvent *e );
static void maprequest( XEvent *e );
static void destroynotify( XEvent *e );
static void configurenotify( XEvent *e );
static void configurerequest( XEvent *e );

static void (*events[LASTEvent])(XEvent *e) = {
    [KeyPress] = keypress,
    [MapRequest] = maprequest,
    [DestroyNotify] = destroynotify,
    [ConfigureNotify] = configurenotify,
    [ConfigureRequest] = configurerequest
};


void die( const char* e ) {
    printf( "yawn: %s\n", e );
    exit( 1 );
}

void sigchld( int unused ) {
	if( signal( SIGCHLD, sigchld ) == SIG_ERR ) { // re-register for next time
		die( "Can't install SIGCHLD handler" );
    }
    
	while( 0 < waitpid( -1, NULL, WNOHANG ) );
}

void keypress( XEvent *e ) {
    printf( "yawn: keypress\n" );
}

void maprequest( XEvent *e ) {
    printf( "yawn: maprequest\n" );
}

void destroynotify( XEvent *e ) {
    printf( "yawn: destroynotify\n" );
}

void configurenotify( XEvent *e ) {
    printf( "yawn: configurenotify\n" );
}

void configurerequest( XEvent *e ) {
    printf( "yawn: configurerequest\n" );
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
