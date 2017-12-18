#include "../src/tree.c"
#include "../include/lexer.h"

static int mt(TOKEN *t, char *s) {/*{{{*/
  FILE *file;
  file = fopen("tmp.c","w+");
  fprintf(file,"%s",s);
  rewind(file);
  int max = count_token_num(file);
  for (int i=0;i<1000;i++) t[i] = initialize_token();
  for (int i=0;i<max ;i++) t[i] = get_next_token(file);
  fclose(file);
  return max;
}/*}}}*/
void TEST_find_program() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "               int    a   ;   int   func   (   int   b   )   ;";
  // find_* 型関数に共通の解説/*{{{*/
  last_fix=-1+mt(t,A);
  // トークン t[0]~t[9]  =  0int  1a  2;  3int  4func  5(  6int  7b  8)  9;
  // mt()     ... 合計10個なので10
  // last_fix ... 最後のトークン (9;) を指すので mt()-1 = 9

  init=0;        // 最初のトークン 0int を指すので 0
  last=last_fix; // 最後のトークン 9;   を指すので 9
  k=find_program(&init,&last,t); // 0int  1a  2;  3int  4func  5(  6int  7b  8)  9; を左から見てゆく
  // トークン t[0]~t[9]  =  0int  1a  2;  3int  4func  5(  6int  7b  8)  9;
  mu_assert(k == V_DEC);         // 0int  1a がマッチ
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "int")); // init --> 0int
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "a"));   // last --> 1a

  init=last+1;   // init = 2;にする
  last=last_fix; // last = 9;にする
  k=find_program(&init,&last,t); //  2;  3int  4func  5(  6int  7b  8)  9; を左から見てゆく
  // トークン t[0]~t[9]  =  0int  1a  2;  3int  4func  5(  6int  7b  8)  9;
  mu_assert(k == SKIP_NODE);         // 2; がマッチ
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, ";")); // init --> 2;
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, ";")); // last --> 2;

  // 次は 3int  4func  5(  6int  7b  8)  9; を左から調べたい
  init=last+1;   // init = 2a にする
  last=last_fix; // last = 9; にする
  k=find_program(&init,&last,t); // 3int  4func  5(  6int  7b  8)  9; を左から見てゆく
  mu_assert(k == PROTOTYPE);     // 3int  4func  5(  6int  7b  8) がマッチ
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, "int")); // init --> 3int
  mu_assert(last == 8); mu_assert(0==strcmp(t[last].str, ")"));   // last --> 8)

  // 次は 9; を左から調べたい
  init=last+1;   // init = 9; にする
  last=last_fix; // last = 9; にする
  k=find_program(&init,&last,t); // 9; を左から順番に見てゆく
  mu_assert(k == SKIP_NODE);     // マッチしない
  mu_assert(init == 9); mu_assert(0==strcmp(t[init].str, ";")); // マッチしなければ init はそのまま
  mu_assert(last == 9); mu_assert(0==strcmp(t[last].str, ";")); // マッチしなければ last もそのまま
