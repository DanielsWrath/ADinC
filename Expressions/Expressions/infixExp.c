/* prefixExp.c, Gerard Renardel, 29 January 2014
 *
 * In this file functions are defined for the construction of expression trees
 * from prefix expressions generated by the following BNF grammar:
 *
 * <prefexp>   ::= <number> | <identifier> | '+' <prefexp> <prefexp>
 *             | '-' <prefexp> <prefexp> | '*' <prefexp> <prefexp> | '/' <prefexp> <prefexp>
 *
 * <number>      ::= <digit> { <digit> }
 *
 * <identifier> ::= <letter> { <letter> | <digit> }
 *
 * Starting pount is the token list obtained from the scanner (in scanner.c).
 */



/* OUR GRAMMAR
 *
 * <¿¿complete??>   ::= '(' <infexp> { <infexp> } ')' { '(' <infexp> { <infexp> } ')'
 | <infexp> { <infexp> }
 *
 * <infexp>         ::= <number> | <identifier> | <infexp> '+' <infexp> | <infexp> '-' <infexp>
 | <infexp> '*' <infexp> | <infexp> '/' <infexp>
 *
 * <number>         ::= <digit> { <digit> }
 *
 * <identifier>     ::= <letter> { <letter> | <digit> }
 *
 *
 */
#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc, free */
#include <assert.h> /* assert */
#include "scanner.h"
#include "recognizeExp.h"
#include "evalExp.h"
#include "infixExp.h"

#define LEFT 0
#define RIGHT 1

int change = 1;

/* The function newExpTreeNode creates a new node for an expression tree.
 */

ExpTree newExpTreeNode(TokenType tt, Token t, ExpTree tL, ExpTree tR) {
    ExpTree new = malloc(sizeof(ExpTreeNode));
    assert (new!=NULL);
    new->tt = tt;
    new->t = t;
    new->left = tL;
    new->right = tR;
    return new;
}

/* The function valueIdentifier recognizes an identifier in a token list and
 * makes the second parameter point to it.
 */

int valueIdentifier(List *lp, char **sp) {
    if (*lp != NULL && (*lp)->tt == Identifier ) {
        *sp = ((*lp)->t).identifier;
        *lp = (*lp)->next;
        return 1;
    }
    return 0;
}

/* The function valueOperator recognizes an arithmetic operator in a token list
 * and makes the second parameter point to it.
 * Here the auxiliary function isOperator is used.
 */

int isOperator(char c) {
    return ( c == '+' || c == '-' || c == '*' || c == '/');
}

int valueOperator(List *lp, char *cp) {
    if (*lp != NULL && (*lp)->tt == Symbol && isOperator(((*lp)->t).symbol) ) {
        *cp = ((*lp)->t).symbol;
        return 1;
    }
    return 0;
}


/* The function freeExpTree frees the memory of the nodes in the expression tree.
 * Observe that here, unlike in freeList, the strings in indentifier nodes
 * are not freed. The reason is that the function newExpTree does not allocate
 * memory for strings in nodes, but only a pointer to a string in a node
 * in the token list.
 */

void freeExpTree(ExpTree tr) {
    if (tr==NULL) {
        return;
    }
    freeExpTree(tr->left);
    freeExpTree(tr->right);
    free(tr);
}

/* The function treeExpression tries to build a tree from the tokens in the token list
 * (its first argument) and makes its second argument point to the tree.
 * The return value indicates whether the action is successful.
 * Observe that we use ordinary recursion, not mutual recursion.
 */
/*
 int treePrefixExpression(List *lp, ExpTree *tp) {
 double w;
 char *s;
 Token t;
 ExpTree tL, tR;
 if ( valueNumber(lp,&w) ) {
 t.number = (int)w;
 *tp = newExpTreeNode(Number, t, NULL, NULL);
 return 1;
 }
 if ( valueIdentifier(lp,&s) ) {
 t.identifier = s;
 *tp = newExpTreeNode(Identifier, t, NULL, NULL);
 return 1;
 }
 if ( valueOperator(lp,&c) && treePrefixExpression(lp,&tL) ) {
 if ( treePrefixExpression(lp,&tR) ) {
 t.symbol = c;
 *tp = newExpTreeNode(Symbol, t, tL, tR);
 return 1;
 } else { /* withuot 'else' the program works fine, but there is a memory leak *
 freeExpTree(tL);
 return 0;
 }
 }
 return 0;
 }
 */

/* !!!EIGEN CODE!!! */
int isChar(char c1, char c2) {
    return ( c1 == c2 );
}

int isPar(List *lp, int option) {
    if (option==1) {
        return (((*lp)->t).symbol== '(');
    }
    else {
        if(*lp!=NULL){
            return (((*lp)->t).symbol == ')');
        }
    }
    return 0;
}


