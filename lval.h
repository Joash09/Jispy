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

enum LVAL_TYPE { LVAL_NUM, LVAL_ERROR, LVAL_SYM, LVAL_SEXPR };

/* Constructors */
lval* lval_num(float num);
lval* lval_error(char* err);
lval* lval_sym(char* s);
lval* lval_sexpr(void);

/* Destructor */
void lval_del(lval* v);

/* Reading Expressions */
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_add(lval* a, lval* b);

/* Evaluating Expressions */
lval* eval(mpc_ast_t* t);
lval* eval_op(lval x, lval y, char* op);

/* Methods */
void lval_print(lval* p);
void lval_print_sexpr(lval* p);
void lval_println(lval* p);

#endif // LVAL_H_
