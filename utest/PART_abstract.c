#include "../src/abstract.c"
#include "../include/lexer.h"
#include "../include/tree.h"

static SOURCE st(NODE *node, TOKEN *t, char *str) {/*{{{*/
  FILE *file;
  file = fopen("tmp.c","w+");
  fprintf(file,"%s",str);
  rewind(file);
  int max_token_num = count_token_num(file);
  for (int i=0;i<1000;i++) t[i] = initialize_token();
  for (int i=0;i<max_token_num ;i++) t[i] = get_next_token(file);
  fclose(file);

  for (int i=0; i<MAX_NODE_NUMBER; i++) node[i] = initialize_node();

  node[0] = make_root_node(max_token_num-1);
  int node_index = 1;
  node[0].down = make_down_tree(0, max_token_num-1, &(node[0]), NULL, &node_index, t, node);

  SOURCE ret;
  ret.program       = node;
  ret.max_node_num  = node_index;
  ret.token         = t;
  ret.max_token_num = max_token_num;

  return ret;
}/*}}}*/
void TEST_eliminate_container() {/*{{{*/
  TOKEN t[1000];
  NODE node[MAX_NODE_NUMBER];
  SOURCE s;

  char *A = "int main() {a=1+2+3+4+5;}";
  s = st(node, t, A);
  eliminate_insignificant(s);
  eliminate_redundant_sentence(s);
  eliminate_redundant_math(s);
  operator_zipper(&s);
  eliminate_container(s);
  // print_tree(s.program, s);

  char *B = "int main() {a=(((1+(2+(3+(4+5))))));}";
  s = st(node, t, B);
  eliminate_insignificant(s);
  eliminate_redundant_sentence(s);
  eliminate_redundant_math(s);
  operator_zipper(&s);
  eliminate_container(s);
  // print_tree(&(s.program[2]), s);
}/*}}}*/
void TEST_check_if_invalid_operator() {/*{{{*/
  TOKEN t[1000];
  NODE node[MAX_NODE_NUMBER];
  SOURCE s;

  char *A = "int main() {a= 3;}";
  s = st(node, t, A);
  mu_assert(true  == check_if_invalid_operator(s, false));

  char *B = "int main() {a=+3;}";
  s = st(node, t, B);
  mu_assert(false == check_if_invalid_operator(s, false));

  char *C = "int main() {a= 3*5*;}";
  s = st(node, t, C);
  mu_assert(false == check_if_invalid_operator(s, false));
}/*}}}*/
void TEST_operator_zipper() {/*{{{*/
  TOKEN t[1000];
  NODE node[MAX_NODE_NUMBER];
  SOURCE s;

  char *A = "int main() {a=1+2+3+4+5;}";
  s = st(node, t, A);
  eliminate_insignificant(s);
  eliminate_redundant_sentence(s);
  eliminate_redundant_math(s);
  operator_zipper(&s);
  // print_tree(&(s.program[2]), s);

  char *B = "int main() {fg = ((2+e/e*(e+3*e)%%(4*(5*6+7+e+func(e,e)))));}";
  s = st(node, t, B);
  eliminate_insignificant(s);
  eliminate_redundant_sentence(s);
  eliminate_redundant_math(s);
  operator_zipper(&s);
  // print_tree(&(s.program[2]), s);
}/*}}}*/
