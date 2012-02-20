#ifndef YAWN_H
#define YAWN_H

#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "globals.h"

void die( const char* e );
void sigchld( int unused );
void setup();
void start();
void teardown();
int quit( int argc, char ** argv );
int spawn( int argc, char ** argv );

typedef enum {
    false,
    true
} bool;

#endif

