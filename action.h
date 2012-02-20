#ifndef ACTION_H
#define ACTION_H

typedef struct action_t {
    int (*action)(int argc, char ** argv);
    char * name;
} action_t;

/*
action_t actions[] = {
    { spawn, "spawn" }
};
*/

int (*get_action(char * name ))(int , char **);

#endif
