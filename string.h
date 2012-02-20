#ifndef STRING_H
#define STRING_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int freadline( FILE * fp, char * line );
void string_array_ncopy( char ** to, char ** from, int n );
int string_split( char * s, char * delims, char ** parts );

#endif

