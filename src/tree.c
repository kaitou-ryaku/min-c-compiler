#include "../include/common.h"
#include "../include/misc.h"
#include "../include/tree.h"

static int search_next_corresponding(const int nt, const int last, const TOKEN *token, const int left, const int right);
static int search_next_rparen(       const int nt, const int last, const TOKEN *token);
static int search_next_rbracket(     const int nt, const int last, const TOKEN *token);
static int count_r_minus_l(          const int nt, const int last, const TOKEN *token, const int left, const int right);
static int count_r_minus_l_paren(    const int nt, const int last, const TOKEN *token);
static int count_r_minus_l_bracket(  const int nt, const int last, const TOKEN *token);
static void inc(int *nt, const int last, bool *if_inside);

static void syntax_check(const char *s,const int nt, const int init, const int last, const TOKEN *token, const int should_be);
static void print_token_error_array(const char *s,const int nt, const int init, const int last, const TOKEN *token, const int *error_array, const int error_array_num);
static void print_token_error(const char *s, const int nt, const int init, const int last, const TOKEN *token);

static int find_program(   int *init, int *last, const TOKEN *token);
static int find_prototype( int *init, int *last, const TOKEN *token);
static int find_f_dec(     int *init, int *last, const TOKEN *token);
static int find_v_dec(     int *init, int *last, const TOKEN *token);
static int find_function(  int *init, int *last, const TOKEN *token);
static int find_sentence(  int *init, int *last, const TOKEN *token);
static int find_if_flow(   int *init, int *last, const TOKEN *token);
static int find_while_flow(int *init, int *last, const TOKEN *token);
static int find_return(    int *init, int *last, const TOKEN *token);
static int find_assign(    int *init, int *last, const TOKEN *token);
static int find_call(      int *init, int *last, const TOKEN *token);
static int find_arguments( int *init, int *last, const TOKEN *token);
static int find_expression(int *init, int *last, const TOKEN *token);
static int find_formula(   int *init, int *last, const TOKEN *token);
static int find_term(      int *init, int *last, const TOKEN *token);
static int find_factor(    int *init, int *last, const TOKEN *token);

extern NODE *make_down_tree(const int init, const int last, NODE *up, NODE *left, int *node_index, TOKEN *token, NODE *node) {/*{{{*/

  NODE *ret = NULL;
  int find_init = init, find_last = last;
  int find_kind = OTHER_NODE;

  if (init <= last) {
    if      (up -> kind == PROGRAM   ) find_kind = find_program(   &find_init, &find_last, token);
    else if (up -> kind == PROTOTYPE ) find_kind = find_prototype( &find_init, &find_last, token);
    else if (up -> kind == F_DEC     ) find_kind = find_f_dec(     &find_init, &find_last, token);
    else if (up -> kind == V_DEC     ) find_kind = find_v_dec(     &find_init, &find_last, token);
    else if (up -> kind == FUNCTION  ) find_kind = find_function(  &find_init, &find_last, token);
    else if (up -> kind == SENTENCE  ) find_kind = find_sentence(  &find_init, &find_last, token);
    else if (up -> kind == IF_FLOW   ) find_kind = find_if_flow(   &find_init, &find_last, token);
    else if (up -> kind == WHILE_FLOW) find_kind = find_while_flow(&find_init, &find_last, token);
    else if (up -> kind == RETURN    ) find_kind = find_return(    &find_init, &find_last, token);
    else if (up -> kind == ASSIGN    ) find_kind = find_assign(    &find_init, &find_last, token);
    else if (up -> kind == CALL      ) find_kind = find_call(      &find_init, &find_last, token);
    else if (up -> kind == ARGUMENTS ) find_kind = find_arguments( &find_init, &find_last, token);
    else if (up -> kind == FORMULA   ) find_kind = find_formula(   &find_init, &find_last, token);
    else if (up -> kind == EXPRESSION) find_kind = find_expression(&find_init, &find_last, token);
    else if (up -> kind == TERM      ) find_kind = find_term(      &find_init, &find_last, token);
    else if (up -> kind == FACTOR    ) find_kind = find_factor(    &find_init, &find_last, token);
  }

  // token列内に有効なノードが見つかった場合は登録
  if (find_kind != OTHER_NODE) {/*{{{*/
    ret          = &(node[*node_index]);
    ret -> init  = find_init;
    ret -> last  = find_last;
    ret -> kind  = find_kind;
    ret -> up    = up;
    ret -> left  = left;
    *node_index += 1;

    ret -> down  = make_down_tree(find_init,   find_last, ret, NULL, node_index, token, node); // retの下の木を作成
    ret -> right = make_down_tree(find_last+1, last,      up , ret , node_index, token, node); // retの下の木を作成
  }/*}}}*/

  return ret;
}/*}}}*/
extern NODE make_root_node(const int max_node_num) {/*{{{*/
  NODE ret;
  ret.kind  = PROGRAM;
  ret.init  = 0;
  ret.last  = max_node_num;
  ret.up    = NULL;
  ret.left  = NULL;
  ret.right = NULL;
  ret.down  = NULL;
  return ret;
}/*}}}*/

