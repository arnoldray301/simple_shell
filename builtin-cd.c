#include <unistd.h>  /* for chdir() */
#include <stdlib.h>  /* for getenv(), setenv() */
#include <stdio.h>   /* for fprintf() */
#include <string.h>  /* for strcmp() */

int execute_cd(char **args)
{
    char cwd[1024];

    if (args[1] == NULL || strcmp(args[1], "~") == 0) {
        /* Change directory to HOME */
        char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "HOME environment variable not set.\n");
            return -1;
        }
        if (chdir(home) != 0) {
            fprintf(stderr, "Failed to change directory to HOME.\n");
            return -1;
        }
    } else if (strcmp(args[1], "-") == 0) {
        /* Change directory to previous directory */
        char *prev_dir = getenv("OLDPWD");
        if (prev_dir == NULL) {
            fprintf(stderr, "Previous directory not set.\n");
            return -1;
        }
        if (chdir(prev_dir) != 0) {
            fprintf(stderr, "Failed to change directory to previous directory.\n");
            return -1;
        }
    } else {
        /* Change directory to specified directory */
        if (chdir(args[1]) != 0) {
            fprintf(stderr, "Failed to change directory.\n");
            return -1;
        }
    }

    /* Update the PWD environment variable */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "Failed to get current working directory.\n");
        return -1;
    }
    if (setenv("PWD", cwd, 1) != 0) {
        fprintf(stderr, "Failed to update PWD environment variable.\n");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    /* Check if an argument is provided for the directory */
    if (argc > 1) {
        /* Create an array to hold the command and its argument */
        char *command[2];
        command[0] = argv[1];  /* Command is the first argument */
        command[1] = NULL;    /* The argument is NULL-terminated */

        /* Call the execute_cd function with the command and its argument */
        execute_cd(command);
    } else {
        /* No argument provided, equivalent to cd $HOME */
        char *command[2];
        command[0] = "~";      /* Use "~" as the argument */
        command[1] = NULL;     /* The argument is NULL-terminated */

        /* Call the execute_cd function with the command and its argument */
        execute_cd(command);
    }

    return 0;
}

