#include "main.h"

int main(void)
{
    char *input;
    char **args;
    int status;

    signal(SIGINT, handle_sigint);

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

        status = execute_command(args);

        free(input);
        free_tokens(args);

        if (status == -1)
            perror("Error executing command");
    }

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

