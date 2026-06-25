#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ftw.h>
#include <fcntl.h>
#include <pwd.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <git2.h>
#include <git2/clone.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/repository.h>
#include <git2/types.h>


#include "doto.h"

extern char **environ;

static int rm_files(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){
    if (remove(fpath) < 0){
        perror("failed to remove");
        return -1;
    }
    return 0;
}

static int mkdir_in_doto_path(char *target_path, char *cwd, char *doto_path){
    if (chdir(doto_path) != 0){
        perror("failed to go to doto directory");
        return 1;
    }

    for (char *p = target_path; *p; p++){
        if (*p == '/'){
            *p = '\0';
            mkdir(target_path, 0755);
            *p = '/';
        }
    }

    if (chdir(cwd) != 0){
        perror("failed to go to doto directory");
        return 1;
    }
    return 0;
}

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
    char *doto_path, *cwd, *home_dir = NULL;
    char *home_env = getenv("HOME");
    int status;
    size_t home_len;

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

    git_libgit2_init();
    status = git_repository_open_ext(NULL, doto_path, GIT_REPOSITORY_OPEN_NO_SEARCH, NULL);
    git_libgit2_shutdown();

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

    if (home_env != NULL) home_dir = realpath(home_env, NULL);
    else {
        struct passwd *pw = getpwuid(getuid());
        if (pw && pw->pw_dir) home_dir = realpath(pw->pw_dir, NULL);
    }

    home_len = strlen(home_dir);

    struct stat stats;

    for (int i = 0; i < argc; i++) {
        char *absolute_path = realpath(argv[i], NULL);
        if (absolute_path == NULL){
            perror("failed to get path");
            free(doto_path);
            free(cwd);
            return 1;
        }

        if (strncmp(absolute_path, home_dir, home_len) != 0){
            fprintf(stderr, "File %s is not in home directory!\n", absolute_path + home_len + 1);
            free(cwd);
            free(doto_path);
            free(absolute_path);
            return 1;
        }

        if (stat(absolute_path, &stats) == 0){
            if (S_ISDIR(stats.st_mode)){
                fprintf(stderr, "failed to add: %s is directory!\n -- this feature is in progress -- \n", absolute_path + home_len + 1);
                free(cwd);
                free(doto_path);
                free(absolute_path);
                return 1;
            }
        }

        size_t dest_len = strlen(doto_path) + strlen(absolute_path + home_len) + 1;
        char dest_path[dest_len];
        snprintf(dest_path, dest_len, "%s%s", doto_path, absolute_path + home_len);

        char dest_path_copy[dest_len];
        snprintf(dest_path_copy, dest_len, "%s", absolute_path + home_len + 1);

        mkdir_in_doto_path(dest_path_copy, cwd, doto_path);

        if (stat(absolute_path, &stats) != 0){
            fprintf(stderr, "failed to read file stat: %s\n", absolute_path + home_len + 1);
            return 1;
        }

        FILE *src = fopen(absolute_path, "rb");
        FILE *dst = fopen(dest_path, "wb");

        if (!src || !dst){
            if (src) fclose(src);
            if (dst) fclose(dst);
            perror("failed to open/write file");
            return 1;
        }

        char buf[65536];
        size_t buf_size;

        printf("adding \033[33m%s\033[0m...\n", absolute_path + home_len + 1);
        while ((buf_size = fread(buf, 1, sizeof(buf), src)) > 0) {
            fwrite(buf, 1, buf_size, dst);
        }

        fclose(src);
        fclose(dst);

        if (chmod(dest_path, stats.st_mode) != 0){
            fprintf(stderr, "failed to copy file's permission: %s\n", absolute_path + home_len + 1);
            return 1;
        }

        struct timespec times[2];
        times[0] = stats.st_atim;
        times[1] = stats.st_mtim;

        if (utimensat(AT_FDCWD, dest_path, times, 0) != 0){
            fprintf(stderr, "failed to copy file's time: %s\n", absolute_path + home_len + 1);
            return 1;
        }
        printf("success added \033[33m%s\033[0m.\n\n", absolute_path + home_len + 1);
    }

    printf("\033[32mcommand succeeded!\033[0m\n");
    return 0;
}

int handle_migrate(){
    printf("Migrating...\n");
    return 0;
}

int handle_init(int argc, char *argv[]){
    char *doto_path, *repo_url = NULL;
    int force_flag = -1;
    int status;

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

        if (argv[!force_flag] != NULL) {
            repo_url = argv[!force_flag];
        }
    }

    if (access(doto_path, F_OK) == 0){
        if (force_flag >= 0){
            if (nftw(doto_path, rm_files, 10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0){
                perror("Error: nftw");
                free(doto_path);
                if (repo_url != NULL) free(repo_url);
                return 1;
            }
        }
        else {
            fprintf(stderr, "doto directory exists!\noverwrite it with --force\n");
            free(doto_path);
            if (repo_url != NULL) free(repo_url);
            return 1;
        }
    }

    git_libgit2_init();

    git_repository *repo = NULL;

    if (repo_url != NULL){
        printf("cloning %s...\n", repo_url);
        git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
        status = git_clone(&repo, repo_url, doto_path, &clone_opts);
    }
    else {
        printf("initializing...\n");
        status = git_repository_init(&repo, doto_path, 0);
    }

    printf("\n");

    git_repository_free(repo);

    if (status != 0){
        const git_error *error = git_error_last();
        printf("after git_error\n");
        fprintf(
            stderr,
            "Failed to %s repo: %s\n",
            repo_url != NULL ? "clone" : "initialize",
            error ? error->message : "Unknown error"
        );
        git_libgit2_shutdown();
        if (repo_url != NULL) free(repo_url);
        return 1;
    }
    printf("successfully %s repository!\n", repo_url != NULL ? "cloned your" : "initialize empty");

    git_libgit2_shutdown();

    return 0;
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
