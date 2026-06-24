#ifndef DOTO_H
#define DOTO_H
#ifndef APP_VERSION
    #define APP_VERSION "v0.0.0"
#endif

int route_command(int argc, char *argv[]);
int print_help();
int print_version();

int handle_init(int argc, char *argv[]);
int handle_add(int argc, char *argv[]);
int handle_migrate();
int handle_cd();

char* get_doto_path();

#endif
