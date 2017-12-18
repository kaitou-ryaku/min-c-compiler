#include "../include/common.h"
#include "../include/misc.h"

extern char *token_kind_str(const int kind) {/*{{{*/
  if      (kind == TERM_OPERATOR   ) return "TERM_OPERATOR";
  else if (kind == FACTOR_OPERATOR ) return "FACTOR_OPERATOR";
  else if (kind == COMPARE_OPERATOR) return "COMPARE_OPERATOR";
  else if (kind == EQUAL           ) return "EQUAL";
  else if (kind == COMMA           ) return "COMMA";
  else if (kind == SEMICOLON       ) return "SEMICOLON";
  else if (kind == LPAREN          ) return "LPAREN";
  else if (kind == RPAREN          ) return "RPAREN";
  else if (kind == LBRACKET        ) return "LBRACKET";
  else if (kind == RBRACKET        ) return "RBRACKET";
  else if (kind == IF_KEYWORD      ) return "IF_KEYWORD";
  else if (kind == ELSE_KEYWORD    ) return "ELSE_KEYWORD";
  else if (kind == WHILE_KEYWORD   ) return "WHILE_KEYWORD";
  else if (kind == RETURN_KEYWORD  ) return "RETURN_KEYWORD";
  else if (kind == TYPE_KEYWORD    ) return "TYPE_KEYWORD";
  else if (kind == NUMBER          ) return "NUMBER";
  else if (kind == IDENTIFY        ) return "IDENTIFY";
  else                               return "Invalid Token kind";
}/*}}}*/
extern void fprint_token(FILE *fp, const TOKEN t) {/*{{{*/
  if (if_valid_token(t)) {
    fprintf(fp, "%-10s: %s\n", token_kind_str(t.kind), t.str);
  } else fprintf(fp, "Invalid token\n");
}/*}}}*/
extern void fprint_all_token(FILE *fp, const TOKEN *t, const int max_token_num) {/*{{{*/
  int i;
  for (i=0; i<max_token_num; i++) {
    fprintf(fp, "%03d %-16s: %-6s", i, token_kind_str(t[i].kind), t[i].str);
    if (i%5 != 4) fprintf(fp, " ");
    else          fprintf(fp, "\n");
  }
  if (i%5 != 0) fprintf(fp, "\n");
}/*}}}*/
extern void fprint_token_series(FILE *fp, const TOKEN *t, const int init, const int last) {/*{{{*/
  if (last - init < 20) {
    for (int i=init  ; i<=last  ; i++) fprintf(fp, "%s ", t[i].str);
  } else {
    for (int i=init  ; i<=init+9; i++) fprintf(fp, "%s ", t[i].str);
    fprintf(fp, ".......... ");
    for (int i=last-9; i<=last  ; i++) fprintf(fp, "%s ", t[i].str);
  }
}/*}}}*/
extern TOKEN initialize_token(void) {/*{{{*/
  TOKEN ret;
  ret.kind   = OTHER_TOKEN;
  ret.str[0] = '\0';
  return ret;
}/*}}}*/
extern bool compare_token(const TOKEN t1, const TOKEN t2) {/*{{{*/
  bool ret = true;
  if (t1.kind != t2.kind)          ret = false;
  if (strcmp(t1.str, t2.str) != 0) ret = false;
  return ret;
}/*}}}*/
extern bool if_valid_token(const TOKEN t) {/*{{{*/
  bool ret = true;
  if (strlen(t.str)+1 > TOKEN_STRING_MAX_LENGTH) ret = false;
  return ret;
}/*}}}*/

extern char *node_kind_str(const int kind) {/*{{{*/
  if      (kind == OTHER_NODE) return "OTHER_NODE";
  else if (kind == SKIP_NODE ) return "SKIP_NODE";
  else if (kind == PROGRAM   ) return "PROGRAM";
  else if (kind == V_DEC     ) return "V_DEC";
  else if (kind == F_DEC     ) return "F_DEC";
  else if (kind == PROTOTYPE ) return "PROTOTYPE";
  else if (kind == FUNCTION  ) return "FUNCTION";
  else if (kind == SENTENCE  ) return "SENTENCE";
  else if (kind == IF_FLOW   ) return "IF_FLOW";
  else if (kind == WHILE_FLOW) return "WHILE_FLOW";
  else if (kind == RETURN    ) return "RETURN";
  else if (kind == ASSIGN    ) return "ASSIGN";
  else if (kind == CALL      ) return "CALL";
  else if (kind == ARGUMENTS ) return "ARGUMENTS";
  else if (kind == EXPRESSION) return "EXPRESSION";
  else if (kind == FORMULA   ) return "FORMULA";
  else if (kind == TERM      ) return "TERM";
  else if (kind == FACTOR    ) return "FACTOR";
  else if (kind == B_OPERATOR) return "B_OPERATOR";
  else if (kind == IMMEDIATE ) return "IMMEDIATE";
  else if (kind == VARIABLE  ) return "VARIABLE";
  else if (kind == TYPE      ) return "TYPE";
  else                         return "Invalid node kind";
}/*}}}*/
extern void fprint_node(FILE *fp, const NODE n, const TOKEN *t) {/*{{{*/
  if (if_valid_node(n)) {
    fprintf(fp, "%-10s: ", node_kind_str(n.kind));
    fprint_token_series(fp, t, n.init, n.last);
    fprintf(fp, "\n");
  } else fprintf(fp, "Invalid node\n");
}/*}}}*/
extern NODE initialize_node(void) {/*{{{*/
  NODE ret;
  ret.kind         = OTHER_NODE;
  ret.init         = -1;
  ret.last         = -1;
  ret.arrived      = false;
  ret.left         = NULL;
  ret.right        = NULL;
  ret.up           = NULL;
  ret.down         = NULL;
  return ret;
}/*}}}*/
extern bool compare_node(const NODE n1, const NODE n2) {/*{{{*/
  bool ret = true;
  if      (n1.kind  != n2.kind)  ret = false;
  else if (n1.init  != n2.init)  ret = false;
  else if (n1.last  != n2.last)  ret = false;
  else if (n1.left  != n2.left)  ret = false;
  else if (n1.right != n2.right) ret = false;
  else if (n1.up    != n2.up)    ret = false;
  else if (n1.down  != n2.down)  ret = false;
  return ret;
}/*}}}*/
extern bool if_valid_node(const NODE n) {/*{{{*/
  bool ret = true;
  if      (n.init < 0)      ret = false;
  else if (n.last < 0)      ret = false;
  else if (n.last < n.init) ret = false;
  return ret;
}/*}}}*/
