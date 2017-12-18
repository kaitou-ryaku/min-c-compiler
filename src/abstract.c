#include "../include/common.h"
#include "../include/misc.h"
#include "../include/node_edit.h"
#include "../include/abstract.h"

static void syntax_error(const char *func, NODE *node, SOURCE s, const char *msg);
static void eliminate_insignificant(SOURCE s);
static void eliminate_redundant_sentence(SOURCE s);
static void eliminate_redundant_math(SOURCE s);
static bool check_if_invalid_operator(SOURCE s, bool if_print);
static void operator_zipper(SOURCE *s);
static void eliminate_container(SOURCE s);

extern SOURCE abstract_tree(SOURCE s) {/*{{{*/
  eliminate_insignificant(s);
  eliminate_redundant_sentence(s);
  eliminate_redundant_math(s);
  assert(check_if_invalid_operator(s, true));
  operator_zipper(&s);
  eliminate_container(s);
  return s;
}/*}}}*/
extern void fprint_tree(FILE *fp, NODE *node, SOURCE s) {/*{{{*/
  bool if_print_layer[1000];
  fresh_tree(s);
  while (node != NULL) {
    if (!(node -> arrived)) {
      fprintf(fp, "[%03d]-[%03d] %-10s", node -> init, node -> last, node_kind_str(node -> kind));
      int vertical = count_vertical(node);
      int up_count = 0;;
      NODE *up_n = node -> up;
      while (up_n != NULL) {
        if (up_n -> right == NULL) if_print_layer[vertical - up_count - 1] = false;
        else                       if_print_layer[vertical - up_count - 1] = true;
        up_count++;
        up_n = up_n -> up;
      }
      for (int i=0; i<vertical; i++) {
        if (if_print_layer[i]) fprintf(fp, "|   ");
        else                   fprintf(fp, "    ");
      }
      // if      (node -> left == NULL)       fprintf(fp, "--- ");
      // else if (if_print_layer[vertical-1]) fprintf(fp, "|   ");
      // else                                 fprintf(fp, "    ");
      fprint_token_series(fp, s.token, node -> init, node -> last);

      if (node -> down != NULL) {
        int len = 0;
        for (int i = node -> init; i <= node -> last; i++) len += (strlen(s.token[node -> init].str)+1);
        for (int i = 0; i< 5-len; i++) fprintf(fp, "-");
      }
      fprintf(fp, "\n");
    }
    node = go_next_node(node);
  }
}/*}}}*/
extern void fprint_tree_dot_style(FILE *fp, NODE *node, SOURCE s) {/*{{{*/
  fresh_tree(s);
  fprintf(fp, "digraph graphname {\n");
  fprintf(fp, "  node [shape=box];\n");

  while (node != NULL) {
    if (!(node -> arrived) && node -> up != NULL) {
      NODE *upnode = node -> up;

      fprintf(fp, "  \"[%03d %s] ", upnode -> init, node_kind_str(upnode -> kind));
      fprint_token_series(fp, s.token, upnode -> init, upnode -> last);
      fprintf(fp, "\" [label=\"%s\\n", node_kind_str(upnode -> kind));
      fprint_token_series(fp, s.token, upnode -> init, upnode -> last);
      fprintf(fp, "\"];\n");

      fprintf(fp, "  \"[%03d %s] ", node -> init, node_kind_str(node -> kind));
      fprint_token_series(fp, s.token, node -> init, node -> last);
      fprintf(fp, "\" [label=\"%s\\n", node_kind_str(node -> kind));
      fprint_token_series(fp, s.token, node -> init, node -> last);
      fprintf(fp, "\"];\n");

      fprintf(fp, "  \"");
      fprintf(fp, "[%03d %s] ", upnode -> init, node_kind_str(upnode -> kind));
      fprint_token_series(fp, s.token, upnode -> init, upnode -> last);

      fprintf(fp, "\" -> \"");

      fprintf(fp, "[%03d %s] ", node -> init, node_kind_str(node -> kind));
      fprint_token_series(fp, s.token, node -> init, node -> last);
      fprintf(fp, "\";\n");
    }
    node = go_next_node(node);
  }
  fprintf(fp, "}\n");
}/*}}}*/

