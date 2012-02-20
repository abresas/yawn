#include "string.h"

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

void string_array_ncopy( char ** to, char ** from, int n ) {
    int i;
    for ( i = 0; i < n; ++i ) {
        to[ i ] = (char*)malloc( strlen( from[ i ] ) * sizeof( char ) );
        strcpy( to[ i ], from[ i ] );
    }
}

int string_split( char * s, char * delims, char ** parts ) {
    int count = 0;
    char * result = strtok( s, delims );
    do {
        parts[ count++ ] = (char*)malloc( strlen( result ) * sizeof( char ) );
        strcpy( parts[ count - 1 ], result );
    } while ( ( result = strtok( NULL, delims ) ) != NULL );
    return count; 
}
