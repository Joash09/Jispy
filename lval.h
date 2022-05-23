#ifndef LVAL_H_
#define LVAL_H_

#include "mpc.h"

/* Forward declarations */
struct lenv;
struct lval;

typedef struct lval lval;
typedef struct lenv lenv;

enum LVAL_TYPE { LVAL_NUM, LVAL_ERROR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN };

typedef lval*(*lbuiltin)(lenv*, lval*);

/* Defining struct */
struct lenv {
    int count;
    char** syms;
    lval** vals;
};

struct lval {
    int type;

    float value;
    char* err;
    char* sym;

    lbuiltin fun;

    int count; // Stores length of cell list
    struct lval** cell;
};

/* Constructors */
lenv* lenv_new();

lval* lval_num(float num);
lval* lval_error(char* err);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin f);

/* Destructor */
void lenv_del(lenv* e);
void lval_del(lval* v);

/* Environment methods */
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv*e, lval* k, lval* v);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);

/* Reading Expressions */
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_add(lval* a, lval* b);
lval* lval_copy(lval* a);

/* Evaluating Expressions */
lval* lval_eval(lenv* e, lval* t);
lval* eval_sexpression(lenv* e, lval* t);

/* Built in operators */
lval* builtin_op(lenv* e, lval* v, char* op);

lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);

lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);

/* S-Expression Evaluation helpers */
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);

/* Q-Expression Evaluation helpers */
lval* lval_join(lval* x, lval* y);

/* Print Methods */
void lval_print(lval* p);
void lval_print_sexpr(lval* p);
void lval_print_qexpr(lval* p);
void lval_println(lval* p);

#endif // LVAL_H_
