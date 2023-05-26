#include <unistd.h>  /* for chdir() */
#include <stdlib.h>  /* for getenv(), setenv() */
#include <stdio.h>   /* for fprintf() */
#include <string.h>  /* for strcmp(), strtok() */
#include <sys/wait.h>

int execute_cd(char *arg)
{
    char cwd[1024];

    if (arg == NULL || strcmp(arg, "~") == 0) {
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
    } else if (strcmp(arg, "-") == 0) {
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
        if (chdir(arg) != 0) {
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

char *end;

int main(int argc, char *argv[])
{
    /* Declarations */
    pid_t pid;
    int status;

    /* Check if an argument is provided for the directory */
    if (argc > 1) {
        /* Tokenize the input command based on ";" separator */
        char *token;
        char *command = argv[1];

        token = strtok(command, ";");
        while (token != NULL) {
            /* Trim leading and trailing spaces */
            char *arg = token;
            while (*arg == ' ') {
                arg++;
            }
            end = arg + strlen(arg) - 1;
            while (*end == ' ' && end > arg) {
                *end = '\0';
                end--;
            }

            /* Execute the command */
            if (strncmp(arg, "cd", 2) == 0) {
                /* If the command is "cd", handle it separately */
                execute_cd(arg + 2);  /* Skip "cd" in the argument*/
            } else {
                /* Handle other commands using execvp */
                char *args[64];  /* Array to store command and arguments*/
                int arg_count = 0;

                /* Tokenize the command and arguments */
                char *cmd_token = strtok(arg, " ");
                while (cmd_token != NULL) {
                    args[arg_count++] = cmd_token;
                    cmd_token = strtok(NULL, " ");
                }
                args[arg_count] = NULL;  /* Null-terminate the args array*/

                /* Fork and execute the command */
                pid = fork();
                if (pid == 0) {
                    execvp(args[0], args);
                    fprintf(stderr, "Failed to execute command: %s\n", args[0]);
                    exit(1);
                } else if (pid > 0) {
                    waitpid(pid, &status, 0);
                } else {
                    fprintf(stderr, "Failed to fork process.\n");
                }
            }

            token = strtok(NULL, ";");
        }
    } else {
        /* No argument provided, equivalent to cd $HOME */
        char *command = "~";

        /* Call the execute_cd function with the command */
        execute_cd(command);
    }

    return 0;
}

