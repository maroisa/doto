#include "doto.h"

#include <stdio.h>
#include <string.h>

int route_command(int argc, char *argv[]){
    int sub_argc = argc - 2;
    char **sub_argv = &argv[2];

    if (strcmp(argv[1], "help") == 0) return print_help();
    if (strcmp(argv[1], "init") == 0) return handle_init(sub_argc, sub_argv);
    if (strcmp(argv[1], "migrate") == 0) return handle_migrate(sub_argc, sub_argv);
    if (strcmp(argv[1], "apply") == 0) return handle_apply(sub_argc, sub_argv);

    printf("WRONG ARGUMENT!\n\n");
    print_help();
    return 1;
}

int print_help(){
    printf("Usage: doto [options] ...\n\n");
    printf("Options:\n");
    printf("    help\tShow this message\n");
    printf("    init\tInitialize git repository for dotfiles\n");
    printf("    migrate\tMigrate your dotfiles from git to home\n");
    printf("    apply\tApply changes to git repository\n");

    return 0;
}


int handle_apply(int argc, char *argv[]){
    printf("Applying...\n");
    return 0;
}

int handle_migrate(int argc, char *argv[]){
    printf("Migrating...\n");
    return 0;
}

int handle_init(int argc, char *argv[]){
    printf("Initialize...\n");
    return 0;
}