int treeInfixFactor(List *lp, ExpTree *tp){
    double w;
    char *s;
    Token t;
    if ( valueNumber(lp,&w) ) {
        t.number=w;
        *tp = newExpTreeNode(Number, t, NULL, NULL);
        
        return 1;
    } else if ( valueIdentifier(lp,&s) ) {
        t.identifier = s;
        *tp = newExpTreeNode(Identifier, t, NULL, NULL);
        return 1;
    } else if (isPar(lp, 1)){
        *lp = (*lp)->next;
        if ((*lp)==NULL) {
            return 0;
        }
        if(treeInfixExpression(lp, tp) && ((*lp)!=NULL) && isPar(lp, 2)) {
            *lp = (*lp)->next;
            /*printf("fdsa\n");*/
            return 1;
        } else {return 0;}
    } else {
        *lp = (*lp)->next;
        /*printf("factor is fout");*/
        return 0;
    }
    return -1;
    
    
}

int treeInfixTerm (List *lp, ExpTree *tp, char c) {
    Token t;
    ExpTree tR;
    if ((*lp)==NULL) {
        return 0;
    }
    if(!treeInfixFactor(lp, tp)){
        return 0;
    }
    while (valueOperator(lp, &c)&& ( isChar(c, '*') || isChar(c, '/') ) ) {
        *lp = (*lp)->next;
        if ((*lp)==NULL) {
            return 0;
        }
        t.symbol = c;
        if (!treeInfixFactor(lp, &tR)) {
            return 0;
        }
        
        *tp = newExpTreeNode(Symbol, t, *tp, tR);
    }
    return 1;
}

int treeInfixExpression(List *lp, ExpTree *tp) {
    char c = '\0';
    Token t;
    ExpTree tR;
    
    if ((*lp)==NULL) {
        return 0;
    }
    
    if(!treeInfixTerm(lp, tp, c)){
        return 0;
    }
    while (valueOperator(lp, &c) && (isChar(c, '-') || isChar(c,'+') ) ) {
        *lp = (*lp)->next;
        t.symbol = c;
        if (!treeInfixTerm(lp, &tR, c)) {
            return 0;
        }
        *tp = newExpTreeNode(Symbol, t, *tp, tR);
    }
    
    return 1;
    
}


/* OUR GRAMMAR
 *
 * <¿¿complete??>   ::= '(' <infexp> { <infexp> } ')' { '(' <infexp> { <infexp> } ')'
 | <infexp> { <infexp> }
 *
 * <infexp>         ::= <number> | <identifier> | <infexp> '+' <infexp> | <infexp> '-' <infexp>
 | <infexp> '*' <infexp> | <infexp> '/' <infexp>
 *
 * <number>         ::= <digit> { <digit> }
 *
 * <identifier>     ::= <letter> { <letter> | <digit> }
 *
 *
 */


/* OUR GRAMMAR
 *
 * <¿¿complete??>   ::= '(' <infexp> { <infexp> } ')' { '(' <infexp> { <infexp> } ')'
 * | <infexp> { <infexp> }
 */



/* The function printExpTreeInfix does what its name suggests.
 */

void printExpTreeInfix(ExpTree tr) {
    if (tr == NULL) {
        return;
    }
    switch (tr->tt) {
        case Number:
            printf("%d",(tr->t).number);
            break;
        case Identifier:
            printf("%s",(tr->t).identifier);
            break;
        case Symbol:
            printf("(");
            printExpTreeInfix(tr->left);
            printf(" %c ",(tr->t).symbol);
            printExpTreeInfix(tr->right);
            printf(")");
            break;
    }
}


/*
 0 * E, E * 0 and 0/E are simplified to 0 (E/0, 0 == rightNode)
 0 + E, E + 0, E - 0, 1 * E, E * 1 and E/1 are simplified to E.
 */
/* MEMORY LEAKS!! */

