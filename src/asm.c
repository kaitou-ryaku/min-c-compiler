#include "../include/common.h"
#include "../include/misc.h"
#include "../include/node_edit.h"
#include "../include/table.h"
#include "../include/asm.h"

extern void asm_program( FILE *fp, SOURCE s, TABLE *tb);
static void fprint_global_variable(FILE *fp, SOURCE s, TABLE *tb);

static int get_stack_frame(TABLE *tb);
static void syntax_error(const char *func);
static void fprint_variable(FILE *fp, NODE *func    , SOURCE s, TABLE *tb);

static void asm_function( FILE *fp, NODE *func    , SOURCE s, TABLE *tb);
static void asm_sentence( FILE *fp, NODE *sentence, SOURCE s, TABLE *tb);
static void asm_if(       FILE *fp, NODE *sentence, SOURCE s, TABLE *tb);
static void asm_while(    FILE *fp, NODE *sentence, SOURCE s, TABLE *tb);
static void asm_return(   FILE *fp, NODE *sentence, SOURCE s, TABLE *tb);
static void asm_assign(   FILE *fp, NODE *sentence, SOURCE s, TABLE *tb);
static void asm_call(     FILE *fp, NODE *sentence, SOURCE s, TABLE *tb);
static void asm_expr(     FILE *fp, NODE *expr    , SOURCE s, TABLE *tb, const char *label); // asm_operator でラベルが必要
static void asm_immediate(FILE *fp, NODE *imm     , SOURCE s, TABLE *tb);
static void asm_variable( FILE *fp, NODE *var     , SOURCE s, TABLE *tb);
static void asm_operator( FILE *fp, NODE *ope     , SOURCE s, TABLE *tb, const char *label); // ループの処理でラベルが必要
static void asm_compare(  FILE *fp, const char *str, const char *label, const int label_num, const char *reg1, const char *reg2);

extern void fprint_symbol_table(FILE *fp, SOURCE s, TABLE *tb) {/*{{{*/
  make_global_table(tb, s);
  fprintf(fp, "--- SYMBOL TABLE FOR GLOBAL SCOPE ---\n");
  fprint_table(fp, tb);
  fprintf(fp, "\n");

  fresh_tree(s);
  NODE *f_dec = s.program -> down;
  while (f_dec != NULL) {
    if (f_dec -> kind == F_DEC) {
      make_local_table(f_dec, tb, s);
      NODE *func = f_dec -> down -> right;

      const char *func_name = s.token[func -> init].str;
      fprintf(fp, "--- SYMBOL TABLE FOR %s() ---\n", func_name);
      fprint_table(fp, tb);
      delete_local_var(tb);
    }
    f_dec = f_dec -> right;
  }
}/*}}}*/
extern void asm_program(FILE *fp, SOURCE s, TABLE *tb) {/*{{{*/
  fprintf(fp, "bits 32\n");
  // fprintf(fp, "org 0x7c00\n"); // x86ブート風の設定
  fprintf(fp, "org 0x0\n");
  fprintf(fp, "jmp _main_\n");

  make_global_table(tb, s);

  // 関数をニーモニックに変換
  fresh_tree(s);
  NODE *f_dec = s.program -> down;
  while (f_dec != NULL) {
    if (f_dec -> kind == F_DEC) {
      make_local_table(f_dec, tb, s);
      NODE *func = f_dec -> down -> right;

      const char *func_name = s.token[func -> init].str;
      fprintf(fp, "\n_%s_:\n", func_name); // 関数名を表示

      while (func -> right != NULL) func = func -> right;
      assert(func -> kind == FUNCTION);
      asm_function(fp, func, s, tb);
      if (0 == strcmp(s.token[f_dec -> down -> right -> init].str, "main")) {
        fprintf(fp, "jmp _program_halt_\n");
      } else {
        fprintf(fp, "leave\n");
        fprintf(fp, "ret\n");
      }

      delete_local_var(tb);
    }
    f_dec = f_dec -> right;
  }

  fprintf(fp, "\n_program_halt_:\n");
  fprintf(fp, "hlt\n");

  // グローバル変数をニーモニックに変換
  fprint_global_variable(fp, s, tb);
}/*}}}*/

static void fprint_global_variable(FILE *fp, SOURCE s, TABLE *tb) {/*{{{*/
  fresh_tree(s);
  NODE *v_dec = s.program -> down;
  while (v_dec != NULL) {
    if (v_dec -> kind == V_DEC) {
      NODE *var = v_dec -> down -> right;
      int hash = search_hash(var, tb, s);
      if (hash < 0) {
        fprintf(fp, "\n");
        syntax_error(__func__);
        fprintf(fp, "hash:%d str:%s\n", hash, s.token[var -> init].str);
      }

      assert(tb[hash].block == 0);
      fprintf(fp, "\n_%s_:\n",  tb[hash].str);
      fprintf(fp, "dd 0x0\n");
    }
    v_dec = v_dec -> right;
  }
}/*}}}*/

