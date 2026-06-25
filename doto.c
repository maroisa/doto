#include <git2/clone.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <git2.h>


#include "doto.h"

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

    for (int i = 0; i < argc; i++) {
        char *absolute_path = realpath(argv[i], NULL);
        if (absolute_path == NULL){
            perror("failed to get file");
            free(home_dir);
            free(doto_path);
            return 1;
        }

        if (strncmp(absolute_path, home_dir, home_len) != 0){
            fprintf(stderr, "file must be in home directory: %s\n", argv[i]);
            free(home_dir);
            free(doto_path);
            free(absolute_path);
            return 1;
        }

        free(home_dir);

        printf("copying...\n");

        char *absolute_doto_path = realpath(doto_path, NULL);

        free(doto_path);

        if (absolute_doto_path == NULL){
            printf("doto directory not found!");
            free(absolute_path);
            return 1;
        }

        char dest_path[512];

        size_t dest_len = strlen(absolute_doto_path) + strlen(absolute_path + home_len) + 1;
        snprintf(dest_path, dest_len, "%s%s", absolute_doto_path, absolute_path + home_len);

        free(absolute_doto_path);

        char *g_dest_path = g_path_get_dirname(dest_path);

        if (g_mkdir_with_parents(g_dest_path, 0755) == -1){
            g_printerr("Error creating directories: %s\n", g_strerror(errno));
            g_free(g_dest_path);
            free(absolute_path);
            return 1;
        }

        g_free(g_dest_path);

        GFile *source_file = g_file_new_for_path(absolute_path);
        GFile *dest_file = g_file_new_for_path(dest_path);
        GError *error = NULL;

        free(absolute_path);

        g_file_copy(source_file, dest_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error);

        if (error != NULL){
            g_printerr("Error copy file: %s\n", error->message);
            g_error_free(error);
            g_object_unref(source_file);
            g_object_unref(dest_file);
            return 1;
        }

        printf("Successfully adding your file.\n");
        g_object_unref(source_file);
        g_object_unref(dest_file);
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
    char *doto_path, *repo_url = NULL;
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

        repo_url = argv[!force_flag];
    }

    if (force_flag >= 0){
        // TODO: replace with glib function
        char* command[] = {"rm", "-rf", doto_path, NULL};
        status = posix_spawnp(&pid, "rm", NULL, NULL, command, environ);

        if (status == -1){
            perror("failed to spawn new process");
            free(doto_path);
            if (repo_url != NULL) free(repo_url);
            return 1;
        }

        if (waitpid(pid, &status, 0) == -1){
            perror("Failed to get process");
            free(doto_path);
            if (repo_url != NULL) free(repo_url);
            return 1;
        }
    } else {
        if (g_file_test(doto_path, G_FILE_TEST_IS_DIR)){
            fprintf(stderr, "doto directory exists!\noverwrite it with --force\n");
            free(doto_path);
            if (repo_url != NULL) free(repo_url);
            return 1;
        };
    }

    printf("creating directory in %s...\n\n", doto_path);

    git_libgit2_init();

    git_repository *repo = NULL;

    if (repo_url != NULL){
        git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
        status = git_clone(&repo, repo_url, doto_path, &clone_opts);
    }
    else {
        status = git_repository_init(&repo, doto_path, 0);
    }

    git_repository_free(repo);

    if (status != 0){
        const git_error *error = git_error_last();
        printf("after git_error\n");
        fprintf(
            stderr,
            "Failed to %s repo: %s\n",
            repo_url != NULL ? "initialize" : "clone",
            error ? error->message : "Unknown error"
        );
        git_libgit2_shutdown();
        if (repo_url != NULL) free(repo_url);
        return 1;
    }

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
