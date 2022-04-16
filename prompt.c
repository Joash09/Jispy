#include <stdio.h>
#include <stdlib.h>
#include <editline.h>


static char input[2048];

int main(int argc, char** argv) {

    puts("Joash's Lisp (JLisp) Version 0.0.1");
    puts("Press Ctrl-C to exit");

    while(1) {

        char* input = readline("j-lisp > ");
        add_history(input);

        printf("You typed %s\n", input);

        free(input);

    }

    return 0;

}
