#include "lval.h"
#include "mpc.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LVAL_ASSERT(args, cond, err)                \
    if (!(cond)) { lval_del(args); return lval_error(err); }

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

lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
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

    /* If S-expression or Q-expression then delete all elements inside */
    case LVAL_QEXPR:
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
    if (strstr(t->tag, "sexpression")) { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpression")) { x = lval_qexpr(); }

    // Handle the children of abstract syntax tree type
    for (int i = 0; i < t->children_num; i++) {

        // Handle
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
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

    lval* result = builtin(t, f->sym);
    lval_del(f);
    return result;

}

lval* lval_eval(lval* t) {
    if (t->type == LVAL_SEXPR) {
        return eval_sexpression(t);
    }
    return t;
}

lval* builtin(lval* v, char* func) {

  if (strcmp("list", func) == 0) {
    return builtin_list(v);
  }
  if (strcmp("head", func) == 0) {
    return builtin_head(v);
  }
  if (strcmp("tail", func) == 0) {
    return builtin_tail(v);
  }
  if (strcmp("join", func) == 0) {
    return builtin_join(v);
  }
  if (strcmp("eval", func) == 0) {
    return builtin_eval(v);
  }
  if (strcmp("len", func) == 0) {
      return builtin_len(v);
  }
  if (strcmp("init", func) == 0) {
      return builtin_init(v);
  }
  if (strstr("+-/*", func)) {
    return builtin_op(v, func);
  }

  lval_del(v);
  return lval_error("Unknown function");

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

/* Take q-expression and return q-expression with first element */
lval* builtin_head(lval* a) {

    /* too many arguments */
    LVAL_ASSERT(a, a->count == 1, "Function 'head' passed too many arguments");

    /* not a q-expression */
    LVAL_ASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' not a Q-Expression");

    /* no child elements */
    LVAL_ASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}!");

    lval* v = lval_take(a, 0);

    /* Delete elements not in the head */
    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }

    return v;

}

/* Takes q-expression and return q-expression with first element removed */
lval* builtin_tail(lval* a) {

    LVAL_ASSERT(a, a->count == 1, "Function 'head' passed too many arguments");

    LVAL_ASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' not a Q-Expression");

    LVAL_ASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed");

    lval* v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;

}

/* Convert s-expression to q-expression */
lval* builtin_list(lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

/* Joins q-expressions together */
lval* builtin_join(lval* a) {

    // Make sure all children are q-expressions
    for(int i = 0; i < a->count; i++) {
        LVAL_ASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' incorrect type");
    }

    lval* x = lval_pop(a, 0);
    while(a->count) { // By popping we reduce count
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;

}

/* Evaluate q-expression */
lval* builtin_eval(lval* a) {

    LVAL_ASSERT(a, a->count == 1, "Function 'eval' passed too many arguments");
    LVAL_ASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' wrong type");

    // Convert expression to s-expression then return evaluated result
    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

/* Return number of elements in Q-expression */
lval* builtin_len(lval* a) {

    LVAL_ASSERT(a, a->count == 1, "Function 'eval' passed too many arguments");
    LVAL_ASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' wrong type");

    lval* result = lval_num(lval_pop(a, 0)->count);
    lval_del(a);
    return result;

}

/* Return all but last element of q-expression */
lval* builtin_init(lval* a) {

    lval* result = lval_take(a, 0);
    lval_del(lval_pop(result, result->count-1));
    return result;

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

/* Join 2 q-expressions */
lval* lval_join(lval* x, lval* y) {

    // For each cell in y; add it to x
    while(y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }

    // Delete y
    lval_del(y);
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
        case LVAL_QEXPR: {
            lval_print_qexpr(p);
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

void lval_print_qexpr(lval* p) {

    putchar('{');

    for(int i = 0; i < p->count; i++) {
        lval_print(p->cell[i]);
        putchar(' ');
    }

    putchar('}');
}

void lval_println(lval *p) {
    lval_print(p);
    putchar('\n');
}
