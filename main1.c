#include "main.h"
#include <stdio.h>
#include <stdlib.h>

char *read_input(void) {
    char *input = NULL;
    size_t bufsize = 0;
    ssize_t chars_read;

    chars_read = getline(&input, &bufsize, stdin);
    if (chars_read == -1) {
        perror("getline");
        free(input);
        return NULL;
    }

    /* Remove trailing newline character */
    if (chars_read > 0 && input[chars_read - 1] == '\n') {
        input[chars_read - 1] = '\0';
    }

    return input;
}

int i = 0;
void free_tokens(char **tokens) {
    if (tokens == NULL)
        return;

    while (tokens[i] != NULL) {
        free(tokens[i]);
        i++;
    }

    free(tokens);
}

int main(void) {
    char *input = NULL;
    char **tokens = NULL;
    int i = 0;

    while (1) {
        printf("($) ");
        input = read_input();

        if (input == NULL)
            break;

        tokens = split_input(input);
        free(input);

        if (tokens == NULL)
            continue;

        i = 0;
        while (tokens[i] != NULL) {
            printf("Token %d: %s\n", i, tokens[i]);
            i++;
        }

        free_tokens(tokens);
    }

    return 0;
}

