#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 1024

void handle_sigint(int signal);
void free_tokens(char **tokens);
char **split_input(char *input);
char *read_input(void);
int execute_command(char **args);



#endif /* MAIN_H */

