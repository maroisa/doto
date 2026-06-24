#include "doto.h"

#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char **environ;

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
    char *doto_path, *cwd = NULL;
    char *home_dir = getenv("HOME");
    pid_t pid;
    int status;

    if (argc < 1){
        fprintf(stderr, "input file expected.\n");
        return 1;
    }

    doto_path = get_doto_path();

    if (doto_path == NULL){
        perror("failed to open doto directory");
        return 1;
    }

    cwd = getcwd(NULL, 0);

    if (chdir(doto_path) != 0){
        perror("failed to go to doto directory");
        free(doto_path);
        free(cwd);
        return 1;
    }

    status = system("git rev-parse --is-inside-work-tree > /dev/null 2>&1");
    if (status != 0){
        fprintf(stderr, "Doto directory is not a git repository.\n");
        printf("initialize it with \"doto init --force\"\n");
        free(doto_path);
        free(cwd);
        return 1;
    }

    if (chdir(cwd) != 0){
        fprintf(stderr, "failed to go back to working directory");
        free(doto_path);
        free(cwd);
        return 1;
    }

    for (int i = 0; i < argc; i++) {
    }

    free(doto_path);
    free(cwd);


    return 0;
}

int handle_migrate(){
    printf("Migrating...\n");
    return 0;
}

int handle_init(int argc, char *argv[]){
    char *doto_path, *repo = NULL;
    int force_flag = -1;
    int status;
    pid_t pid;

    if (argc > 2){
        fprintf(stderr, "Too much argument provided.\n");
        return 1;
    }

    doto_path = get_doto_path();

    if (doto_path == NULL){
        perror("failed to open doto directory");
        return 1;
    }

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--force") == 0)
            force_flag = i;
    }

    if (argc == 2){
        if (force_flag == -1){
            printf("wrong argument.\n");
            free(doto_path);
            return 1;
        }

        repo = argv[!force_flag];
    }

    if (force_flag >= 0){
        char* command[] = {"rm", "-rf", doto_path, NULL};
        status = posix_spawnp(&pid, "rm", NULL, NULL, command, environ);

        if (status == -1){
            perror("failed to spawn new process");
            free(doto_path);
            if (repo != NULL) free(repo);
            return 1;
        }

        if (waitpid(pid, &status, 0) == -1){
            perror("Failed to get process");
            free(doto_path);
            if (repo != NULL) free(repo);
            return 1;
        }
    }

    printf("creating directory in %s...\n\n", doto_path);

    if (mkdir(doto_path, 0755) == -1){
        if (errno == EEXIST){
            fprintf(stderr, "%s already exist.\nUse --force to overwrite\n", doto_path);

            free(doto_path);
            if (repo != NULL) free(repo);
            return 1;
        }

        perror("Failed to create doto directory.\n");

        free(doto_path);
        if (repo != NULL) free(repo);
        return 1;
    }

    if (chdir(doto_path) != 0) {
        perror("failed to go to doto's directory");
        free(doto_path);
        if (repo != NULL) free(repo);
        return 1;
    }

    free(doto_path);

    if (repo != NULL){
        char *command[] = {"git", "clone", repo, ".", NULL};
        status = posix_spawnp(&pid, "git", NULL, NULL, command, environ);
    } else {
        char *command[] = {"git", "init", NULL};
        status = posix_spawnp(&pid, "git", NULL, NULL, command, environ);
    }

    if (status != 0){
        fprintf(stderr, "Failed to spawn git process.\n");
        return 1;
    }

    if (waitpid(pid, &status, 0) == -1){
        perror("Failed to spawn child process.");
        return 1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0){
        if (repo == NULL) printf("Successfully initialize new git repository.\n");
        else printf("Successfully clone repository.\n");

        free(repo);
        return 0;
    } else return 1;
}

int handle_cd(){
    char* doto_path;
    char* shell = getenv("SHELL");
    if (shell == NULL){
        perror("Error changing directory");
        return 1;
    }

    doto_path = get_doto_path();
    if (doto_path == NULL){
        fprintf(stderr, "failed to get doto directory");
        return 1;
    }

    if (chdir(doto_path) == -1){
        fprintf(stderr, "doto directory does not exist.\n run \"doto init\" to initialize\n");
        free(doto_path);
        return 1;
    };

    free(doto_path);

    char *args[] = {shell, NULL};
    if (execv(shell, args) == -1) {
        perror("Error launching shell");
        return 1;
    }

    return 0;
}


char* get_doto_path(){
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
