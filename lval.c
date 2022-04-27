#include "lval.h"
#include "mpc.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** "Constructors"
 */

lval* lval_num(float num) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->value = num;
    return v;
}

lval* lval_error(char* err) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERROR;
    v->err = malloc(strlen(err)+1); // Allocate size of string first
    strcpy(v->err, err); // Then copy
    return v;
}

lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(sizeof(s)+1);
    strcpy(v->sym, s);
    return v;
}

lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/*
** Destructor
*/
void lval_del(lval* v) {

    switch(v -> type) {
        case LVAL_NUM:
            break;
        case LVAL_ERROR: {
            free(v->err);
        } break;
        case LVAL_SYM: {
            free(v->sym);
        } break;
        case LVAL_SEXPR: {
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
        } break;
    }

    free(v);
}

/*
** Reading Expressions
**
** Create lval depedning on tag after parsing (mpc_ast_t)
*/
lval* lval_read(mpc_ast_t* t) {

    // If just number or symbol return lval object
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    // If empty line; create s-expression
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strcmp(t->tag, "sexpression")) { x = lval_sexpr(); }

    // Handle the children of abstract syntax tree type
    for (int i = 0; i < t->children_num; i++) {

        // Handle
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }

        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

/*
** Simple construct lval with number type
*/
lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    float x = strtof(t->contents, NULL);
    return errno != ERANGE ? lval_num(x): lval_error("invalid number");
}

/*
** Add element to children of S-Expression (a)
**
 */
lval* lval_add(lval* a, lval* b) {

    a->count++;
    a->cell = realloc(a->cell, (sizeof(lval*) * a->count));
    a->cell[a->count-1] = b;
    return a;

}


/*
** Recursive funtion which returns either number or the result of expression
** In a simple lisp + 2 3
** The first child is tagged regex
** The second child is tagged operator (i.e. +)
** Third and Fourth child is tagged expression | number
*/
/* lval* eval_sexpression(lval* t) { */
/*     // Evaluate children */
/*     for (int i = 0; i < t->count; i++) { */
/*         t->cell[i] = lval_eval(t->cell[i]); */
/*     } */
/* } */

/* lval* eval(mpc_ast_t* t) { */

/*     if (strstr(t->tag, "number")) { */
/*         return lval_num(atof(t->contents)); */
/*     } */

/*     // else is this has operators and expressions */
/*     char *op = t->children[1]->contents; */

/*     lval* x = eval(t->children[2]); */

/*     int i = 3; */
/*     while(strstr(t->children[i]->tag, "expression")) { */
/*         x = eval_op(x, eval(t->children[i]), op); */
/*         i++; */
/*     } */

/*     return x; */
/* } */


/* lval* eval_op(lval x, lval y, char* op) { */

/*     if(x.type == LVAL_ERROR) { return x; } */
/*     if(y.type == LVAL_ERROR) { return y; } */

/*     if (strstr(op, "+")) { return lval_num(x.value + y.value); } */
/*     if (strstr(op, "-")) { return lval_num(x.value - y.value); } */
/*     if (strstr(op, "*")) { return lval_num(x.value * y.value); } */
/*     if (strstr(op, "/")) { return y.value == 0 ? lval_error("Cannot divide by zero") : lval_num(x.value/y.value); } */
/*     if (strstr(op, "^")) { return lval_num(pow(x.value, y.value)); } */
/*     // if (strstr(op, "%")) { return modff(x, y); } */
/*     // */
/*     return lval_error("Unknown operator"); */
/* } */

/*
** Methods
*/
void lval_print(lval* p) {
    switch(p->type) {
        case LVAL_NUM: {
            printf("%f", p->value);
        }
        break;
        case LVAL_ERROR: {
            printf("Error: %s", p->err);
        }
        break;
        case LVAL_SYM: {
            printf("%s", p->sym);
        }
        break;
        case LVAL_SEXPR: {
            lval_print_sexpr(p);
        }
        break;
    }
}

void lval_print_sexpr(lval* p) {

    putchar('(');

    for(int i = 0; i < p->count; i++) {
        lval_print(p->cell[i]);
        putchar(' ');
    }

    putchar(')');

}

void lval_println(lval *p) {
    lval_print(p);
    putchar('\n');
}