/*}}}*/

  char *B = "int func( int abc ){ de(f); int g; int hi; return 0;}int x;";
  last_fix=-1+mt(t,B);/*{{{*/
  init=     0;last=last_fix;k=find_program(&init,&last,t);
  mu_assert(k == F_DEC);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last ==21); mu_assert(0==strcmp(t[last].str, "}"));

  init=last+1;last=last_fix;k=find_program(&init,&last,t);
  mu_assert(k == V_DEC);
  mu_assert(init ==22); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last ==23); mu_assert(0==strcmp(t[last].str, "x"));

  init=last+1;last=last_fix;k=find_program(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==24); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last ==24); mu_assert(0==strcmp(t[last].str, ";"));
  /*}}}*/
}/*}}}*/
void TEST_find_prototype() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "int func(int a,int bc, int d)";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == TYPE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "int"));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "func"));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == V_DEC);
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, "a"));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, ","));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ","));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == V_DEC);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, "bc"));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 8); mu_assert(0==strcmp(t[init].str, ","));
  mu_assert(last == 8); mu_assert(0==strcmp(t[last].str, ","));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == V_DEC);
  mu_assert(init == 9); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last ==10); mu_assert(0==strcmp(t[last].str, "d"));

  init=last+1;last=last_fix;k=find_prototype(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==11); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last ==11); mu_assert(0==strcmp(t[last].str, ")"));
  /*}}}*/
}/*}}}*/
void TEST_find_f_dec() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "int func(int a,int bc) { int c; while(1==2) {hoge();} return 0;}";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == TYPE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "int"));

  init=last+1;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "func"));

  init=last+1;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == V_DEC);
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, "a"));

  init=last+1;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, ","));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ","));

  init=last+1;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == V_DEC);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, "bc"));

  init=last+1;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 8); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last == 8); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_f_dec(&init,&last,t);
  mu_assert(k == FUNCTION);
  mu_assert(init == 9); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last ==28); mu_assert(0==strcmp(t[last].str, "}"));
  /*}}}*/
}/*}}}*/
void TEST_find_v_dec() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "int a";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_v_dec(&init,&last,t);
  mu_assert(k == TYPE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "int"));

  init=last+1;last=last_fix;k=find_v_dec(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "a"));/*}}}*/
}/*}}}*/
void TEST_find_function() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "         \
    {                 \
      int a;          \
      a=1;            \
      {a=2;}          \
                      \
      while(a==3) {   \
        func(a,4);    \
      }               \
                      \
      if (a<=5) {     \
        a=6;          \
      }               \
                      \
      if (a>=7) {     \
        a=8;          \
      } else {        \
        a=9;          \
      }               \
                      \
      func(a,10);     \
      return a;       \
    }";

  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "{"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == V_DEC);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "int"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "a"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last == 3); mu_assert(0==strcmp(t[last].str, ";"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 4); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, ";"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 8); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last ==13); mu_assert(0==strcmp(t[last].str, "}"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==14); mu_assert(0==strcmp(t[init].str, "while"));
  mu_assert(last ==28); mu_assert(0==strcmp(t[last].str, "}"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==29); mu_assert(0==strcmp(t[init].str, "if"));
  mu_assert(last ==40); mu_assert(0==strcmp(t[last].str, "}"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==41); mu_assert(0==strcmp(t[init].str, "if"));
  mu_assert(last ==59); mu_assert(0==strcmp(t[last].str, "}"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==60); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last ==66); mu_assert(0==strcmp(t[last].str, ";"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==67); mu_assert(0==strcmp(t[init].str, "return"));
  mu_assert(last ==69); mu_assert(0==strcmp(t[last].str, ";"));

  init=last+1;last=last_fix;k=find_function(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==70); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last ==70); mu_assert(0==strcmp(t[last].str, "}"));
  /*}}}*/
}/*}}}*/
void TEST_find_sentence() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "a=1;";
  last_fix=-1+mt(t,A);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == ASSIGN);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 3); mu_assert(0==strcmp(t[last].str, ";"));

  char *B = "{a=2;func(a,3);}";
  last_fix=-1+mt(t,B);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "{"));
  last_fix=-1+mt(t,B);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, ";"));
  last_fix=-1+mt(t,B);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last ==10); mu_assert(0==strcmp(t[last].str, ")"));
  last_fix=-1+mt(t,B);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==11); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last ==11); mu_assert(0==strcmp(t[last].str, ";"));
  last_fix=-1+mt(t,B);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==12); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last ==12); mu_assert(0==strcmp(t[last].str, "}"));

  char *C = "while (a==3) { func(a,4);}";
  last_fix=-1+mt(t,C);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == WHILE_FLOW);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "while"));
  mu_assert(last ==14); mu_assert(0==strcmp(t[last].str, "}"));

  char *D = "if (a<=5) { a=6;} ";
  last_fix=-1+mt(t,D);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == IF_FLOW);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "if"));
  mu_assert(last ==11); mu_assert(0==strcmp(t[last].str, "}"));

  char *E = "if (a>=7) { a=8;} else { a=9;}";
  last_fix=-1+mt(t,E);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == IF_FLOW);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "if"));
  mu_assert(last ==18); mu_assert(0==strcmp(t[last].str, "}"));

  char *F = "func(a,10);";
  last_fix=-1+mt(t,F);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == CALL);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ")"));
  last_fix=-1+mt(t,F);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == OTHER_NODE);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, ";"));

  char *G = "return a;";
  last_fix=-1+mt(t,G);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == RETURN);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "return"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, ";"));

  char *H = ";";
  last_fix=-1+mt(t,H);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == OTHER_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, ";"));

  char *J = "{a=1;{a=2;}a=3;}";
  last_fix=-1+mt(t,J);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "{"));
  last_fix=-1+mt(t,J);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, ";"));
  last_fix=-1+mt(t,J);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, "{"));
  last_fix=-1+mt(t,J);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 9); mu_assert(0==strcmp(t[last].str, ";"));
  last_fix=-1+mt(t,J);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==10); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last ==10); mu_assert(0==strcmp(t[last].str, "}"));
  last_fix=-1+mt(t,J);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==11); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last ==14); mu_assert(0==strcmp(t[last].str, ";"));
  last_fix=-1+mt(t,J);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==15); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last ==15); mu_assert(0==strcmp(t[last].str, "}"));

  char *K = "{{{{a=1;}}}}";
  last_fix=-1+mt(t,K);init=0;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "{"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "{"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "{"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 3); mu_assert(0==strcmp(t[last].str, "{"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 4); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, ";"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 8); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last == 8); mu_assert(0==strcmp(t[last].str, "}"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 9); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last == 9); mu_assert(0==strcmp(t[last].str, "}"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==10); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last ==10); mu_assert(0==strcmp(t[last].str, "}"));
  last_fix=-1+mt(t,K);init=last+1;last=last_fix;k=find_sentence(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==11); mu_assert(0==strcmp(t[init].str, "}"));
  mu_assert(last ==11); mu_assert(0==strcmp(t[last].str, "}"));
}/*}}}*/
void TEST_find_if_flow() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "if (a==1) {a=2;}";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "if"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "if"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, "1"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last ==11); mu_assert(0==strcmp(t[last].str, "}"));/*}}}*/

  char *B = "if (func(a)) {;} else{func(c,d);}";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "if"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "if"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 7); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last == 9); mu_assert(0==strcmp(t[last].str, "}"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==10); mu_assert(0==strcmp(t[init].str, "else"));
  mu_assert(last ==10); mu_assert(0==strcmp(t[last].str, "else"));

  init=last+1;last=last_fix;k=find_if_flow(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init ==11); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last ==19); mu_assert(0==strcmp(t[last].str, "}"));/*}}}*/
}/*}}}*/
void TEST_find_while() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "while (a) {a=2;func(a,b);}";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_while_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "while"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "while"));

  init=last+1;last=last_fix;k=find_while_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_while_flow(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "a"));

  init=last+1;last=last_fix;k=find_while_flow(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last == 3); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_while_flow(&init,&last,t);
  mu_assert(k == SENTENCE);
  mu_assert(init == 4); mu_assert(0==strcmp(t[init].str, "{"));
  mu_assert(last ==16); mu_assert(0==strcmp(t[last].str, "}"));/*}}}*/
}/*}}}*/
void TEST_find_return() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "return func(3,a);";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_return(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "return"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "return"));

  init=last+1;last=last_fix;k=find_return(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_return(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 7); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, ";"));/*}}}*/
}/*}}}*/
void TEST_find_assign() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "abc=func(4,2) + 5;";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "abc"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "abc"));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "="));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "="));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 9); mu_assert(0==strcmp(t[last].str, "5"));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==10); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last ==10); mu_assert(0==strcmp(t[last].str, ";"));/*}}}*/

  char *B = "a=(4);";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "a"));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "="));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "="));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ";"));/*}}}*/

  char *C = "a=func(4);";
  last_fix=-1+mt(t,C);/*{{{*/

  init=     0;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "a"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "a"));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "="));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "="));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_assign(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, ";"));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, ";"));/*}}}*/
}/*}}}*/
void TEST_find_call() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "func(8,9)";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_call(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "func"));

  init=last+1;last=last_fix;k=find_call(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_call(&init,&last,t);
  mu_assert(k == ARGUMENTS);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "8"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, "9"));

  init=last+1;last=last_fix;k=find_call(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/

  char *B = "func()";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_call(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "func"));

  init=last+1;last=last_fix;k=find_call(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_call(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/
}/*}}}*/
void TEST_find_arguments() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "1";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_arguments(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "1"));/*}}}*/

  char *B = "6,7,8";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_arguments(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init ==0); mu_assert(0==strcmp(t[init].str, "6"));
  mu_assert(last ==0); mu_assert(0==strcmp(t[last].str, "6"));

  init=last+1;last=last_fix;k=find_arguments(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init ==2); mu_assert(0==strcmp(t[init].str, "7"));
  mu_assert(last ==2); mu_assert(0==strcmp(t[last].str, "7"));

  init=last+1;last=last_fix;k=find_arguments(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init ==4); mu_assert(0==strcmp(t[init].str, "8"));
  mu_assert(last ==4); mu_assert(0==strcmp(t[last].str, "8"));/*}}}*/

  char *C = "f(x,y),3";
  last_fix=-1+mt(t,C);/*{{{*/

  init=     0;last=last_fix;k=find_arguments(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init ==0); mu_assert(0==strcmp(t[init].str, "f"));
  mu_assert(last ==5); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_arguments(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init ==7); mu_assert(0==strcmp(t[init].str, "3"));
  mu_assert(last ==7); mu_assert(0==strcmp(t[last].str, "3"));/*}}}*/

}/*}}}*/
void TEST_find_expression() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "1==2";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == FORMULA);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "1"));

  init=last+1;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "=="));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "=="));

  init=last+1;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == FORMULA);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "2"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "2"));/*}}}*/

  char *B = "(a*b)+c == (d*(e*f))";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == FORMULA);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, "c"));

  init=last+1;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 7); mu_assert(0==strcmp(t[init].str, "=="));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, "=="));

  init=last+1;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == FORMULA);
  mu_assert(init == 8); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last ==16); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/

  char *C = "(a+b+c) == func(d,e)*f";
  last_fix=-1+mt(t,C);/*{{{*/

  init=     0;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == FORMULA);
  mu_assert(init ==0); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last ==6); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 7); mu_assert(0==strcmp(t[init].str, "=="));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, "=="));

  init=last+1;last=last_fix;k=find_expression(&init,&last,t);
  mu_assert(k == FORMULA);
  mu_assert(init == 8); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last ==15); mu_assert(0==strcmp(t[last].str, "f"));/*}}}*/

}/*}}}*/
void TEST_find_formula() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "1";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "1"));/*}}}*/

  char *B = "(1+2+3)";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/

  char *C = "+1+2+3*a";
  last_fix=-1+mt(t,C);/*{{{*/

  init=     0;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "1"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, "2"));
  mu_assert(last == 3); mu_assert(0==strcmp(t[last].str, "2"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 4); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, "3"));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, "a"));
  /*}}}*/

  char *D = "1+2*3*4+(5*6)+7*(8+9)*a+b*cde(f,g)*h+i*(j*(k*(l+m))*n)*o";
  last_fix=-1+mt(t,D);/*{{{*/

  init=     0;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "1"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "2"));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, "4"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 7); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last == 7); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init == 8); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last ==12); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init ==13); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last ==13); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init ==14); mu_assert(0==strcmp(t[init].str, "7"));
  mu_assert(last ==22); mu_assert(0==strcmp(t[last].str, "a"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init ==23); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last ==23); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init ==24); mu_assert(0==strcmp(t[init].str, "b"));
  mu_assert(last ==33); mu_assert(0==strcmp(t[last].str, "h"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init ==34); mu_assert(0==strcmp(t[init].str, "+"));
  mu_assert(last ==34); mu_assert(0==strcmp(t[last].str, "+"));

  init=last+1;last=last_fix;k=find_formula(&init,&last,t);
  mu_assert(k == TERM);
  mu_assert(init ==35); mu_assert(0==strcmp(t[init].str, "i"));
  mu_assert(last ==53); mu_assert(0==strcmp(t[last].str, "o"));/*}}}*/

}/*}}}*/
void TEST_find_term() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "1";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "1"));/*}}}*/

  char *B = "(1+2+3)";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/

  char *C = "(1*2)*func(3*4)/(5+6)";
  last_fix=-1+mt(t,C);/*{{{*/

  init=     0;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 5); mu_assert(0==strcmp(t[init].str, "*"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, "*"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last ==11); mu_assert(0==strcmp(t[last].str, ")"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init ==12); mu_assert(0==strcmp(t[init].str, "/"));
  mu_assert(last ==12); mu_assert(0==strcmp(t[last].str, "/"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init ==13); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last ==17); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/

  char *D = "e*e*e";
  last_fix=-1+mt(t,D);/*{{{*/

  init=     0;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "e"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "e"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "*"));
  mu_assert(last == 1); mu_assert(0==strcmp(t[last].str, "*"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init == 2); mu_assert(0==strcmp(t[init].str, "e"));
  mu_assert(last == 2); mu_assert(0==strcmp(t[last].str, "e"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == B_OPERATOR);
  mu_assert(init == 3); mu_assert(0==strcmp(t[init].str, "*"));
  mu_assert(last == 3); mu_assert(0==strcmp(t[last].str, "*"));

  init=last+1;last=last_fix;k=find_term(&init,&last,t);
  mu_assert(k == FACTOR);
  mu_assert(init == 4); mu_assert(0==strcmp(t[init].str, "e"));
  mu_assert(last == 4); mu_assert(0==strcmp(t[last].str, "e"));/*}}}*/
}/*}}}*/
void TEST_find_factor() {/*{{{*/
  int k, init, last, last_fix;
  TOKEN t[1000];

  char *A = "1";
  last_fix=-1+mt(t,A);/*{{{*/

  init=     0;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == IMMEDIATE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "1"));/*}}}*/

  char *B = "abc";
  last_fix=-1+mt(t,B);/*{{{*/

  init=     0;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == VARIABLE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "abc"));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "abc"));/*}}}*/

  char *C = "(1+2+3)";
  last_fix=-1+mt(t,C);/*{{{*/

  init=     0;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "1"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, "3"));

  init=last+1;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 6); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last == 6); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/

  char *D = "func(3*4)";
  last_fix=-1+mt(t,D);/*{{{*/

  init=     0;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == CALL);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "func"));
  mu_assert(last == 5); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/

  char *E = "(x+y == func(1,2)*3)";
  last_fix=-1+mt(t,E);/*{{{*/

  init=     0;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init == 0); mu_assert(0==strcmp(t[init].str, "("));
  mu_assert(last == 0); mu_assert(0==strcmp(t[last].str, "("));

  init=last+1;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == EXPRESSION);
  mu_assert(init == 1); mu_assert(0==strcmp(t[init].str, "x"));
  mu_assert(last ==12); mu_assert(0==strcmp(t[last].str, "3"));

  init=last+1;last=last_fix;k=find_factor(&init,&last,t);
  mu_assert(k == SKIP_NODE);
  mu_assert(init ==13); mu_assert(0==strcmp(t[init].str, ")"));
  mu_assert(last ==13); mu_assert(0==strcmp(t[last].str, ")"));/*}}}*/
}/*}}}*/
