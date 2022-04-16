/* System Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <editline.h>

/* Included libraries */
#include "mpc.h"

static char input[2048];

int main(int argc, char** argv) {

        // Define Parsers
        mpc_parser_t* Number = mpc_new("number");
        mpc_parser_t* Operator = mpc_new("operator");
        mpc_parser_t* Expression = mpc_new("expression");
        mpc_parser_t* Lisps = mpc_new("lisps"); // Overall rule for Lisp line

        // Define Grammar
        mpca_lang(MPCA_LANG_DEFAULT, " \
number: /-?[0-9]+(\\.[0-9]+)?/ ; \
operator: '+' | '-' | '*' | '/' ; \
expression: <number> | '(' <operator> <expression>+ ')' ; \
lisps: /^/ <operator> <expression>+ /$/ ; \
        ",
                  Number, Operator, Expression, Lisps);

    puts("Joash's Lisp (JLisp) Version 0.0.1");
    puts("Press Ctrl-C to exit");

    while(1) {

        char* input = readline("j-lisp > ");
        add_history(input);

        // Parse input
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lisps, &r)) {
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }


        free(input);

    }

    return 0;

}
