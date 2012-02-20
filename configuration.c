#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "configuration.h"
#include "string.h"
#include "yawn.h"
#include "action.h"

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

void configuration_read( char * filename ) {
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
        argc = string_split( line, " ", argv );
        callback = get_action( section );
        callback( argc, argv );
    }
}
