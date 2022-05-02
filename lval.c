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
  switch (v->type) {
    /* Do nothing special for number type */
    case LVAL_NUM: break;

    /* For Err or Sym free the string data */
    case LVAL_ERROR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;

    /* If Sexpr then delete all elements inside */
    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      /* Also free the memory allocated to contain the pointers */
      free(v->cell);
    break;
  }

  /* Free the memory allocated for the "lval" struct itself */
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
lval* eval_sexpression(lval* t) {

    // Evaluate children
    for (int i = 0; i < t->count; i++) {
        t->cell[i] = lval_eval(t->cell[i]);
    }

    // Don't bother with the rest if there are any errors
    for (int i = 0; i < t->count; i++) {
        if (t->cell[i]->type == LVAL_ERROR) {
            return lval_take(t, i);
        }
    }

    // Empty expression (i.e. '()')
    if (t->count == 0) {
        return t;
    }

    // If there is only one expression; return only that one expression
    if(t->count == 1) {
        return lval_take(t, 0);
    }

    // Ensure first element is a symbol
    lval* f = lval_pop(t, 0);
    if(f->type != LVAL_SYM) {
        lval_del(f); lval_del(t);
        return lval_error("S-Expression does not start with symbol");
    }

    lval* result = builtin_op(t, f->sym);
    lval_del(f);
    return result;

}

lval* lval_eval(lval* t) {
    if (t->type == LVAL_SEXPR) {
        return eval_sexpression(t);
    }
    return t;
}

lval* builtin_op(lval* v, char* op) {

    /* Ensure all children are numbers */
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type != LVAL_NUM) {
            lval_del(v);
            return lval_error("Cannot operate on a non-number");
        }
    }

    lval* x = lval_pop(v, 0);

    if((strcmp(op, "-") == 0) && v->count == 0) {
        x->value = -x->value;
    }

    while(v->count > 0) {

        lval* y = lval_pop(v, 0);

        if (strcmp(op, "+") == 0) {
            x->value += y->value;
        }
        if (strcmp(op, "-") == 0) {
            x->value -= y->value;
        }
        if (strcmp(op, "*") == 0) {
            x->value *= y->value;
        }
        if (strcmp(op, "/") == 0) {
            if (y->value == 0) {
                lval_del(x);
                lval_del(y);
                lval_error("Cannot divide by zero");
                break;
            }
            x->value /= y->value;
        }
        if (strcmp(op, "^") == 0) {
            x->value = pow(x->value, y->value);
        }

        lval_del(y);

    }

    lval_del(v);
    return x;
}


/* Pop the child of lval at index i */
lval* lval_pop(lval* v, int i) {

    lval* x = v->cell[i];

    // Shifting the address of i+1 back to i
    // We are shifting back size of lval struct in bytes multiplied by the rest of elements in array
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));

    // decrease count
    v->count--;

    // Resize the memory allocated to dynamic array
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);

    return x;
}

/* Gets child element then deletes parent */
lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

/*
** Methods
*/
void lval_print(lval* p) {
    switch(p->type) {
        case LVAL_NUM: {
            printf("%f", p->value);
            break;
        }
        case LVAL_ERROR: {
            printf("Error: %s", p->err);
            break;
        }
        case LVAL_SYM: {
            printf("%s", p->sym);
            break;
        }
        case LVAL_SEXPR: {
            lval_print_sexpr(p);
            break;
        }
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
