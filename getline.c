#include "main.h"

#define BUFFER_SIZE 1024

char *custom_getline(void)
{
    static char buffer[BUFFER_SIZE];
    static int buffer_index = 0;
    static int chars_in_buffer = 0;
    static int end_of_file = 0;
    char *line = NULL;
    int line_index = 0;
    char current_char;
    while (1) {
        if (buffer_index >= chars_in_buffer) {
            /* Read more characters into the buffer*/
            chars_in_buffer = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (chars_in_buffer <= 0) {
                /* End of file or error encountered*/
                end_of_file = 1;
                break;
            }
            buffer_index = 0;
        }
        
        current_char = buffer[buffer_index++];
        
        if (current_char == '\n') {
            /* Found end of line*/
            line[line_index++] = '\0';
            break;
        }
        
        /* Append the character to the line*/
        line = realloc(line, (line_index + 1) * sizeof(char));
        line[line_index++] = current_char;
    }
    
    if (end_of_file && line_index == 0) {
        /* End of file reached, return NULL to indicate no more input*/
        return NULL;
    }
    
    return line;
}

