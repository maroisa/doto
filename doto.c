#ifndef APP_VERSION
#define APP_VERSION "v0.0.0"
#endif

#include "doto.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int route_command(int argc, char *argv[]){
    int sub_argc = argc - 2;
    char **sub_argv = &argv[2];

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) return print_help();
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) return print_version();

    if (strcmp(argv[1], "init") == 0) return handle_init(sub_argc, sub_argv);
    if (strcmp(argv[1], "migrate") == 0) return handle_migrate();
    if (strcmp(argv[1], "add") == 0) return handle_add(sub_argc, sub_argv);
    if (strcmp(argv[1], "cd") == 0) return handle_cd();

    fprintf(stderr, "wrong argument. Use -h or --help to show commands\n");
    return 1;
}

int print_help(){
    printf("Usage: doto <options> ...\n\n");
    printf("Options:\n");
    printf("    -h, --help         Show this message\n");
    printf("    -v, --version      Show package's version\n");
    printf("    init [repository]  Initialize doto directory\n");
    printf("    migrate            Migrate your dotfiles from git to home\n");
    printf("    add <file>...      add change(s) to doto directory\n");
    printf("    cd                 cd to doto directory\n");
    return 0;
}

int print_version(){
    printf("doto %s\n", APP_VERSION);
    return 0;
}

int handle_add(int argc, char *argv[]){
    printf("Applying...\n");
    return 0;
}

int handle_migrate(){
    printf("Migrating...\n");
    return 0;
}

int handle_init(int argc, char *argv[]){
    char* doto_path = get_doto_dir();
    int isForced = 0;

    for (int i = 0; i < 2; i++) {
        if (strcmp(argv[i], "--force") == 0){
            isForced = 1;
        }
    }

    printf("creating directory in %s...\n\n", doto_path);

    if (mkdir(doto_path, 0755) == -1){
        if (errno == EEXIST){
            fprintf(stderr, "%s already exist.\nUse --force to overwrite\n", doto_path);
            return 1;
        }

        perror("Failed to create doto directory.\n");
        return 1;
    }
    printf("Successfully initialize doto directory in %s\n", doto_path);

    free(doto_path);

    return 0;
}

int handle_cd(){
    char* doto_dir = get_doto_dir();
    char* shell = getenv("SHELL");
    if (shell == NULL){
        perror("Error changing directory");
        return 1;
    }

    if (chdir(doto_dir) == -1){
        fprintf(stderr, "doto directory does not exist.\n run \"doto init\" to initialize\n");
        return 1;
    };

    char *args[] = {shell, NULL};
    if (execv(shell, args) == -1) {
        perror("Error launching shell");
        return 1;
    }

    return 0;
}

char* get_doto_dir(){
    const char *home = getenv("HOME");

    if (home == NULL || strcmp(home, "") == 0){
        fprintf(stderr, "$HOME environment variable is not set.\n");
        return NULL;
    }

    size_t doto_path_len = strlen(home) + strlen(".local/share/doto") + 2;
    char *doto_path = (char*)malloc(doto_path_len);
    if (doto_path == NULL){
        fprintf(stderr, "Failed to allocate memory");
        return NULL;
    }

    snprintf(doto_path, doto_path_len, "%s/.local/share/doto", home);

    return doto_path;
}