static int get_stack_frame(TABLE *tb) {/*{{{*/
  int ret = 0;
  for (int i=0; i<MAX_HASH_NUMBER; i++)
    if ((tb[i].used) && (tb[i].block > 1) && (-tb[i].address > ret))
      ret = -tb[i].address;

  int margin = (16 - (ret % 16)) % 16; // スタックフレームサイズを16の倍数にするための処理
  ret += margin;
  return ret;
}/*}}}*/
static void syntax_error(const char *func) {/*{{{*/
  fprintf(stderr, "Syntax Error in %s():\n", func);
}/*}}}*/

static void asm_function(FILE *fp, NODE *func, SOURCE s, TABLE *tb) {/*{{{*/
  fprintf(fp, "push ebp\n");
  fprintf(fp, "mov ebp, esp\n");
  fprintf(fp, "sub esp, 0x%X\n", get_stack_frame(tb));

  NODE *sentence = func -> down;
  while (sentence != NULL) {
    if (sentence -> kind != V_DEC) asm_sentence(fp, sentence, s, tb);
    sentence = sentence -> right;
  }
}/*}}}*/
static void asm_sentence(FILE *fp, NODE *sentence, SOURCE s, TABLE *tb) {/*{{{*/
  if (sentence != NULL) {
    const int kind = sentence -> kind;
    if        (kind == SENTENCE) {
      NODE *down = sentence -> down;
      while (down != NULL) {
        asm_sentence(fp, down, s, tb);
        down = down -> right;
      }
    }
    else if (kind == IF_FLOW)    asm_if(    fp, sentence, s, tb);
    else if (kind == WHILE_FLOW) asm_while( fp, sentence, s, tb);
    else if (kind == RETURN)     asm_return(fp, sentence, s, tb);
    else if (kind == ASSIGN)     asm_assign(fp, sentence, s, tb);
    else if (kind == CALL)       asm_call(  fp, sentence, s, tb);
    else syntax_error(__func__);
  }
}/*}}}*/
static void fprint_variable(FILE *fp, NODE *var    , SOURCE s, TABLE *tb) {/*{{{*/
  int hash = search_hash(var, tb, s);
  if (hash < 0) {
    fprintf(fp, "\n");
    syntax_error(__func__);
    fprintf(fp, "hash:%d str:%s\n", hash, s.token[var -> init].str);
  }
  int block   = tb[hash].block;

  if      (block == 0) fprintf(fp, "[_%s_]"    ,  tb[hash].str);     // グローバル変数
  else if (block == 1) fprintf(fp, "[ebp+0x%x]",  tb[hash].address); // 関数の引数
  else if (block >  1) fprintf(fp, "[ebp-0x%x]", -tb[hash].address); // ローカル変数
}/*}}}*/

