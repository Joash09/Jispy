/* System Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <editline.h>
#include <string.h>
#include <math.h>

/* Included libraries */
#include "mpc.h"

typedef struct {
    int type;
    float value;
    int error;
} lval;

enum LVAL_TYPE { LVAL_NUM, LVAL_ERROR };
enum LERR_TYPE { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval eval(mpc_ast_t* t);
lval eval_op(lval x, lval y, char* op);
lval lval_num(float num);
lval lval_error(int error);
void lval_print(lval p);

static char input[2048]; // Global input buffer

int main(int argc, char** argv) {

        // Define Parsers
        mpc_parser_t* Number = mpc_new("number");
        mpc_parser_t* Operator = mpc_new("operator");
        mpc_parser_t* Expression = mpc_new("expression");
        mpc_parser_t* Lisps = mpc_new("lisps"); // Overall rule for Lisp line

        // Define Grammar
        mpca_lang(MPCA_LANG_DEFAULT, " \
number: /-?[0-9]+(\\.[0-9]+)?/ ; \
operator: '+' | '-' | '*' | '/' | '^'; \
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

            lval result = eval(r.output);
            lval_print(result);
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
lval eval(mpc_ast_t* t) {

    if (strstr(t->tag, "number")) {
        return lval_num(atof(t->contents));
    }

    // else is this has operators and expressions
    char *op = t->children[1]->contents;

    lval x = eval(t->children[2]);

    int i = 3;
    while(strstr(t->children[i]->tag, "expression")) {
        x = eval_op(x, eval(t->children[i]), op);
        i++;
    }

    return x;
}

lval eval_op(lval x, lval y, char* op) {

    if(x.type == LVAL_ERROR) { return x; }
    if(y.type == LVAL_ERROR) { return y; }

    if (strstr(op, "+")) { return lval_num(x.value + y.value); }
    if (strstr(op, "-")) { return lval_num(x.value - y.value); }
    if (strstr(op, "*")) { return lval_num(x.value * y.value); }
    if (strstr(op, "/")) { return y.value == 0 ? lval_error(LERR_DIV_ZERO) : lval_num(x.value/y.value); }
    if (strstr(op, "^")) { return lval_num(pow(x.value, y.value)); }
    // if (strstr(op, "%")) { return modff(x, y); }
    //
    return lval_error(LERR_BAD_OP);
}

lval lval_num(float num) {
    lval v;
    v.type = LVAL_NUM;
    v.value = num;
    return v;
}

lval lval_error(int err) {
    lval v;
    v.type = LVAL_ERROR;
    v.error = err;
    return v;
}

void lval_print(lval p) {
    switch(p.type) {

        case LVAL_NUM: {
            printf("%f\n", p.value);
        }
        break;
        case LVAL_ERROR: {
            switch(p.error) {
            case LERR_DIV_ZERO: {
              printf("Cannot divide by zero\n");
            } break;
            case LERR_BAD_NUM: {
              printf("Cannot convert number\n");
            } break;
            case LERR_BAD_OP: {
              printf("Unsupported operator\n");
            } break;
            }
        }
    }
}