static int search_next_corresponding(const int nt, const int last, const TOKEN *token, const int left, const int right) {/*{{{*/
  int num_left = 0;
  int num_right= 0;

  bool if_find = true;
  int ret = nt;

  while (ret <= last) {
    int kind = token[ret].kind;
    if      (kind == left ) num_left++;
    else if (kind == right) num_right++;

    if (num_left <= num_right) {
      if_find = true;
      break;
    }
    ret++;
  }

  if (!if_find) {
    print_token_error(__func__,last,nt,last,token);
    printf("Corresponding %s Not Found\n", token_kind_str(right));
    ret = -1;
  }

  return ret;
}/*}}}*/
static int search_next_rparen(const int nt, const int last, const TOKEN *token) {/*{{{*/
  return search_next_corresponding(nt, last, token, LPAREN, RPAREN);
}/*}}}*/
static int search_next_rbracket(const int nt, const int last, const TOKEN *token) {/*{{{*/
  return search_next_corresponding(nt, last, token, LBRACKET, RBRACKET);
}/*}}}*/
static int count_r_minus_l(          const int nt, const int last, const TOKEN *token, const int left, const int right) {/*{{{*/
  int num_left = 0;
  int num_right= 0;

  int i = nt;
  while (i <= last) {
    int kind = token[i].kind;
    if      (kind == left ) num_left++;
    else if (kind == right) num_right++;
    i++;
  }
  return num_right - num_left;
}/*}}}*/
static int count_r_minus_l_paren(    const int nt, const int last, const TOKEN *token) {/*{{{*/
  return count_r_minus_l(nt, last, token, LPAREN, RPAREN);
}/*}}}*/
static int count_r_minus_l_bracket(  const int nt, const int last, const TOKEN *token) {/*{{{*/
  return count_r_minus_l(nt, last, token, LBRACKET, RBRACKET);
}/*}}}*/
static void inc(int *nt, const int last, bool *if_inside) {/*{{{*/
  if (*if_inside) {
    if (*nt <  last) *nt += 1;
    else *if_inside = false;
  }
}/*}}}*/

static void syntax_check(const char *s,const int nt, const int init, const int last, const TOKEN *token, const int should_be) {/*{{{*/
  if (token[nt].kind != should_be) {
    print_token_error(s, nt, init, last, token);
    printf(" --> %s\n", token_kind_str(should_be));
  }
}/*}}}*/
static void print_token_error_array(const char *s,const int nt, const int init, const int last, const TOKEN *token,  const int *array, const int array_num) {/*{{{*/
  print_token_error(s, nt, init, last, token);
  fprintf(stderr, " --> ");
  for (int i=0; i<array_num; i++) fprintf(stderr, "%s, ", token_kind_str(array[i]));
  fprintf(stderr, "\n");
}/*}}}*/
static void print_token_error(const char *s, const int nt, const int init, const int last, const TOKEN *token) {/*{{{*/
  fprintf(stderr, "\nSyntax Error %-15s:", s);
  for (int i=init; i<last; i++) {
    if (i==nt) fprintf(stderr, " ```%s'''", token[i].str);
    else       fprintf(stderr, " %s"      , token[i].str);
  }
  fprintf(stderr, " ");
}/*}}}*/