static void asm_if(FILE *fp, NODE *sentence, SOURCE s, TABLE *tb) {/*{{{*/
  NODE *cond  = sentence -> down;
  NODE *block = cond -> right;
  const int label_num = sentence -> init;

  asm_expr(fp, cond, s, tb, "if_begin");
  fprintf(fp, "pop eax\n");
  fprintf(fp, "cmp eax, 0\n");

  if (block -> right == NULL) { // if () {} 型
    fprintf(fp, "je  LABEL%d_if_end\n",label_num);
    asm_sentence(fp, block, s, tb);
  }
  else {                       // if () {} else {} 型
    fprintf(fp, "je  LABEL%d_else\n"  ,label_num);
    asm_sentence(fp, block, s, tb);
    fprintf(fp, "jmp LABEL%d_if_end\n",label_num);
    fprintf(fp, "LABEL%d_else:\n"   ,label_num);
    asm_sentence(fp, block->right, s, tb);
  }
  fprintf(fp, "LABEL%d_if_end:\n", label_num);
}/*}}}*/
static void asm_while(FILE *fp, NODE *sentence, SOURCE s, TABLE *tb) {/*{{{*/
  NODE *cond  = sentence -> down;
  NODE *block = cond -> right;
  const int label_num = sentence -> init;

  // 一回目のループを実行するかどうか
  asm_expr(fp, cond, s, tb, "while_begin");
  fprintf(fp, "pop eax\n");
  fprintf(fp, "cmp eax, 0\n");
  fprintf(fp, "je  LABEL%d_while_end\n", label_num);

  // ブロックの中身
  fprintf(fp, "LABEL%d_while_begin:\n", label_num);
  asm_sentence(fp, block, s, tb);

  // ブロック処理後に再度ループを実行するかどうか
  asm_expr(fp, cond, s, tb, "while_end");
  fprintf(fp, "pop eax\n");
  fprintf(fp, "cmp eax, 0\n");
  fprintf(fp, "jne LABEL%d_while_begin\n", label_num);

  // ループ終了
  fprintf(fp, "LABEL%d_while_end:\n", label_num);
}/*}}}*/
static void asm_return(FILE *fp, NODE *sentence, SOURCE s, TABLE *tb) {/*{{{*/
  NODE *expr = sentence -> down;
  asm_expr(fp, expr, s, tb, "");
  fprintf(fp, "pop eax\n");
}/*}}}*/
static void asm_assign(FILE *fp, NODE *sentence, SOURCE s, TABLE *tb) {/*{{{*/
  NODE *id   = sentence -> down;
  NODE *expr = id -> right;
  assert(expr -> right == NULL);
  asm_expr(fp, expr, s, tb, "");
  fprintf(fp, "mov ");
  fprint_variable(fp, id, s, tb);
  fprintf(fp, ", eax\n");
}/*}}}*/
static void asm_call(FILE *fp, NODE *sentence, SOURCE s, TABLE *tb) {/*{{{*/
  NODE *func      = sentence -> down;
  NODE *arguments = func -> right;
  if (arguments != NULL) {
    NODE *arg = arguments -> down;
    while (arg -> right != NULL) arg = arg -> right;
    while (arg != NULL) {
      asm_expr(fp, arg, s, tb, "");
      arg = arg -> left;
    }

    fprintf(fp, "call _%s_\n", s.token[func -> init].str);

    arg = arguments -> down;
    while (arg != NULL) {
      fprintf(fp, "pop edx\n");
      arg = arg -> right;
    }
  }
  else fprintf(fp, "call _%s_\n", s.token[func -> init].str);

  fprintf(fp, "push eax\n");
}/*}}}*/
static void asm_expr(FILE *fp, NODE *expr, SOURCE s, TABLE *tb, const char *label) {/*{{{*/
  assert(expr != NULL);
  if      (expr -> kind == IMMEDIATE)  asm_immediate(fp, expr, s, tb);
  else if (expr -> kind == VARIABLE)   asm_variable( fp, expr, s, tb);
  else if (expr -> kind == CALL)       asm_call(     fp, expr, s, tb);
  else if (expr -> kind == B_OPERATOR) asm_operator( fp, expr, s, tb, label);
  else {
    syntax_error(__func__);
    fprintf(fp, "%s\n", node_kind_str(expr -> kind));
  }
}/*}}}*/
static void asm_immediate(FILE *fp, NODE *imm, SOURCE s, TABLE *tb) {/*{{{*/
  fprintf(fp, "mov eax, ");
  int num = atoi(s.token[imm -> init].str);
  if (num < 0) fprintf(fp, "-0x%x\n", -num);
  else         fprintf(fp, "0x%x\n", num);
  fprintf(fp, "push eax\n");
}/*}}}*/
static void asm_variable(FILE *fp, NODE *var, SOURCE s, TABLE *tb) {/*{{{*/
  fprintf(fp, "mov eax, ");
  fprint_variable(fp, var, s, tb);
  fprintf(fp, "; %s\n", s.token[var -> init].str);
  fprintf(fp, "push eax\n");
}/*}}}*/
static void asm_operator(FILE *fp, NODE *ope, SOURCE s, TABLE *tb, const char *label) {/*{{{*/
  NODE *down_left  = ope -> down;
  NODE *down_right = down_left -> right;
  assert(down_right -> right == NULL);
  asm_expr(fp, down_left , s, tb, label);
  asm_expr(fp, down_right, s, tb, label);
  fprintf(fp, "pop edx\n");
  fprintf(fp, "pop eax\n");

  char *str = s.token[ope -> init].str;
  if      (0 == strcmp(str, "+" )) fprintf(fp, "add  eax, edx\n");
  else if (0 == strcmp(str, "-" )) fprintf(fp, "sub  eax, edx\n");
  else if (0 == strcmp(str, "*" )) fprintf(fp, "imul eax, edx\n");
  else if (0 == strcmp(str, "/" )) fprintf(fp, "idiv eax, edx\n");
  else if (0 == strcmp(str, "%" )) fprintf(fp, "imod eax, edx\n");
  else asm_compare(fp, str, label, ope -> init, "eax", "edx");
  fprintf(fp, "push eax\n");
}/*}}}*/
static void asm_compare(FILE *fp, const char *str, const char *label, const int label_num, const char *reg1, const char *reg2) {/*{{{*/
  fprintf(fp, "cmp %s, %s\n", reg1, reg2);
  if      (0 == strcmp(str, "==")) fprintf(fp, "je  ");
  else if (0 == strcmp(str, "!=")) fprintf(fp, "jne ");
  else if (0 == strcmp(str, "<" )) fprintf(fp, "jl  ");
  else if (0 == strcmp(str, "<=")) fprintf(fp, "jle ");
  else if (0 == strcmp(str, ">" )) fprintf(fp, "jg  ");
  else if (0 == strcmp(str, ">=")) fprintf(fp, "jge ");
  else syntax_error(__func__);
  fprintf(fp, "LABEL_%s_true_%d\n", label, label_num);

  // 条件が満たされなければ eax = 0
  fprintf(fp, "mov eax, 0x0\n");
  fprintf(fp, "jmp LABEL_%s_end_%d\n", label, label_num);

  // 条件が満たされれば eax = 1
  fprintf(fp, "LABEL_%s_true_%d:\n", label, label_num);
  fprintf(fp, "mov eax, 0x1\n");
  fprintf(fp, "LABEL_%s_end_%d:\n", label, label_num);
}/*}}}*/
