/* System Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <editline.h>
#include <string.h>

/* Included libraries */
#include "mpc.h"

float eval(mpc_ast_t* t);
float eval_op(float x, float y, char* op);

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

        // Parse and evaluate input
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lisps, &r)) {

            float result = eval(r.output);
            printf("%f\n", result);
            mpc_ast_delete(r.output);

        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }


        free(input);

    }

    return 0;

}

// Recursive funtion which returns either number or the result of expression
// In a simple lisp + 2 3
// The first child is tagged regex
// The second child is tagged operator (i.e. +)
// Third and Fourth child is tagged expression | number
float eval(mpc_ast_t* t) {

    if (strstr(t->tag, "number")) {
        return atof(t->contents);
    }

    // else is this lisp
    char *op = t->children[1]->contents;

    float x = eval(t->children[2]);

    int i = 3;
    while(strstr(t->children[i]->tag, "expression")) {
        x = eval_op(x, eval(t->children[i]), op);
        i++;
    }

    return x;
}

float eval_op(float x, float y, char* op) {
    if (strstr(op, "+")) { return x + y; }
    if (strstr(op, "-")) { return x - y; }
    if (strstr(op, "*")) { return x * y; }
    if (strstr(op, "/")) { return x / y; }
    return 0;
}
