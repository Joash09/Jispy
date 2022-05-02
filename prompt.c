/* System Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <editline.h>
#include <string.h>
#include <math.h>

/* Included libraries */
#include "mpc.h"
#include "lval.h"

static char input[2048]; // Global input buffer

int main(int argc, char** argv) {

        // Define Parsers
        mpc_parser_t* Number = mpc_new("number");
        mpc_parser_t* Symbol = mpc_new("symbol");
        mpc_parser_t* Sexpression = mpc_new("sexpression");
        mpc_parser_t* Expression = mpc_new("expression");
        mpc_parser_t* Lisps = mpc_new("lisps"); // Overall rule for Lisp line

        // Define Grammar
        mpca_lang(MPCA_LANG_DEFAULT, " \
number: /-?[0-9]+(\\.[0-9]+)?/ ; \
symbol: '+' | '-' | '*' | '/' | '^' ; \
sexpression: '(' <expression>* ')' ; \
expression: <number> | <symbol> | <sexpression> ; \
lisps: /^/ <expression>* /$/ ; \
        ",
        Number, Symbol, Sexpression, Expression, Lisps);

    puts("Joash's Lisp (Jispy) Version 0.0.1");
    puts("Press Ctrl-C to exit");

    while(1) {

        char* input = readline("j-lispy > ");
        add_history(input);

        // Parse and evaluate input
        mpc_result_t* r;
        if (mpc_parse("<stdin>", input, Lisps, r)) {
            lval* input_lval = eval_sexpression(lval_read(r->output));
            lval_println(input_lval);
            lval_del(input_lval);
            mpc_ast_delete(r->output);

        } else {
            mpc_err_print(r->error);
            mpc_err_delete(r->error);
        }

        free(input);

    }

    mpc_cleanup(5, Number, Symbol, Sexpression, Expression, Lisps);

    return 0;

}