static void syntax_error(const char *func, NODE *node, SOURCE s, const char *msg) {/*{{{*/
  fprintf(stderr, "\nSyntax Error at %s(): ", func);
  fprint_token_series(stderr, s.token, node -> init, node -> last);
  fprintf(stderr, "\n%s\n", msg);
}/*}}}*/
static void eliminate_insignificant(SOURCE s) {/*{{{*/
  // program 以下の SKIP_NODE と ; だけの命令を全削除する関数
  NODE *node = s.program;
  fresh_tree(s);
  while (node != NULL) {
    if (node -> kind == SKIP_NODE) delete_node_recursive(node);
    if ( (node -> kind == SENTENCE)
      && (s.token[node -> init].kind == SEMICOLON)
      && (node -> init == node -> last)) delete_node_recursive(node);
    node = go_next_node(node); // 削除後に down に移動しても、再度戻ってくるので問題ない
  }
}/*}}}*/
static void eliminate_redundant_sentence(SOURCE s) {/*{{{*/
  NODE *node = s.program;
  fresh_tree(s);
  while (node != NULL) {
    NODE *up   = node -> up;
    if (up == NULL) {
      node = go_next_node(node);
      continue;
    }

    // 孤独な自分の上がSENTENCEなら、SENTECEを自分で置き換えて自分を削除
    if (up -> kind == SENTENCE) replace_by_solitary_node(node);
    node = go_next_node(node);
  }
}/*}}}*/
static void eliminate_redundant_math(SOURCE s) {/*{{{*/
  NODE *node = s.program;
  fresh_tree(s);
  while (node != NULL) {
    NODE *up   = node -> up;
    if (up == NULL) {
      node = go_next_node(node);
      continue;
    }

    // 孤独な数式の上がコンテナなら、コンテナを数式で置き換えて数式を削除
    if ( (    (node -> kind == IMMEDIATE ) // 自分が数式
           || (node -> kind == VARIABLE  )
           || (node -> kind == CALL      )
           || (node -> kind == EXPRESSION)
           || (node -> kind == FORMULA   )
           || (node -> kind == TERM      )
           || (node -> kind == FACTOR    )
         ) &&
         (    (up   -> kind == EXPRESSION) // 自分の上がコンテナ
           || (up   -> kind == FORMULA   )
           || (up   -> kind == TERM      )
           || (up   -> kind == FACTOR    )
         )
       ) replace_by_solitary_node(node); // 自分が孤独であれば、上のコンテナを自分で置き換えて自分を削除

    node = go_next_node(node);
  }
}/*}}}*/
static bool check_if_invalid_operator(SOURCE s, bool if_print) {/*{{{*/
  // B_OPERATORの両隣に文字が存在することを確認
  // a = +3 のような+は B_OPERATORではなく S_OPERATOR として構文解析するのが筋?
  bool  ret = true;
  NODE *node = s.program;
  fresh_tree(s);
  while (node != NULL) {
    if (node -> kind == B_OPERATOR) {
      if (node -> left  == NULL) {
        ret = false;
        if (if_print) syntax_error(__func__, node, s, "No operand in left side of the operator");
      }
      if (node -> right == NULL) {
        ret = false;
        if (if_print) syntax_error(__func__, node, s, "No operand in right side of the operator");
      }
    }
    node = go_next_node(node); // 削除後に down に移動しても、再度戻ってくるので問題ない
  }

  return ret;
}/*}}}*/
static void operator_zipper(SOURCE *s) {/*{{{*/
  // 1+2+3+4+5 --> (((1+2)+3)+4)+5
  //
  // ___BEFORE___
  //      [node -> up]
  // NULL [node      ] A B ... [right_ope] [right_term] NULL
  //
  //
  // ___AFTER___
  //      [node -> up]
  // NULL [container ] [right_ope] [right_term] NULL
  // NULL [node      ] A B ... NULL

  NODE *node = s -> program;
  fresh_tree(*s);
  while (node != NULL) {
    if ( (node -> up    != NULL) && (node -> left  == NULL) && (node -> right != NULL)) {
      if ( ((node -> right) -> kind == B_OPERATOR) && 3 < count_horizontal(node)) {
        NODE *left_term  = node;
        NODE *right_term = node;
        while (true) {
          if (right_term -> right == NULL) break;
          right_term = right_term -> right;
        }
        NODE *right_ope = right_term -> left;
        NODE *container = &((s -> program)[s -> max_node_num]);
        *container = initialize_node();
        s -> max_node_num += 1;

        container -> kind  = (node -> up) -> kind;
        container -> init  = left_term -> init;
        container -> last  = (right_ope -> left) -> last;
        container -> up    = node -> up;
        container -> left  = NULL;
        container -> right = right_ope;
        container -> down  = left_term;

        (node -> up) -> down = container;
        right_ope    -> left = container;

        NODE *tmp = left_term;
        while (true) {
          tmp -> up = container;
          if (tmp -> right == right_ope) {
            tmp -> right = NULL;
            break;
          }
          tmp = tmp -> right;
        }

        container -> arrived = false;
        node = container;
      }
    }
    node = go_next_node(node);
  }
}/*}}}*/
static void eliminate_container(SOURCE s) {/*{{{*/
  NODE *node = s.program;
  fresh_tree(s);
  while (node != NULL) {
    int kind = node -> kind;
    if ( (kind == EXPRESSION)
      || (kind == FORMULA)
      || (kind == TERM)
      || (kind == FACTOR)
    ) {
      assert(node -> down != NULL);
      assert(count_horizontal(node -> down) == 3);
      assert((node -> down) -> right != NULL);
      assert(((node -> down) -> right) -> kind == B_OPERATOR);

      NODE *op = (node -> down) -> right;
      node -> kind = op -> kind;
      node -> init = op -> init;
      node -> last = op -> last;
      delete_node_recursive(op);
    }
    node = go_next_node(node); // 削除後に down に移動しても、再度戻ってくるので問題ない
  }
}/*}}}*/
