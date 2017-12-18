#ifndef __COMMON__
#define __COMMON__
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define _STR(x)        #x
#define _STR2(x)       _STR(x)
#define ERROR_LOCATION fprintf(stderr,"EXIT FAILURE at ");fprintf(stderr, __FILE__ ":" _STR2(__LINE__) ": %s()\n",__func__);
#define EXIT_NOMSG     {ERROR_LOCATION                    ;exit(EXIT_FAILURE);}
#define EXIT_MSG(x)    {ERROR_LOCATION fprintf(stderr,x)  ;exit(EXIT_FAILURE);}
#define EXIT_VAR(x,y)  {ERROR_LOCATION fprintf(stderr,x,y);exit(EXIT_FAILURE);}

#define TOKEN_STRING_MAX_LENGTH 32
#define MAX_NODE_NUMBER 10000
#define MAX_HASH_NUMBER 500
#define INT_SIZE        4 // (byte)

typedef enum {// TOKEN_KIND/*{{{*/
  OTHER_TOKEN     ,
  TERM_OPERATOR   , // "+"|"-"
  FACTOR_OPERATOR , // "*"|"/"|"%"
  COMPARE_OPERATOR, // "=="|"!="|">"|">="|"<"|"<="
  EQUAL           , // "="
  COMMA           , // ","
  SEMICOLON       , // ";"
  LPAREN          , // "("
  RPAREN          , // ")"
  LBRACKET        , // "{"
  RBRACKET        , // "}"
  IF_KEYWORD      , // "if"
  ELSE_KEYWORD    , // "else"
  WHILE_KEYWORD   , // "while"
  RETURN_KEYWORD  , // "return"
  TYPE_KEYWORD    , // "int"
  NUMBER          , // CHAR_NUMBER(CHAR_NUMBER)*
  IDENTIFY          // CHAR_ALPHABET(CHAR_NUMBER|CHAR_ALPHABET)*
} TOKEN_KIND;/*}}}*/
typedef enum {// SYNTAX_KIND/*{{{*/
  OTHER_NODE,
  SKIP_NODE ,
  PROGRAM   , // (V_DEC ";" | F_DEC | PROTOTYPE ";")*
  V_DEC     , // TYPE VARIABLE
  F_DEC     , // TYPE VARIABLE "(" (\ep | V_DEC (, V_DEC)*) ")" FUNCTION
  PROTOTYPE , // TYPE VARIABLE "(" (\ep | V_DEC (, V_DEC)*) ")"
  FUNCTION  , // "{" (V_DEC ";")* SENTENCE* "}"
  SENTENCE  , //  (";" | "{" SENTENCE* "}" | IF_FLOW | WHILE_FLOW | RETURN | CALL ";" | ASSIGN)
  IF_FLOW   , // "if" "(" EXPRESSION ")" "{" SENTENCE* "}" (\ep |"else" "{" SENTENCE "}")
  WHILE_FLOW, // "while" "(" EXPRESSION ")" "{" SENTENCE* "}"
  RETURN    , // "return" EXPRESSION ";"
  ASSIGN    , // VARIABLE "=" EXPRESSION ";"
  CALL      , // VARIABLE "(" (\ep | ARGUMENTS) ")"
  ARGUMENTS , // (\ep | EXPRESSION ("," EXPRESSION)*)
  EXPRESSION, // FORMULA (%COMPARE_OPERATOR% FORMULA)*
  FORMULA   , // (\ep | %TERM_OPERATOR% ) TERM ( %TERM_OPERATOR% TERM)*
  TERM      , // FACTOR (\ep|%FACTOR_OPERATOR% FACTOR)*
  FACTOR    , // (IMMEDIATE | CONDITION | CALL | VARIABLE)
  B_OPERATOR, // (%TERM_OPERATOR% | %FACTOR_OPERATOR% | %COMPARE_OPERATOR%)
  IMMEDIATE , // %NUMBER%
  VARIABLE  , // %IDENTIFY%
  TYPE      , // %TYPE_KEYWORD%
} SYNTAX_KIND;/*}}}*/
typedef struct {// TOKEN/*{{{*/
  int kind;
  char str[TOKEN_STRING_MAX_LENGTH];
} TOKEN;/*}}}*/
typedef struct __STRUCT_NODE__ {// NODE/*{{{*/
  int kind;
  int init;
  int last;
  bool arrived;
  struct __STRUCT_NODE__ *left, *right, *up, *down;
} NODE;/*}}}*/
typedef struct {// SOURCE/*{{{*/
  NODE  *program;
  int    max_node_num;
  TOKEN *token;
  int    max_token_num;
} SOURCE;/*}}}*/
typedef struct {// TABLE/*{{{*/
  bool  used;
  char *str;         // token.str へのポインタ
  NODE *node;        // 宣言のノード
  int   kind;        // V_DEC or F_DEC
  int   type;        // intなら0
  int   size;        // V_DEC intなら4byteなので4
  int   token_index; // 変数を参照する際に、未宣言変数でないかチェックする用
  NODE *root;        // 宣言の存在する関数 (グローバルなら &(node[0]))
  char *root_name;   // 宣言の存在する場所の名前
  int   block;       // 宣言の存在するブロックの深さ (引数なら0, 関数内の宣言なら1, if文内の宣言なら2)
  int   address;     // ローカル変数なら [ebp-address] グローバル変数なら [address]
} TABLE;/*}}}*/

#endif
