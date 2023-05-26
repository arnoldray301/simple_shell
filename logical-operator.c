#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int execute_command(char *command, char *path)
{
    pid_t pid;
    int status;

    char *full_command = NULL;
    if (path != NULL)
    {
        size_t full_command_size = strlen(path) + strlen(command) + 2;
        full_command = malloc(full_command_size);
        snprintf(full_command, full_command_size, "%s/%s", path, command);
    }

    if (full_command == NULL || access(full_command, X_OK) == -1)
    {
        fprintf(stderr, "Command '%s' not found\n", command);
        free(full_command);
        return 0;
    }

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        free(full_command);
        return 0;
    }
    if (pid == 0)
    {
        char *args[2];
        args[0] = full_command;
        args[1] = NULL;

        if (execve(args[0], args, NULL) == -1)
        {
            perror("execve");
            free(full_command);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        wait(&status);
        free(full_command);
    }

    return 1;
}

ssize_t read_command(char **command)
{
    ssize_t bytes_read;
    size_t buffer_size = BUFFER_SIZE;

    bytes_read = getline(command, &buffer_size, stdin);
    if (bytes_read == -1)
    {
        perror("getline");
        return -1;
    }

    (*command)[bytes_read - 1] = '\0';

    return bytes_read;
}

int execute_exit(int status)
{
    printf("Exiting shell with status: %d\n", status);
    exit(status);
}

int execute_setenv(char **args)
{
    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "setenv: Missing arguments\n");
        return 0;
    }

    if (setenv(args[1], args[2], 1) == -1)
    {
        fprintf(stderr, "setenv: Failed to set environment variable\n");
        return 0;
    }

    return 1;
}

int execute_unsetenv(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "unsetenv: Missing argument\n");
        return 0;
    }

    if (unsetenv(args[1]) == -1)
    {
        fprintf(stderr, "unsetenv: Failed to unset environment variable\n");
        return 0;
    }

    return 1;
}

int execute_cd(char **args)
{
    char *new_dir;
    char current_dir[PATH_MAX];

    if (args[1] == NULL || strcmp(args[1], "~") == 0)
    {
        new_dir = getenv("HOME");
    }
    else if (strcmp(args[1], "-") == 0)
    {
        new_dir = getenv("OLDPWD");
    }
    else
    {
        new_dir = args[1];
    }

    if (chdir(new_dir) == -1)
    {
        perror("chdir");
        return 0;
    }

    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        perror("getcwd");
        return 0;
    }

    if (setenv("PWD", current_dir, 1) == -1)
    {
        fprintf(stderr, "cd: Failed to update PWD environment variable\n");
        return 0;
    }

    printf("Current directory: %s\n", current_dir);
    return 1;
}

int execute_commands(char **commands, char *path)
{
    int i = 0;
    int status = 0;

    while (commands[i] != NULL)
    {
        char *command = commands[i];
        char *operator = NULL;

        /* Find logical operator if present*/
        char *op_and = strstr(command, "&&");
        char *op_or = strstr(command, "||");

        if (op_and == command)
        {
            operator = "&&";
        }
        else if (op_or == command)
        {
            operator = "||";
        }

        /* Execute command based on logical operator*/
        if (operator != NULL)
        {
            command += strlen(operator) + 1; /* Skip the operator and space*/

            if ((status == 0 && strcmp(operator, "&&") == 0) ||
                (status != 0 && strcmp(operator, "||") == 0))
            {
                if (!execute_command(command, path))
                    break; /* Stop execution if command fails*/
            }
        }
        else
        {
            if (!execute_command(command, path))
                break; /* Stop execution if command fails*/
        }

        i++;
    }

    return status;
}

int main(void)
{
    char *command = NULL;
    ssize_t bytes_read;
    char *path = getenv("PATH");

    while (1)
    {
        printf("($) ");
        fflush(stdout);

        bytes_read = read_command(&command);
        if (bytes_read == -1)
            continue;

        if (strcmp(command, "exit") == 0)
        {
            free(command);
            execute_exit(0);
            break;
        }

        if (strncmp(command, "exit ", 5) == 0)
        {
            int status = atoi(command + 5);
            free(command);
            execute_exit(status);
            break;
        }

        if (strncmp(command, "setenv ", 7) == 0)
        {
            char *args[3];
            char *token;
            int i = 0;

            token = strtok(command + 7, " ");
            while (token != NULL && i < 2)
            {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            if (!execute_setenv(args))
            {
                free(command);
                continue;
            }
        }
        else if (strncmp(command, "unsetenv ", 9) == 0)
        {
            char *args[2];
            char *token;
            int i = 0;

            token = strtok(command + 9, " ");
            while (token != NULL && i < 1)
            {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            if (!execute_unsetenv(args))
            {
                free(command);
                continue;
            }
        }
        else if (strncmp(command, "cd ", 3) == 0)
        {
            char *args[2];
            char *token;
            int i = 0;

            token = strtok(command + 3, " ");
            while (token != NULL && i < 1)
            {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            if (!execute_cd(args))
            {
                free(command);
                continue;
            }
        }
        else
        {
            char *token;
            char **commands = malloc(BUFFER_SIZE * sizeof(char *));
            int i = 0;

            token = strtok(command, ";");
            while (token != NULL && i < BUFFER_SIZE - 1)
            {
                commands[i++] = token;
                token = strtok(NULL, ";");
            }
            commands[i] = NULL;

            execute_commands(commands, path);

            free(commands);
        }

        free(command);
    }

    return 0;
}