static int find_program(   int *init, int *last, const TOKEN *token) {/*{{{*/
  // PROGRAM ::= (V_DEC ";" | PROTOTYPE ";" | F_DEC)*
  // PROGRAM の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  // ; int func(...) の;を飛ばす処理
  if (*init >= *last) return SKIP_NODE;
  if (token[nt].kind == SEMICOLON) {
    *last = *init;
    return SKIP_NODE;
  }

  // TYPE_KEYWORD ID
  syntax_check(__func__,nt,arg_init,arg_last,token,TYPE_KEYWORD);
  *init = nt;
  inc(&nt, arg_last, &if_inside);
  syntax_check(__func__,nt,arg_init,arg_last,token,IDENTIFY);
  inc(&nt, arg_last, &if_inside);

  if      (token[nt].kind == SEMICOLON) {   // TYPE_KEYWORD ID ;/*{{{*/
    kind  = V_DEC;
    *last = nt-1;
  }/*}}}*/
  else if (token[nt].kind == LPAREN) {      // TYPE_KEYWORD ID (...)/*{{{*/
    nt = search_next_rparen(nt, arg_last, token);
    assert(nt > 0);
    inc(&nt, arg_last, &if_inside);
    if      (token[nt].kind == SEMICOLON) { // TYPE_KEYWORD ID (...) ;/*{{{*/
      kind  = PROTOTYPE;
      *last = nt-1;
    }/*}}}*/
    else if (token[nt].kind == LBRACKET) {  // TYPE_KEYWORD ID (...) {...}/*{{{*/
      kind  = F_DEC;
      nt = search_next_rbracket(nt, arg_last, token);
      assert(nt > 0);
      *last = nt;
    }/*}}}*/
    else {                                  // 構文エラー処理/*{{{*/
      int er[] = {SEMICOLON, LBRACKET};
      print_token_error_array(__func__,nt,arg_init,arg_last,token,er,2);
    }/*}}}*/
  }/*}}}*/
  else {                                    // 構文エラー処理/*{{{*/
    int er[] = {SEMICOLON, LPAREN};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,2);
  }/*}}}*/

  if (!if_inside) kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_prototype( int *init, int *last, const TOKEN *token) {/*{{{*/
  // PROTOTYPE ::= TYPE_KEYWORD VARIABLE "(" (\ep | V_DEC (, V_DEC)*) ")"
  // PROTOTYPE の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  if (count_r_minus_l_paren(nt, arg_last, token) == 0) { // (...) の外側/*{{{*/
    *last = nt;
    if      (token[nt].kind == TYPE_KEYWORD) kind = TYPE;
    else if (token[nt].kind == IDENTIFY)     kind = VARIABLE;
    else if (token[nt].kind == LPAREN)       kind = SKIP_NODE;
    else {
      int er[] = {TYPE_KEYWORD, IDENTIFY, LPAREN};
      print_token_error_array(__func__,nt,arg_init,arg_last,token,er,3);
    }
  }/*}}}*/
  else {                                                 // (...) の内側/*{{{*/
    if      (token[nt].kind == TYPE_KEYWORD) {
      kind  = V_DEC;
      inc(&nt, arg_last, &if_inside);
      syntax_check(__func__,nt,arg_init,arg_last,token,IDENTIFY);
      *last = nt;
    }
    else if (token[nt].kind == COMMA) {
      kind  = SKIP_NODE;
      *last = nt;
    }
    else if (token[nt].kind == RPAREN) {
      kind  = SKIP_NODE;
      *last = nt;
    }
    else {
      int er[] = {TYPE_KEYWORD, COMMA, RPAREN};
      print_token_error_array(__func__,nt,arg_init,arg_last,token,er,3);
    }
  }/*}}}*/

  if (!if_inside) kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_f_dec(     int *init, int *last, const TOKEN *token) {/*{{{*/
  // F_DEC ::= TYPE_KEYWORD VARIABLE "(" (\ep | V_DEC (, V_DEC)*) ")" FUNCTION
  // F_DEC の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  // {...}の左のカッコの場合は、対応するカッコまでをFUNCTIONとする
  // 左カッコでなければ、プロトタイプ宣言の関数をそのまま使う
  int kind = OTHER_NODE;
  if (token[*init].kind == LBRACKET) {
    kind = FUNCTION;
    assert(*last == search_next_rbracket(*init, *last, token));
  } else kind = find_prototype(init, last, token);

  return kind;
}/*}}}*/
static int find_v_dec(     int *init, int *last, const TOKEN *token) {/*{{{*/
  // V_DEC ::= TYPE VARVARIABLE ";"
  // V_DEC の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;

  if      (token[arg_init].kind == IDENTIFY) {
    assert(arg_init   == arg_last);
    kind  = VARIABLE;
  }
  else if (token[arg_init].kind == TYPE_KEYWORD) {
    assert(arg_init+1 == arg_last);
    kind  = TYPE;
    *last = arg_init;
  }
  else {
    int er[] = {IDENTIFY, TYPE_KEYWORD};
    print_token_error_array(__func__,arg_init,arg_init,arg_last,token,er,2);
  }

  return kind;
}/*}}}*/
static int find_function(  int *init, int *last, const TOKEN *token) {/*{{{*/
  // FUNCTION ::= "{" (V_DEC ";")* SENTENCE* "}"
  // FUNCTION の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  if (token[nt].kind == RBRACKET) {       // 呼元関数の{...}の最後のカッコの場合/*{{{*/
    assert(arg_init == arg_last);
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt].kind == LBRACKET && search_next_rbracket(nt, arg_last, token) == arg_last && token[arg_last].kind == RBRACKET) { // 左括弧を削っていた場合/*{{{*/
    *last = *init;
    return SKIP_NODE;
  }/*}}}*/
  else if (token[nt].kind == LBRACKET) {       // ブロックの{...}の最初のカッコの場合/*{{{*/
    kind = SENTENCE;
    *init = nt;
    nt = search_next_rbracket(nt, arg_last, token); // {...}
    assert(nt > 0);
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == TYPE_KEYWORD) {   // TYPE_KEYWORD/*{{{*/
    kind = V_DEC;
    *init = nt;
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,IDENTIFY);
    *last = nt;
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,SEMICOLON);
  }/*}}}*/
  else if (token[nt].kind == SEMICOLON) {      // ;/*{{{*/
    if (token[nt-2].kind == TYPE_KEYWORD && token[nt-1].kind == IDENTIFY) kind = SKIP_NODE;
    else kind = SENTENCE;
    *last = *init;
  }/*}}}*/
  else if (token[nt].kind == IF_KEYWORD) {     // if/*{{{*/
    kind = SENTENCE;
    *init = nt;
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LPAREN);     // if (
    nt = search_next_rparen(nt, arg_last, token);
    assert(nt > 0);
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LBRACKET);   // if (...) {
    nt = search_next_rbracket(nt, arg_last, token);      // if (...) {...}
    assert(nt > 0);

    inc(&nt, arg_last, &if_inside);
    if (token[nt].kind == ELSE_KEYWORD) {                // if (...) {...} else
      inc(&nt, arg_last, &if_inside);
      syntax_check(__func__,nt,arg_init,arg_last,token,LBRACKET); // if (...) {...} else {
      nt = search_next_rbracket(nt, arg_last, token);    // if (...) {...} else {...}
      assert(nt > 0);
      *last = nt;
    } else {                                             // if (...) {...} ????
      nt--;
      *last = nt;
    }
  }/*}}}*/
  else if (token[nt].kind == WHILE_KEYWORD) {  // while/*{{{*/
    kind = SENTENCE;
    *init = nt;
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LPAREN);     // while (
    nt = search_next_rparen(nt, arg_last, token);        // while (...)
    assert(nt > 0);
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LBRACKET);   // while (...) {
    nt = search_next_rbracket(nt, arg_last, token);      // while (...) {...}
    assert(nt > 0);
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == RETURN_KEYWORD) { // return/*{{{*/
    kind = SENTENCE;
    *init = nt;
    while (nt < arg_last) {
      nt++;
      if (token[nt].kind == SEMICOLON) break;  // return [^;]* ;
    }
    syntax_check(__func__,nt,arg_init,arg_last,token,SEMICOLON);
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == IDENTIFY) {       // ID/*{{{*/
    kind = SENTENCE;
    *init = nt;
    while (nt < arg_last) {
      nt++;
      if (token[nt].kind == SEMICOLON) break;  // ID [^;]* ;
    }
    syntax_check(__func__,nt,arg_init,arg_last,token,SEMICOLON);
    *last = nt;
  }/*}}}*/
  else {                                       // 構文エラー処理/*{{{*/
    int er[] = {TYPE_KEYWORD, SEMICOLON, IF_KEYWORD, WHILE_KEYWORD, RETURN_KEYWORD, IDENTIFY, RBRACKET, LBRACKET};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,8);
  }/*}}}*/

  if (!if_inside) kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_sentence(  int *init, int *last, const TOKEN *token) {/*{{{*/
  // SENTENCE ::=  (";" | IF_FLOW | WHILE_FLOW | RETURN | CALL ";" | ASSIGN | "{" SENTENCE* "}")
  // FUNCTION の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;
  bool if_in_bracket = (count_r_minus_l_bracket(arg_init, arg_last, token) > 0);

  if      (token[nt].kind == SEMICOLON) {      // ;/*{{{*/
    kind = OTHER_NODE;
    *init = nt;
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == IF_KEYWORD) {     // if/*{{{*/
    kind = IF_FLOW;
    *init = nt;
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LPAREN);
    nt = search_next_rparen(nt, arg_last, token);          // if (...)
    assert(nt > 0);
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LBRACKET);
    nt = search_next_rbracket(nt, arg_last, token);        // if (...) {...}
    assert(nt > 0);

    if (nt == arg_last) *last = nt;
    else {
      inc(&nt, arg_last, &if_inside);
      if (token[nt].kind == ELSE_KEYWORD) {                // if (...) {...} else
        inc(&nt, arg_last, &if_inside);
        syntax_check(__func__,nt,arg_init,arg_last,token,LBRACKET);
        nt = search_next_rbracket(nt, arg_last, token);    // if (...) {...} else {...}
        assert(nt > 0);
        *last = nt;
      } else {                                             // if (...) {...} ????
        nt--;
        *last = nt;
      }
    }
  }/*}}}*/
  else if (token[nt].kind == WHILE_KEYWORD) {  // while/*{{{*/
    kind = WHILE_FLOW;
    *init = nt;
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LPAREN);     // while (
    nt = search_next_rparen(nt, arg_last, token);        // while (...)
    assert(nt > 0);
    inc(&nt, arg_last, &if_inside);
    syntax_check(__func__,nt,arg_init,arg_last,token,LBRACKET);   // while (...) {
    nt = search_next_rbracket(nt, arg_last, token);      // while (...) {...}
    assert(nt > 0);
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == RETURN_KEYWORD) { // return/*{{{*/
    kind = RETURN;
    *init = nt;
    while (nt < arg_last) {
      nt++;
      if (token[nt].kind == SEMICOLON) break;  // return [^;]* ;
    }
    syntax_check(__func__,nt,arg_init,arg_last,token,SEMICOLON);
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == IDENTIFY) {       // ID/*{{{*/
    *init = nt;
    inc(&nt, arg_last, &if_inside);
    if (token[nt].kind == LPAREN) {
      kind = CALL;
      nt = search_next_rparen(nt, arg_last, token);        // ID (...)
      assert(nt > 0);
      *last = nt;
    }
    else {
      kind = ASSIGN;
      while (nt < arg_last) {
        nt++;
        if (token[nt].kind == SEMICOLON) break;  // ID [^;]* ;
      }
      syntax_check(__func__,nt,arg_init,arg_last,token,SEMICOLON);
      *last = nt;
    }
  }/*}}}*/
  else if (token[nt].kind == LBRACKET) {       // ブロックの{...}の最初のカッコの場合/*{{{*/
    *last = *init;
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt-1].kind == LBRACKET) {       // ブロックの{...}の最初のカッコの一つ右の場合/*{{{*/
    const int tmp_nt = search_next_rbracket(nt-1, arg_last, token);
    assert(tmp_nt > 0);
    kind = SENTENCE;

    if (tmp_nt == arg_last) {         // {...}
      inc(&nt, arg_last, &if_inside);
      *init = nt;                     // { init...}=last
      find_sentence(init, last, token);

    } else {                          // init={...}=last ...
      *init = nt;
      *last = tmp_nt;
    }
  }/*}}}*/
  else if (token[nt].kind == RBRACKET) {       // の最後のカッコの場合/*{{{*/
    if_in_bracket = false;
    kind = SKIP_NODE;
    *init = nt;
    *last = nt;
  }/*}}}*/
  else {                                       // 構文エラー処理/*{{{*/
    int er[] = {SEMICOLON, IF_KEYWORD, WHILE_KEYWORD, RETURN_KEYWORD, IDENTIFY, LBRACKET};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,8);
  }/*}}}*/

  if (if_in_bracket && kind != SKIP_NODE) kind = SENTENCE;
  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_if_flow(   int *init, int *last, const TOKEN *token) {/*{{{*/
  // IF_FLOW ::= "if" "(" EXPRESSION ")" "{" SENTENCE* "}" (\ep |"else" "{" SENTENCE "}")
  // IF_FLOW の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  if      (token[nt].kind == IF_KEYWORD) {   // if/*{{{*/
    *last = *init;
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt].kind == LPAREN) {       // (...)の最初の丸括弧/*{{{*/
    syntax_check(__func__,nt,arg_init,arg_last,token,LPAREN);
    *last = *init;
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt-1].kind == LPAREN) {     // (...)の最初の丸括弧の次/*{{{*/
    const int tmp_last = search_next_rparen(nt-1, arg_last, token);
    assert(nt < tmp_last);                 // if ( 何かある )
    kind  = EXPRESSION;
    *init = nt;
    *last = tmp_last - 1;
  }/*}}}*/
  else if (token[nt].kind == RPAREN) {       // (...)の最後の丸括弧/*{{{*/
    kind  = SKIP_NODE;
    *init = nt;
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == LBRACKET) {     // {...}/*{{{*/
    const int tmp_last = search_next_rbracket(nt, arg_last, token);
    assert(nt+1 < tmp_last);                 // { 何かある }
    kind  = SENTENCE;
    *init = nt;
    *last = tmp_last;
  }/*}}}*/
  else if (token[nt].kind == ELSE_KEYWORD) { // else/*{{{*/
    *last = *init;
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt-1].kind == ELSE_KEYWORD) { // else {...} の最初の中括弧/*{{{*/
    syntax_check(__func__,nt,arg_init,arg_last,token,LBRACKET);
    const int tmp_last = search_next_rbracket(nt, arg_last, token);
    assert(nt+1 < tmp_last);                 // else { 何かある }
    kind  = SENTENCE;
    *init = nt;
    *last = tmp_last;
  }/*}}}*/
  else {                                     // 構文エラー処理/*{{{*/
    int er[] = {IF_KEYWORD, RPAREN, LBRACKET, ELSE_KEYWORD};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,4);
  }/*}}}*/

  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_while_flow(int *init, int *last, const TOKEN *token) {/*{{{*/
  // WHILE_FLOW ::= "while" "(" EXPRESSION ")" "{" SENTENCE* "}"
  // WHILE_FLOW の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  if      (token[nt].kind == WHILE_KEYWORD) {// while/*{{{*/
    *last = *init;
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt].kind == LPAREN) {       // (...)の最初の丸括弧/*{{{*/
    syntax_check(__func__,nt,arg_init,arg_last,token,LPAREN);
    *last = *init;
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt-1].kind == LPAREN) {     // (...)の最初の丸括弧の次/*{{{*/
    const int tmp_last = search_next_rparen(nt-1, arg_last, token);
    assert(nt < tmp_last);                   // while ( 何かある )
    kind  = EXPRESSION;
    *init = nt;
    *last = tmp_last - 1;
  }/*}}}*/
  else if (token[nt].kind == RPAREN) {       // (...)の最後の丸括弧/*{{{*/
    kind  = SKIP_NODE;
    *init = nt;
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == LBRACKET) {     // {...}/*{{{*/
    const int tmp_last = search_next_rbracket(nt, arg_last, token);
    assert(nt+1 < tmp_last);                 // { 何かある }
    kind  = SENTENCE;
    *init = nt;
    *last = tmp_last;
  }/*}}}*/
  else {                                     // 構文エラー処理/*{{{*/
    int er[] = {WHILE_KEYWORD, RPAREN, LBRACKET};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,3);
  }/*}}}*/

  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_return(    int *init, int *last, const TOKEN *token) {/*{{{*/
  // RETURN ::= "return" EXPRESSION ";"
  // RETURN の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  syntax_check(__func__,arg_last,arg_init,arg_last,token,SEMICOLON);

  if (token[nt].kind == RETURN_KEYWORD) {
    *last = *init;
    kind = SKIP_NODE;
  }
  else if (token[nt-1].kind == RETURN_KEYWORD) {
    assert(nt < arg_last);
    kind  = EXPRESSION;
    *init = nt;
    *last = arg_last-1;
  }
  else if (token[nt].kind == SEMICOLON) {
    assert(nt == arg_init && nt == arg_last);
    kind = SKIP_NODE;
  }
  else {
    int er[] = {RETURN_KEYWORD, SEMICOLON};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,2);
  }

  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_assign(    int *init, int *last, const TOKEN *token) {/*{{{*/
  // ASSIGN ::= VARIABLE "=" EXPRESSION ";"
  // ASSIGN の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;

  // 代入のイコールが存在するか検索
  bool if_equal = false;
  for (int i=nt; i<=arg_last; i++) if (token[i].kind == EQUAL) {if_equal = true; break;}

  if      (if_equal && token[nt].kind == EQUAL) { // イコール直上/*{{{*/
    kind  = SKIP_NODE;
    *last = nt;
  }/*}}}*/
  else if (if_equal && token[nt].kind != EQUAL) { // 左辺/*{{{*/
    assert(nt == arg_init);
    syntax_check(__func__,nt,arg_init,arg_last,token,IDENTIFY);
    kind  = VARIABLE;
    *last = nt;
  }/*}}}*/
  else {                                          // 右辺/*{{{*/
    syntax_check(__func__,arg_last,arg_init,arg_last,token,SEMICOLON);
    if (token[nt].kind == SEMICOLON) kind = SKIP_NODE;
    else {
      kind  = EXPRESSION;
      *init = nt;
      *last = arg_last - 1;
    }
  }/*}}}*/

  return kind;
}/*}}}*/
static int find_call(      int *init, int *last, const TOKEN *token) {/*{{{*/
  // CALL ::= VARIABLE "(" (\ep | ARGUMENTS) ")"
  // CALL の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;

  if      (token[nt].kind == IDENTIFY && token[nt+1].kind == LPAREN) {  // ID (...)/*{{{*/
    if (search_next_rparen(nt+1, arg_last, token) == arg_last) {
      *last = *init;
      kind = VARIABLE;
    }
  }/*}}}*/
  else if (token[nt].kind == LPAREN && search_next_rparen(nt, arg_last, token) == arg_last) {    // (...) の左括弧/*{{{*/
    *last = *init;
    kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt-1].kind == LPAREN) {  // (何かある) なら ARGUMENTS を返し、()ならSKIP/*{{{*/
    assert(0 == count_r_minus_l_paren(nt-1, *last, token));
    const int tmp_init = nt;
    const int tmp_last = search_next_rparen(nt-1, arg_last, token)-1;
    assert(tmp_last+1 == arg_last);

    if (tmp_init <= tmp_last) {
      kind  = ARGUMENTS;
      *init = tmp_init;
      *last = tmp_last;
    } else kind = SKIP_NODE;
  }/*}}}*/
  else if (token[nt].kind == RPAREN) {    // (...) の右カッコの場合はSKIP/*{{{*/
    assert(nt == arg_last);
    kind = SKIP_NODE;
  }/*}}}*/
  else {                                  // 構文エラー処理/*{{{*/
    int er[] = {IDENTIFY, LPAREN, RPAREN};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,3);
  }/*}}}*/

  return kind;
}/*}}}*/
static int find_arguments( int *init, int *last, const TOKEN *token) {/*{{{*/
  // ARGUMENTS  ::= EXPRESSION ("," EXPRESSION)*)
  // ARGUMENTS の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  assert(token[arg_last].kind != COMMA);
  assert(token[arg_last].kind != SEMICOLON);

  kind = EXPRESSION;

  if (token[nt].kind == COMMA) inc(&nt, arg_last, &if_inside);
  *init = nt;

  while (nt <= arg_last) {
    if      (token[nt].kind == LPAREN) { // init ... (...)
      nt = search_next_rparen(nt, arg_last, token);
      assert(nt > 0);
      if (nt == arg_last) break;         // init ... (...) arg_last
    }
    else if (token[nt].kind == COMMA) {  // init ... ,
      *last = nt - 1;
      break;
    }
    nt++;
  }

  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_expression( int *init, int *last, const TOKEN *token) {/*{{{*/
  // EXPRESSION ::= FORMULA (%COMPARE_OPERATOR% FORMULA)*
  // EXPRESSION の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_last = *last;
  int  kind = OTHER_NODE;
  int  nt   = *init;

  if (token[nt].kind == COMPARE_OPERATOR) { // 比較演算子直上
    kind  = B_OPERATOR;
    *last = nt;

  } else {
    kind  = FORMULA;
    while (nt < arg_last) {
      if (token[nt].kind == COMPARE_OPERATOR) break;
      nt++;
    }
    if (token[nt].kind == COMPARE_OPERATOR) *last = nt-1;
    else {assert(nt == arg_last);           *last = nt; }
  }

  return kind;
}/*}}}*/
static int find_formula(int *init, int *last, const TOKEN *token) {/*{{{*/
  // FORMULA ::= (\ep | %TERM_OPERATOR% ) TERM ( %TERM_OPERATOR% TERM)*
  // FORMULA の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;
  int tk = token[nt].kind;

  if ( (tk == NUMBER) || (tk == IDENTIFY) || (tk == LPAREN)) {
    while (nt < arg_last) {
      tk = token[nt].kind;

      if (   (tk == NUMBER)
          || (tk == IDENTIFY)
          || (tk == RPAREN)
          || (tk == FACTOR_OPERATOR))
        inc(&nt, arg_last, &if_inside);

      else if (tk == LPAREN) {
        nt = search_next_rparen(nt, arg_last, token);
        assert(nt > 0);
      }

      else if (tk == TERM_OPERATOR) {
        nt--;
        break;
      }

      else {                                  // 構文エラー処理/*{{{*/
        int er[] = {NUMBER, IDENTIFY, LPAREN, RPAREN, FACTOR_OPERATOR, COMPARE_OPERATOR, TERM_OPERATOR};
        print_token_error_array(__func__,nt,arg_init,arg_last,token,er,7);
        inc(&nt, arg_last, &if_inside);
      }/*}}}*/
    }
    kind = TERM;
    *last = nt;
  }
  else if (token[nt].kind == TERM_OPERATOR) {
    kind  = B_OPERATOR;
    *last = nt;
  }

  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_term(      int *init, int *last, const TOKEN *token) {/*{{{*/
  // TERM       ::= FACTOR (\ep|FACTOR_OPERATOR FACTOR)*
  // TERM      の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  if      (token[nt].kind == LPAREN) {           // (...)/*{{{*/
    kind  = FACTOR;
    nt = search_next_rparen(nt, arg_last, token);
    assert(nt > 0);
    *last = nt;
    if (nt < arg_last) syntax_check(__func__,nt+1,arg_init,arg_last,token,FACTOR_OPERATOR);
  }/*}}}*/
  else if (token[nt].kind == NUMBER) {           // NUMBER/*{{{*/
    kind  = FACTOR;
    *last = nt;
    if (nt < arg_last) syntax_check(__func__,nt+1,arg_init,arg_last,token,FACTOR_OPERATOR);
  }/*}}}*/
  else if (token[nt].kind == IDENTIFY) {         // ID/*{{{*/
    kind = FACTOR;
    if (nt < arg_last) {
      nt++;
      if      (token[nt].kind == FACTOR_OPERATOR) *last = nt-1;
      else if (token[nt].kind == LPAREN) {
        nt = search_next_rparen(nt, arg_last, token);
        assert(nt > 0);
        *last = nt;
      }
    }
  }/*}}}*/
  else if (token[nt].kind == RPAREN) {           // (...) のカッコの右端処理/*{{{*/
    kind = SKIP_NODE;
    *last = nt;
  }/*}}}*/
  else if (token[nt].kind == FACTOR_OPERATOR) {  // [* / %]/*{{{*/
    kind = B_OPERATOR;
    *last = nt;
  }/*}}}*/
  else {                                  // 構文エラー処理/*{{{*/
    int er[] = {LPAREN, NUMBER, IDENTIFY, RPAREN, FACTOR_OPERATOR};
    print_token_error_array(__func__,nt,arg_init,arg_last,token,er,5);
    inc(&nt, arg_last, &if_inside);
  }/*}}}*/

  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
static int find_factor(    int *init, int *last, const TOKEN *token) {/*{{{*/
  // FACTOR ::= (IMMEDIATE | "(" EXPRESSION ")" | CALL | VARIABLE)
  // FACTOR の下に直接生える、init~last間のトークン列に対し、最も左でマッチしたノードのinit,last,kindを返す

  const int arg_init = *init, arg_last = *last;
  int  kind      = OTHER_NODE;
  int  nt        = *init;
  bool if_inside = true;

  if (token[nt].kind == RPAREN) {   // (...)の右側のカッコの処理/*{{{*/
    *last = *init;
    kind  = SKIP_NODE;
  }/*}}}*/
  else if (token[nt-1].kind == LPAREN && search_next_rparen(nt-1, arg_last, token) == arg_last && token[arg_last].kind == RPAREN) { // 左括弧を削っていた場合/*{{{*/
    *last = arg_last-1;
    kind = EXPRESSION;
  }/*}}}*/
  else {
    if      (token[nt].kind == LPAREN) {   // (...)/*{{{*/
      assert(search_next_rparen(nt, arg_last, token) == arg_last);
      // 左カッコを削る
      *last = *init;
      kind = SKIP_NODE;
    }/*}}}*/
    else if (token[nt].kind == NUMBER) {   // NUMBER/*{{{*/
      assert(nt == arg_last);
      *last = *init;
      kind  = IMMEDIATE;
    }/*}}}*/
    else if (token[nt].kind == IDENTIFY) { // ID/*{{{*/
      if (nt < arg_last) {
        kind = CALL;
        nt++;
        syntax_check(__func__,nt,arg_init,arg_last,token,LPAREN);
        nt = search_next_rparen(nt, arg_last, token);
        assert(nt == arg_last || nt == arg_last-1);
        *last = nt;
      } else kind = VARIABLE;
    }/*}}}*/
    else {                                 // 構文エラー処理/*{{{*/
      int er[] = {LPAREN, NUMBER, IDENTIFY, RPAREN};
      print_token_error_array(__func__,nt,arg_init,arg_last,token,er,4);
      inc(&nt, arg_last, &if_inside);
    }/*}}}*/
  }
  if (!if_inside)    kind = OTHER_NODE;
  return kind;
}/*}}}*/
