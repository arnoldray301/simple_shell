#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *custom_getline(void);

int main(void)
{
    char *input;
    char *prompt = "($) ";

    while (1) {
        printf("%s", prompt);
        
        input = custom_getline();
        
        if (input == NULL) {
            printf("\n");
            break;
        }
        
        printf("You entered: %s\n", input);
        free(input);
    }
    
    return 0;
}

