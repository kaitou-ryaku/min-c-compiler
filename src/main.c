#include "../include/common.h"
#include "../include/misc.h"
#include "../include/lexer.h"
#include "../include/tree.h"
#include "../include/abstract.h"
#include "../include/asm.h"
#include "../include/node_edit.h" // デバッグ用

int main(int argc, char* argv[]) {
  if (argc < 2) EXIT_MSG("Source File Not Found.\n");
  FILE *file;
  file = fopen(argv[1], "r");
  if (file == NULL) EXIT_VAR("Source File ```%s''' Cannot Be Opened.", argv[1]);

  const int max_token_num = count_token_num(file);
  TOKEN *token;
  token = (TOKEN *)malloc( sizeof(TOKEN)*max_token_num );
  for (int i=0; i<max_token_num; i++) token[i] = get_next_token(file);
  fclose(file);

  file = fopen("lex.txt", "w");
  fprintf(file, "--- LEXICAL ANALYSIS (TOKEN LIST) ---\n");
  fprint_all_token(file, token, max_token_num);
  fclose(file);

  NODE node[MAX_NODE_NUMBER];
  for (int i=0; i<MAX_NODE_NUMBER; i++) node[i] = initialize_node();

  node[0] = make_root_node(max_token_num-1);
  int node_index = 1;
  node[0].down = make_down_tree(0, max_token_num-1, &(node[0]), NULL, &node_index, token, node);

  SOURCE src;
  src.program       = node;
  src.max_node_num  = node_index;
  src.token         = token;
  src.max_token_num = max_token_num;

  // fprintf(stderr, "--- PARSE TREE ---\n");
  // fprint_tree(stderr, src.program, src);
  // fprintf(stderr, "\n");

  file = fopen("pt.dot", "w");
  fprint_tree_dot_style(file, src.program, src);
  fclose(file);

  src = abstract_tree(src);

  // fprintf(stderr, "--- ABSTRACT SYNTAX TREE ---\n");
  // fprint_tree(stderr, src.program, src);
  // fprintf(stderr, "\n");

  file = fopen("ast.dot", "w");
  fprint_tree_dot_style(file, src.program, src);
  fclose(file);

  file = fopen("symb.txt", "w");
  TABLE tb_tmp[MAX_HASH_NUMBER];
  fprint_symbol_table(file, src, tb_tmp);
  fclose(file);

  if (argc < 3) EXIT_MSG("Output File Not Specified.\n");
  file = fopen(argv[2], "w");
  if (file == NULL) EXIT_VAR("Output File ```%s''' Cannot Be Opened.", argv[2]);

  TABLE tb[MAX_HASH_NUMBER];
  asm_program(file, src, tb);
  fclose(file);

  free(token);

  return 0;
}
