#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define CONFIG_DIR_HOME 0
#define CONFIG_DIR_SYSTEM 1

char * xdg_config_dir( int directory_type );
char * xdg_config_path( char * filename, int directory_type );
void configuration_read( char * filename );

#endif

