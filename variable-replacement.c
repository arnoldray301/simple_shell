#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdlib.h>

#define BUFFER_SIZE 1024

struct Alias {
    char* name;
    char* value;
};

typedef struct Alias Alias;

Alias* aliases = NULL;
size_t num_aliases = 0;

ssize_t read_command(char** command) {
    ssize_t bytes_read;
    size_t bufsize = 0;

    bytes_read = getline(command, &bufsize, stdin);
    if (bytes_read == -1) {
        perror("getline");
    } else if (bytes_read > 0 && (*command)[bytes_read - 1] == '\n') {
        (*command)[bytes_read - 1] = '\0';  /* Remove trailing newline*/
    }

    return bytes_read;
}

void execute_exit(int status) {
    exit(status);
}

int execute_setenv(char** args) {
    if (args[1] != NULL && args[2] != NULL) {
        if (setenv(args[1], args[2], 1) == -1) {
            perror("setenv");
            return 0;
        }
    } else {
        fprintf(stderr, "Usage: setenv <variable> <value>\n");
        return 0;
    }
    return 1;
}

int execute_unsetenv(char** args) {
    if (args[1] != NULL) {
        if (unsetenv(args[1]) == -1) {
            perror("unsetenv");
            return 0;
        }
    } else {
        fprintf(stderr, "Usage: unsetenv <variable>\n");
        return 0;
    }
    return 1;
}

int execute_cd(char** args) {
    if (args[1] != NULL) {
        if (chdir(args[1]) == -1) {
            perror("chdir");
            return 0;
        }
    } else {
        const char* home = getenv("HOME");
        if (home != NULL) {
            if (chdir(home) == -1) {
                perror("chdir");
                return 0;
            }
        } else {
            fprintf(stderr, "cd: No home directory found\n");
            return 0;
        }
    }
    return 1;
}

int execute_alias(char** args) {
    if (args[1] != NULL && args[2] != NULL) {
        const char* name = args[1];
        const char* value = args[2];

        /* Check if the alias already exists */
        size_t i;
        for (i = 0; i < num_aliases; ++i) {
            if (strcmp(aliases[i].name, name) == 0) {
                free(aliases[i].value);
                aliases[i].value = strdup(value);
                return 1;
            }
        }

        /* Create a new alias */
        aliases = realloc(aliases, (num_aliases + 1) * sizeof(Alias));
        aliases[num_aliases].name = strdup(name);
        aliases[num_aliases].value = strdup(value);
        ++num_aliases;
    } else {
        fprintf(stderr, "Usage: alias <name> <value>\n");
        return 0;
    }
    return 1;
}

void remove_alias(const char* name) {
    size_t i;
    for (i = 0; i < num_aliases; ++i) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].name);
            free(aliases[i].value);
            memmove(&aliases[i], &aliases[i + 1], (num_aliases - i - 1) * sizeof(Alias));
            --num_aliases;
            aliases = realloc(aliases, num_aliases * sizeof(Alias));
            break;
        }
    }
}

int execute_unalias(char** args) {
    if (args[1] != NULL) {
        remove_alias(args[1]);
    } else {
        fprintf(stderr, "Usage: unalias <name>\n");
        return 0;
    }
    return 1;
}

char* expand_variables(char* command) {
    char* expanded = strdup(command);
    char* token = strtok(expanded, " ");
    while (token != NULL) {
        /* Handle variable replacement for $? */
        if (strcmp(token, "$?") == 0) {
            char exit_code_str[32];
            snprintf(exit_code_str, sizeof(exit_code_str), "%d", WEXITSTATUS(system(NULL)));
            free(token);
            token = strdup(exit_code_str);
        }
        /* Handle variable replacement for $$ */
        else if (strcmp(token, "$$") == 0) {
            pid_t pid = getpid();
            char pid_str[32];
            snprintf(pid_str, sizeof(pid_str), "%d", pid);
            free(token);
            token = strdup(pid_str);
        }
        token = strtok(NULL, " ");
    }
    return expanded;
}

size_t i; /* Declear variable*/

void execute_command(char* command) {
    char** args = NULL;
    char* expanded_command = expand_variables(command);
    char* token = strtok(expanded_command, " ");
    size_t num_args = 0;
    
    while (token != NULL) {
        args = realloc(args, (num_args + 1) * sizeof(char*));
        args[num_args] = strdup(token);
        ++num_args;
        token = strtok(NULL, " ");
    }

    if (num_args > 0) {
        char* cmd = args[0];
        if (strcmp(cmd, "exit") == 0) {
            execute_exit(0);
        } else if (strcmp(cmd, "setenv") == 0) {
            execute_setenv(args);
        } else if (strcmp(cmd, "unsetenv") == 0) {
            execute_unsetenv(args);
        } else if (strcmp(cmd, "cd") == 0) {
            execute_cd(args);
        } else if (strcmp(cmd, "alias") == 0) {
            execute_alias(args);
        } else if (strcmp(cmd, "unalias") == 0) {
            execute_unalias(args);
        } else {
            pid_t child_pid = fork();
            if (child_pid == 0) {
                /* Child process */
                execvp(cmd, args);
                perror("execvp");
                exit(1);
            } else if (child_pid > 0) {
                /* Parent process */
                int status;
                waitpid(child_pid, &status, 0);
            } else {
                /* Fork failed */
                perror("fork");
            }
        }
    }

    /* Clean up allocated memory */
    free(expanded_command);
    for (i = 0; i < num_args; ++i) {
        free(args[i]);
    }
    free(args);
}

void cleanup_aliases() {
    for (i = 0; i < num_aliases; ++i) {
        free(aliases[i].name);
        free(aliases[i].value);
    }
    free(aliases);
}

int main() {
    char* command = NULL;
    ssize_t bytes_read;

    while (1) {
        printf("Enter a command: ");
        fflush(stdout);

        bytes_read = read_command(&command);
        if (bytes_read == -1) {
            break;
        } else if (bytes_read == 0) {
            continue;
        }

        execute_command(command);

        free(command);
        command = NULL;
    }

    cleanup_aliases();

    return 0;
}

