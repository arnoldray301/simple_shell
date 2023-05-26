#include "main.h"

#define TOKEN_DELIMITERS " \t\r\n\a"

char **split_input(char *input)
{
    char **tokens = NULL;
    int token_count = 0;
    int max_tokens = 10;
    int input_len = strlen(input);
    int i, j, start, end;

    tokens = malloc(max_tokens * sizeof(char *));
    if (tokens == NULL) {
        return NULL;
    }

    i = 0;
    while (i < input_len) {
        /* Skip delimiters */
        while (input[i] && strchr(TOKEN_DELIMITERS, input[i])) {
            i++;
        }

        if (!input[i]) {
            break; /* Reached end of input */
        }

        start = i;

        /* Find the end of the token */
        while (input[i] && !strchr(TOKEN_DELIMITERS, input[i])) {
            i++;
        }

        end = i;

        /* Extract the token */
        tokens[token_count] = malloc((end - start + 1) * sizeof(char));
        if (tokens[token_count] == NULL) {
            /* Memory allocation failed */
            for (j = 0; j < token_count; j++) {
                free(tokens[j]);
            }
            free(tokens);
            return NULL;
        }
        strncpy(tokens[token_count], input + start, end - start);
        tokens[token_count][end - start] = '\0';
        token_count++;

        if (token_count >= max_tokens) {
            max_tokens += 10;
            tokens = realloc(tokens, max_tokens * sizeof(char *));
            if (tokens == NULL) {
                /* Memory allocation failed */
                for (j = 0; j < token_count; j++) {
                    free(tokens[j]);
                }
                free(tokens);
                return NULL;
            }
        }
    }

    tokens[token_count] = NULL;
    return tokens;
}

void execute_exit(char **args)
{
    if (args[1] != NULL) {
        int exit_status = atoi(args[1]);
        exit(exit_status);
    } else {
        exit(0);
    }
}

int result;
int execute_setenv(char **args)
{
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "Usage: setenv VARIABLE VALUE\n");
        return -1;
    }

    result = setenv(args[1], args[2], 1);
    if (result != 0) {
        fprintf(stderr, "Failed to set environment variable\n");
        return -1;
    }

    return 0;
}
int result;
int execute_unsetenv(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: unsetenv VARIABLE\n");
        return -1;
    }

    result = unsetenv(args[1]);
    if (result != 0) {
        fprintf(stderr, "Failed to unset environment variable\n");
        return -1;
    }

    return 0;
}

char cwd[1024];
int execute_cd(char **args)
{
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

int main()
{
    char **tokens;
    int i;
    char input[100]; /* Increased size to handle larger input */
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0'; /* Remove trailing newline character */

    tokens = split_input(input);
    if (tokens != NULL) {
        if (strcmp(tokens[0], "exit") == 0) {
            execute_exit(tokens);
        } else if (strcmp(tokens[0], "setenv") == 0) {
            execute_setenv(tokens);
        } else if (strcmp(tokens[0], "unsetenv") == 0) {
            execute_unsetenv(tokens);
        } else if (strcmp(tokens[0], "cd") == 0) {
            execute_cd(tokens);
        } else {
            /* Handle other commands */
        }

        /* Free memory allocated for tokens */
        i = 0;
        while (tokens[i] != NULL) {
            free(tokens[i]);
            i++;
        }
        free(tokens);
    }
    return 0;
}

