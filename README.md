# 🖥 Creating my own Lisp language

In an effort to learn how programming languages are actually designed, I stumbled across this [https://buildyourownlisp.com](resource) which I thought is worth replicating. This repo is a recreation of this project with modifications from me where possible. 

# 🏃 How to run

``` sh
make
make run
```

# 📓 Personal Notes

### Chapter 5

Natural languages are build up of repeated substructures. Examples of these structures include combining nouns with "and" or describing an object by placing an adjective before the noun (e.g. grey cat). Languages therefore can be described as a infinite set of rules referred to as "re-write" structures. These rules form the grammar of a language. Programming languages also have grammar which define whether input from the use makes sense. To define the grammar of a programming language we will make use of a <em>parser</em>. 

Personally, I feel like there is a resemblence to first order logic; a subject which I've been meaning to delve deeper into. 

### Chapter 6

Useful [https://regexone.com/problem/matching_decimal_numbers](resource) for Regex expressions.

### Chapter 7 : Evaluation

After parsing, the user's input can be represented in a tree structure. From there we can recursively evaluate the leaf elements. 

### Chapter 8

Useful to represent values as structs since they can either take on value type or error type

``` c++
typedef struct {
    int type;
    float value;
    int error;
} lval;

enum lval_type { LVAL_NUM, LVAL_ERROR };

enum LERR_TYPE { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };
```

### Chapter 9 : S-Expressions

So far Lisp code has been written as such:

``` common-lisp
+ 2 3
* 3 -1
- 3 23
```

In the Lisp language we can also represent lists of either data or code; as such:

``` common-lisp
+ (* 2 3)
- (+ 3 4) 7)
```

The operators and numbers are referred to as "atoms" and represent the functions to be computed and data for which the functions need to be applied. A list is a sequence of atoms enclosed by parenthesis. An S-Expression represents these nested lists.

Additional [resource](https://www.cs.unm.edu/~luger/ai-final2/LISP/CH%2011_S-expressions,%20The%20Syntax%20of%20Lisp.pdf) for understanding S-Expressions.

We refactor the lval struct to represent s-expressions by:

* lval has a string representing symbols (i.e. + - etc.)
* lval has an array of child lval types and int tracking the number of children
* There is an additional lval type to represent a S-Expression

``` c++
typedef struct lval {
    int type;
    
    float value;
    char* err;
    char* sym;

    int count; // Stores length of cell list
    struct lval** cell;
    
} lval;

enum LVAL_TYPE { LVAL_NUM, LVAL_ERROR, LVAL_SYM, LVAL_SEXPR };
```

We could have also used a linked list (a lval object contains a pointer to next lval item) instead of a variable array.

### Chapter 10: Quoted expressions

One of the most talked about features of Lisp, which is that Lisp can treat code as data. Quoted expressions tell the interpreter not to evaluate the symbols, S-expressions etc. within a quoted expression. Quoted expressions do not exist in other Lisp implementations but they have <em>MACROS</em> (such as ') to tell the interpreter not to evaluate that list. Quoted expressions in this project are denoted with curly brackets "{}".

Adding in builtin methods (map to keywords in language), to use on q-expressions: 

* builtin_head: Return q-expression with only first element
* builtin_tail: Returen q-expression with first element removed
* builtin_list: Take s-expression and convert to q-expression
* builtin_join: Joins q-expressions together
* builtin_eval: Evaluate whatever is in the quoted expression

* lval_join: Helper to builtin_join

* builtin: calls the correct builtin function depending on symbol

### General C Notes (mostly memory management 😆)

* Structs always has a fixed size (i.e. can't use a struct for a list)
* Lists that can vary in size are represented by an address which points to the element of the first element within that list
* When allocating memory for a string use sizeof(string)+1 to include the null terminating character
* <em>memmove</em> copies data from source to destination as if intermediate buffer used. This is useful in the case the source and destination memory addresses overlap (i.e moving array element to another position in array) since as the move function copies data to the destination it won't read values it has just written. Safer option than <em>memcopy</em>
* When adding/removing elements from dynamic array remember to reallocate the memory according with <em>realloc</em>

* MACROS can be used to clean up code. Eg when throwing errors:

```c
#define LASSERT(args, cond, err) \
if (!(cond)) { lval_del(args); return lval_error(err); } 
```
