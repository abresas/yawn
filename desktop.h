#ifndef DESKTOP_H
#define DESKTOP_H

struct desktop_t {
    struct client_t *head;
    struct client_t *current;
};

int desktop_change( int argc, char ** argv );
void desktop_save( int i );
void desktop_select( int i );

#endif