ExpTree removeSubTree(ExpTree tp, int side){
    /* side 0 = left, side 1 = right */
    printf("REMOVE\n");
    if (side == LEFT) {
        freeExpTree(tp->right);
        ExpTree t1 = tp->left;
        free(tp);
        return t1;
    }
    freeExpTree(tp->left);
    ExpTree t1 = tp->right;
    free(tp);
    return t1;
}
/*
ExpTree simplify(ExpTree tr){
    if ( (tr->left) == NULL && (tr->right) == NULL) {
        return tr;
    }
    
    (tr->left)=simplify(tr->left);
    (tr->right)=simplify(tr->right);
    
    switch ((tr->t).symbol) {
        case '*':
            if ( (tr->left)->t.number==0 ) {
                
                return removeSubTree(tr, LEFT);
            }
            if ((tr->right)->t.number==0) {
                return removeSubTree(tr, RIGHT);
            }
            if ( (tr->left)->t.number==1 ) {
                return removeSubTree(tr, RIGHT);
            }
            if ((tr->right)->t.number==1) {
                return removeSubTree(tr, LEFT);
            }
            break;
        case '/':
            if ( (tr->left)->t.number==0 ) {
                return removeSubTree(tr, LEFT);
            }
            if ((tr->right)->t.number==1) {
                return removeSubTree(tr,LEFT);
            }
            break;
        case '-':
            if ((tr->right)->t.number==0) {
                return removeSubTree(tr,LEFT);
            }
            break;
        case '+':
            if ( (tr->left)->t.number==0 ) {
                return removeSubTree(tr,RIGHT);
            }
            if ((tr->right)->t.number==0) {
                return removeSubTree(tr, LEFT);
            }
            break;
    }
    return tr;
}
*/

ExpTree simplify(ExpTree tp){
    int leftNumber = -1, rightNumber = -1;
    if (tp->left == NULL && tp->right==NULL) {
        return tp;
    }
    if (tp->left != NULL) {
        (tp->left)=simplify(tp->left);
        leftNumber = tp->left->t.number;
    }
    if (tp->right != NULL) {
        (tp->right)=simplify(tp->right);
        rightNumber = tp->right->t.number;
    }
    
    switch (tp->t.symbol) {
        case '*':
            if (leftNumber == 0 || rightNumber == 1) {
                simplify(tp->left);
                return removeSubTree(tp, LEFT);
            }
            if (rightNumber == 0 || leftNumber == 1) {
                simplify(tp->right);
                return removeSubTree(tp,RIGHT);
            }
            break;
        case '/':
            if (leftNumber == 0 || rightNumber == 1) {
                simplify(tp->left);
                return removeSubTree(tp, LEFT);
            }
            break;
        case '+':
            if (leftNumber == 0) {
                simplify(tp->right);
                return removeSubTree(tp, RIGHT);
            }
            if (rightNumber == 0) {
                simplify(tp->left);
                return removeSubTree(tp, LEFT);
            }
            break;
        case '-':
            if (rightNumber == 0) {
                simplify(tp->left);
                return removeSubTree(tp, RIGHT);
            }
            break;
    }
    return tp;
}

/* The function isNumerical checks for an expression tree whether it represents
 * a numerical expression, i.e. without identifiers.
 */

int isNumerical(ExpTree tr) {
    assert(tr!=NULL);
    if (tr->tt==Number) {
        return 1;
    }
    if (tr->tt==Identifier) {
        return 0;
    }
    return (isNumerical(tr->left) && isNumerical(tr->right));
}

/* The function valueExpTree computes the value of an expression tree that represents a
 * numerical expression.
 */

double valueExpTree(ExpTree tr) {  /* precondition: isNumerical(tr)) */
    double lval, rval;
    assert(tr!=NULL);
    if (tr->tt==Number) {
        return (tr->t).number;
    }
    lval = valueExpTree(tr->left);
    rval = valueExpTree(tr->right);
    switch ((tr->t).symbol) {
        case '+':
            return (lval + rval);
        case '-':
            return (lval - rval);
        case '*':
            return (lval * rval);
        case '/':
            assert( rval!=0 );
            return (lval / rval);
        default:
            abort();
    }
}

/* the function prefExpressionExpTrees performs a dialogue with the user and tries
 * to recognize the input as a prefix expression. When it is a numerical prefix
 * expression, its value is computed and printed.
 */

void prefExpTrees() {
    char *ar;
    List tl, tl1;
    ExpTree t = NULL;
    printf("give an expression: ");
    ar = readInput();
    while (ar[0] != '!') {

        tl = tokenList(ar);
        printList(tl);
        tl1 = tl;
        if ( treeInfixExpression(&tl1,&t) && tl1 == NULL ) {
            /* there should be no tokens left */
            printf("in infix notation: ");
            printExpTreeInfix(t);
            printf("\n");
            ExpTree t2 = t;
            if ( isNumerical(t) ) {
                simplify(&t);
                printf("the value is %g\n",valueExpTree(t));
                printf("simplified: ");
                printExpTreeInfix(t2);
                printf("\n");
                
            } else {
                simplify(&t);
                printf("this is not a numerical expression\n");
                printf("simplified: ");
                printExpTreeInfix(t2);
                printf("\n");
            }
        } else {
            printf("this is not an expression\n");
        }
        freeExpTree(t);
        t = NULL;
        freeTokenList(tl);
        free(ar);
        printf("\ngive an expression: ");
        ar = readInput();
    }
    free(ar);
    printf("good bye\n");
}
