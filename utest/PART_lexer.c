#include "../src/lexer.c"

void TEST_add_char_to_token_str() {/*{{{*/
  TOKEN t = {0,"hoge"};
  int   c = 1;

  add_char_to_token_str(&t, &c, 'x');
  mu_assert( strcmp(t.str, "hxge") == 0);
  mu_assert( c == 2);

  add_char_to_token_str(&t, &c, 'y');
  mu_assert( strcmp(t.str, "hxye") == 0);
  mu_assert( c == 3);
}/*}}}*/
void TEST_add_str_to_token_str() {/*{{{*/
  TOKEN t = {0,"hoge"};
  int   c = 1;

  add_str_to_token_str(&t, &c, "xxx");
  mu_assert( strcmp(t.str, "xxxe") == 0);
  mu_assert( c == 3);

  add_str_to_token_str(&t, &c, "y");
  mu_assert( strcmp(t.str, "yxxe") == 0);
  mu_assert( c == 1);
}/*}}}*/
void TEST_get_next_char() {/*{{{*/
  // 改行とコメントを空白1文字に変換
  FILE *f;
  f = fopen("tmp.c", "w+");
  fprintf(f,"a b c\n");
  fprintf(f,"d ef// h o g e      \n");
  fprintf(f,"  gh  i/* hoge\n");
  fprintf(f,"fuga*/jk l\n");

  rewind(f);


  // |a b c
  // |d ef// h o g e
  // |  gh  i/* hoge
  // |fuga*/jk l

  mu_assert( get_next_char(f) == 'a');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'b');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'c');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'd');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'e');
  mu_assert( get_next_char(f) == 'f');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'g');
  mu_assert( get_next_char(f) == 'h');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'i');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'j');
  mu_assert( get_next_char(f) == 'k');
  mu_assert( get_next_char(f) == ' ');
  mu_assert( get_next_char(f) == 'l');
  fclose(f);
}/*}}}*/
void TEST_get_next_token() {/*{{{*/
  TOKEN t;

  FILE *f  ;f=fopen("tmp.c", "w+");
  fprintf(f,"int main() /* hoge */ {\n");rewind(f);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "int"  ) && t.kind == TYPE_KEYWORD    );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "main" ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "("    ) && t.kind == LPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ")"    ) && t.kind == RPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "{"    ) && t.kind == LBRACKET        );

  fclose(f);f=fopen("tmp.c", "w+");
  fprintf(f,"x=2* ((a+bc)/(defg - i*3/j%%k) +m/n);\n");rewind(f);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "x"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "="    ) && t.kind == EQUAL           );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "2"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "*"    ) && t.kind == FACTOR_OPERATOR );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "("    ) && t.kind == LPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "("    ) && t.kind == LPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "a"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "+"    ) && t.kind == TERM_OPERATOR   );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "bc"   ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ")"    ) && t.kind == RPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "/"    ) && t.kind == FACTOR_OPERATOR );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "("    ) && t.kind == LPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "defg" ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "-"    ) && t.kind == TERM_OPERATOR   );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "i"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "*"    ) && t.kind == FACTOR_OPERATOR );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "3"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "/"    ) && t.kind == FACTOR_OPERATOR );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "j"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "%"    ) && t.kind == FACTOR_OPERATOR );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "k"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ")"    ) && t.kind == RPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "+"    ) && t.kind == TERM_OPERATOR   );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "m"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "/"    ) && t.kind == FACTOR_OPERATOR );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "n"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ")"    ) && t.kind == RPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ";"    ) && t.kind == SEMICOLON       );

  fclose(f);f=fopen("tmp.c", "w+");
  fprintf(f,"while(a+ b==c*   d){int a;}\n");rewind(f);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "while") && t.kind == WHILE_KEYWORD   );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "("    ) && t.kind == LPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "a"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "+"    ) && t.kind == TERM_OPERATOR   );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "b"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "=="   ) && t.kind == COMPARE_OPERATOR);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "c"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "*"    ) && t.kind == FACTOR_OPERATOR );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "d"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ")"    ) && t.kind == RPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "{"    ) && t.kind == LBRACKET        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "int"  ) && t.kind == TYPE_KEYWORD    );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "a"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ";"    ) && t.kind == SEMICOLON       );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "}"    ) && t.kind == RBRACKET        );

  fclose(f);f=fopen("tmp.c", "w+");
  fprintf(f,"if ( 3 >= 4 ) {;} else {a = 1;}\n");rewind(f);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "if"   ) && t.kind == IF_KEYWORD      );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "("    ) && t.kind == LPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "3"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ">="   ) && t.kind == COMPARE_OPERATOR);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "4"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ")"    ) && t.kind == RPAREN          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "{"    ) && t.kind == LBRACKET        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ";"    ) && t.kind == SEMICOLON       );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "}"    ) && t.kind == RBRACKET        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "else" ) && t.kind == ELSE_KEYWORD    );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "{"    ) && t.kind == LBRACKET        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "a"    ) && t.kind == IDENTIFY        );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "="    ) && t.kind == EQUAL           );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "1"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ";"    ) && t.kind == SEMICOLON       );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "}"    ) && t.kind == RBRACKET        );

  fclose(f);f=fopen("tmp.c", "w+");
  fprintf(f,"3 > 4 < 5 != 6\n");rewind(f);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "3"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, ">"    ) && t.kind == COMPARE_OPERATOR);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "4"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "<"    ) && t.kind == COMPARE_OPERATOR);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "5"    ) && t.kind == NUMBER          );
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "!="   ) && t.kind == COMPARE_OPERATOR);
  t=get_next_token(f); mu_assert( 0==strcmp( t.str, "6"    ) && t.kind == NUMBER          );

  fclose(f);
}/*}}}*/
void TEST_count_token_num() {/*{{{*/
  char *A = "int main() /* hoge */ {\n";
  FILE *f  ;f=fopen("tmp.c", "w+"); fprintf(f,"%s",A);rewind(f);
  mu_assert(count_token_num(f) ==  5);

  char *B = "x=2* ((a+bc)/(defg - i*3/j%k) +m/n);\n";
  fclose(f);f=fopen("tmp.c", "w+"); fprintf(f,"%s",B);rewind(f);
  mu_assert(count_token_num(f) == 28);

  char *C = "while(a+ b==c*   d){int a;}\n";
  fclose(f);f=fopen("tmp.c", "w+"); fprintf(f,"%s",C);rewind(f);
  mu_assert(count_token_num(f) == 15);

  char *D = "if ( 3 >= 4 ) {;} else {a = 1;}\n";
  fclose(f);f=fopen("tmp.c", "w+"); fprintf(f,"%s",D);rewind(f);
  mu_assert(count_token_num(f) == 16);

  char *E = "3 > 4 < 5 != 6\n";
  fclose(f);f=fopen("tmp.c", "w+"); fprintf(f,"%s",E);rewind(f);
  mu_assert(count_token_num(f) ==  7);

  char *F = "int func(int a,int bc, int d)";
  fclose(f);f=fopen("tmp.c", "w+"); fprintf(f,"%s",F);rewind(f);
  mu_assert(count_token_num(f) == 12);

  fclose(f);
}/*}}}*/
