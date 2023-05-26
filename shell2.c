#include "main.h"

char *get_full_path(char *command, char **paths);
int command_exists(char *command, char **paths);

int main(void)
{
    char *input;
    char **args;
    int status;
    char *path;
    char **paths;

    signal(SIGINT, handle_sigint);

    path = getenv("PATH");
    paths = split_input(path);

    while (1)
    {
        printf("$ ");
        input = read_input();
        args = split_input(input);

        if (args[0] == NULL)
        {
            free(input);
            free_tokens(args);
            continue;
        }

        if (strcmp(args[0], "exit") == 0)
        {
            free(input);
            free_tokens(args);
            break;
        }

        if (command_exists(args[0], paths))
        {
            char *full_path = get_full_path(args[0], paths);
            args[0] = full_path;

            status = execute_command(args);

            free(full_path);
        }
        else
        {
            printf("Command not found: %s\n", args[0]);
            status = -1;
        }

        free(input);
        free_tokens(args);

        if (status == -1)
            perror("Error executing command");
    }

    free_tokens(paths);

    return EXIT_SUCCESS;
}

void handle_sigint(int signal)
{
    (void)signal;
    write(STDOUT_FILENO, "\n$ ", 3);
}

void free_tokens(char **tokens)
{
    int i = 0;
    while (tokens[i])
    {
        free(tokens[i]);
        i++;
    }
    free(tokens);
}

char **split_input(char *input)
{
    char **tokens = NULL;
    char *token;
    int position = 0;

    tokens = malloc(sizeof(char *) * BUFFER_SIZE);
    if (!tokens)
    {
        perror("Allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(input, " \t\n\r");
    while (token)
    {
        tokens[position] = strdup(token);
        if (!tokens[position])
        {
            perror("Allocation error");
            exit(EXIT_FAILURE);
        }
        position++;

        token = strtok(NULL, " \t\n\r");
    }
    tokens[position] = NULL;

    return tokens;
}

char *read_input(void)
{
    char *input = NULL;
    size_t bufsize = 0;
    getline(&input, &bufsize, stdin);
    return input;
}

int execute_command(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        execve(args[0], args, NULL);
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("Fork error");
        return -1;
    }
    else
    {
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return status;
}

char *get_full_path(char *command, char **paths)
{
    char *full_path = NULL;

    int i = 0;
    while (paths[i] != NULL)
    {
        char *path = paths[i];
        full_path = malloc(strlen(path) + strlen(command) + 2);  /* +2 for '/' and '\0'*/
        if (!full_path)
        {
            perror("Allocation error");
            exit(EXIT_FAILURE);
        }
        sprintf(full_path, "%s/%s", path, command);
        if (access(full_path, X_OK) == 0)
            return full_path;

        free(full_path);
        i++;
    }

    return NULL;
}

int command_exists(char *command, char **paths)
{
    char *full_path = get_full_path(command, paths);
    if (full_path)
    {
        free(full_path);
        return 1;
    }
    return 0;
}

