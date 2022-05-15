#ifndef LVAL_H_
#define LVAL_H_

#include "mpc.h"

typedef struct lval {
    int type;

    float value;
    char* err;
    char* sym;

    int count; // Stores length of cell list
    struct lval** cell;
} lval;

enum LVAL_TYPE { LVAL_NUM, LVAL_ERROR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

/* Constructors */
lval* lval_num(float num);
lval* lval_error(char* err);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

/* Destructor */
void lval_del(lval* v);

/* Reading Expressions */
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_add(lval* a, lval* b);

/* Evaluating Expressions */
lval* lval_eval(lval* t);
lval* eval_sexpression(lval* t);

/* Built in operators */
lval* builtin(lval* v, char* func);
lval* builtin_op(lval* v, char* op);
lval* builtin_head(lval* a);
lval* builtin_tail(lval* a);
lval* builtin_list(lval* a);
lval* builtin_join(lval* a);
lval* builtin_eval(lval* a);

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
