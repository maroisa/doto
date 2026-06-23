#ifndef DOTO_H
#define DOTO_H

int route_command(int argc, char *argv[]);
int print_help();

int handle_init(int argc, char *argv[]);
int handle_migrate(int argc, char *argv[]);
int handle_apply(int argc, char *argv[]);

#endif
