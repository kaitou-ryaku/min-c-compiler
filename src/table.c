#include "../include/common.h"
#include "../include/misc.h"
#include "../include/node_edit.h"
#include "../include/table.h"

typedef enum {
  V_KIND_IN_DEC, // 関数内部での変数宣言
  V_KIND_FN_ARG, // 関数本体引数宣言
  V_KIND_PR_ARG, // プロトタイプ引数宣言
  V_KIND_GB_DEC, // グローバル変数宣言
  V_KIND_IN_CAL, // 関数内部での関数呼出
  V_KIND_IN_REF, // 関数内部での変数参照
  V_KIND_FN_STR, // 関数本体の名前
  V_KIND_PR_STR  // プロトタイプ宣言の名前
} VARIABLE_KIND;

static void fprint_table_function(FILE *fp, const TABLE t, const int hash);
static void fprint_table_variable(FILE *fp, const TABLE t, const int hash);
static void register_global_v( NODE *node, int *address, TABLE *t, SOURCE s);
static void register_function( NODE *node, TABLE *t, SOURCE s);
static void register_prototype(NODE *node, TABLE *t, SOURCE s);
static void register_arguments(NODE *f_dec, TABLE *t, SOURCE s);
static void register_local_var(NODE *f_dec, TABLE *t, SOURCE s);
static bool if_args_rational(const NODE n1, const NODE n2, SOURCE s);

static TABLE initialize_table();
static int two_node_to_hash(NODE *var, const int kind, SOURCE s);
static int get_empty_hash(NODE *var, const int kind, const TABLE *t, SOURCE s);
static bool if_same_hash_num(const int a, const int b);
static int inc_hash(const int a);
static bool if_same_hash_info(const TABLE t, NODE *var, NODE *root, const int kind, SOURCE s);
static int search_hash_by_condition(NODE *var, NODE *root, const int kind, TABLE *t, SOURCE s);
extern int search_hash(NODE *var, TABLE *t, SOURCE s);
static int get_var_kind(NODE *var);

extern void make_global_table(TABLE *t, SOURCE s) {/*{{{*/
  for (int i=0; i<MAX_HASH_NUMBER; i++) {
    t[i] = initialize_table();
  }

  int address = 0;
  fresh_tree(s);
  NODE *node = s.program -> down;
  assert(node != NULL);
  while (node != NULL) {
    if      (node -> kind == V_DEC)     register_global_v( node, &address, t, s);
    else if (node -> kind == F_DEC)     register_function( node,           t, s);
    else if (node -> kind == PROTOTYPE) register_prototype(node,           t, s);
    node = node -> right;
  }
}/*}}}*/
extern void make_local_table( NODE *f_dec, TABLE *t, SOURCE s) {/*{{{*/
  assert(f_dec -> kind == F_DEC);
  register_arguments(f_dec, t, s);
  register_local_var(f_dec, t, s);
}/*}}}*/
extern void delete_local_var(TABLE *t) {/*{{{*/
  for (int i=0; i<MAX_HASH_NUMBER; i++) {
    if (t[i].block > 0) t[i] = initialize_table();
  }
}/*}}}*/
extern void fprint_table(FILE *fp, TABLE *t) {/*{{{*/
  fprintf(fp, "%-10s %-10s %10s %5s %5s %9s %5s %5s\n", "NAME", "SCOPE", "CATEGORY", "TYPE", "SIZE", "ADDRESS", "TOKEN", "HASH");
  for (int i=0; i<MAX_HASH_NUMBER; i++) if (t[i].used && t[i].kind == F_DEC                   ) fprint_table_function(fp, t[i], i);
  for (int i=0; i<MAX_HASH_NUMBER; i++) if (t[i].used && t[i].kind == V_DEC && t[i].block == 0) fprint_table_variable(fp, t[i], i);
  for (int i=0; i<MAX_HASH_NUMBER; i++) if (t[i].used && t[i].kind == V_DEC && t[i].block == 1) fprint_table_variable(fp, t[i], i);
  for (int i=0; i<MAX_HASH_NUMBER; i++) if (t[i].used && t[i].kind == V_DEC && t[i].block == 2) fprint_table_variable(fp, t[i], i);
}/*}}}*/
static void fprint_table_function(FILE *fp, const TABLE t, const int hash) {/*{{{*/
  assert(t.kind == F_DEC);
  fprintf(fp, "%-10s " , t.str);

  assert(t.root -> kind == PROGRAM); // 宣言の存在する関数 (グローバルなら &(node[0]))
  fprintf(fp, "%-10s ", t.root_name);
  fprintf(fp, "%10s ", "FUNCTION");

  if (t.type == 0) fprintf(fp, "%5s ", "int");
  else             fprintf(fp, "%5s ", "???");

  fprintf(fp, "    - ");              // 関数のサイズは使用しない
  fprintf(fp, "    -     ");          // 関数のアドレスは使用しない
  fprintf(fp, "%5d ", t.token_index); // 変数を参照する際に、未宣言変数でないかチェックする用
  fprintf(fp, "%5d" , hash);
  fprintf(fp, "\n");
}/*}}}*/
static void fprint_table_variable(FILE *fp, const TABLE t, const int hash) {/*{{{*/
  assert(t.kind == V_DEC);
  fprintf(fp, "%-10s " , t.str);
  if      (t.block == 0) {
    assert(t.root -> kind == PROGRAM); // 宣言の存在する関数 (グローバルなら &(node[0]))
    fprintf(fp, "%-10s ", t.root_name);
    fprintf(fp, "%10s ", "VARIABLE");
  } else {
    assert(t.root -> kind == F_DEC);
    fprintf(fp, "%-10s ", t.root_name);
    if      (t.block == 1) fprintf(fp, "%10s ", "ARGUMENT");
    else if (t.block == 2) fprintf(fp, "%10s ", "VARIABLE");
    else assert(t.block <= 2);
  }

  if (t.type == 0) fprintf(fp, "%5s ", "int");
  else             fprintf(fp, "%5s ", "???");

  fprintf(fp, "%5d ", t.size);        // V_DEC intなら4byteなので4

  if (t.block > 0) fprintf(fp, "%5d%4s ", t.address, "+ebp"); // ローカル変数なら [ebp-address]
  else             fprintf(fp, "%9s", "    -     ");          // グローバル変数のアドレスは使用しない

  fprintf(fp, "%5d ", t.token_index); // 変数を参照する際に、未宣言変数でないかチェックする用
  fprintf(fp, "%5d" , hash);
  fprintf(fp, "\n");
}/*}}}*/
static void register_global_v(NODE *node, int *address, TABLE *t, SOURCE s) {/*{{{*/
  assert(node -> kind == V_DEC);
  NODE *var = node -> down -> right;
  assert(get_var_kind(var) == V_KIND_GB_DEC);
  assert(0==strcmp(s.token[node -> down -> init].str, "int"));

  int hash = get_empty_hash(var, V_DEC, t, s);

  // 同名の変数は無いはず
  assert( search_hash_by_condition(var, s.program, V_DEC, t, s) < 0 );

  // 情報を登録
  t[hash].used        = true;
  t[hash].str         = s.token[var -> init].str;
  t[hash].node        = node;
  t[hash].kind        = V_DEC;       // V_DEC or F_DEC or PROTOTYPE
  t[hash].type        = 0;           // intなら0
  t[hash].size        = INT_SIZE;    // V_DEC intなら4byteなので4
  t[hash].token_index = var -> init; // 変数を参照する際に、未宣言変数でないかチェックする用
  t[hash].root        = s.program;   // 宣言の存在する関数 (グローバルなら &(node[0]))
  t[hash].root_name   = "GLOBAL";    // 宣言の存在する場所の名前
  t[hash].block       = 0;           // 宣言の存在するブロックの深さ (グローバルなら0, 引数なら1, 関数内の宣言なら2, if文内の宣言なら3)
  t[hash].address     = 0;           // プログラム全体のサイズが決まってから割当

  *address += INT_SIZE;
}/*}}}*/
static void register_function(NODE *node, TABLE *t, SOURCE s) {/*{{{*/
  assert(node -> kind == F_DEC);
  NODE *var = node -> down -> right;
  assert(get_var_kind(var) == V_KIND_FN_STR);
  assert(0==strcmp(s.token[node -> down -> init].str, "int"));

  // 同名の関数は無いはず
  assert( search_hash_by_condition(var, s.program, F_DEC, t, s) < 0 );

  // プロトタイプとして検索して、ヒットすれば上書き、無ければ新規登録
  bool if_rewrite = false;
  int hash = search_hash_by_condition(var, s.program, PROTOTYPE, t, s);
  if (hash >= 0) if_rewrite = true;
  else hash = get_empty_hash(var, F_DEC, t, s);

  if (if_rewrite) { // 上書き
    // assert(if_args_rational(*(t[hash].node), *node, s));// 引数のチェック
    if_args_rational(*(t[hash].node), *node, s);// 引数のチェック
    t[hash].kind = F_DEC;
    if (var -> init < t[hash].token_index) {
      t[hash].node        = node;
      t[hash].str         = s.token[var -> init].str;
      t[hash].token_index = var -> init;
    }
  }
  else { // 新規登録
    t[hash].used        = true;
    t[hash].str         = s.token[var -> init].str;
    t[hash].node        = node;
    t[hash].kind        = F_DEC;       // V_DEC or F_DEC or PROTOTYPE
    t[hash].type        = 0;           // intなら0
    t[hash].size        =-1;           // 関数を解析してサイズが確定してから決定
    t[hash].token_index = var -> init;
    t[hash].root        = s.program;
    t[hash].root_name   = "GLOBAL";    // 宣言の存在する場所の名前
    t[hash].block       = 0;           // 宣言の存在するブロックの深さ (グローバルなら0, 引数なら1, 関数内の宣言なら2, if文内の宣言なら3)
    t[hash].address     = 0;           // プログラム全体のサイズが決まってから割当
  }
}/*}}}*/
static void register_prototype(NODE *node, TABLE *t, SOURCE s) {/*{{{*/
  assert(node -> kind == PROTOTYPE);
  NODE *var = node -> down -> right;
  assert(get_var_kind(var) == V_KIND_PR_STR);
  assert(0==strcmp(s.token[node -> down -> init].str, "int"));

  // 同名のプロトタイプは無いはず
  assert( search_hash_by_condition(var, s.program, PROTOTYPE, t, s) < 0 );

  // 関数として検索して、ヒットすれば上書き、無ければ新規登録
  bool if_rewrite = false;
  int hash = search_hash_by_condition(var, s.program, F_DEC, t, s);
  if (hash >= 0) if_rewrite = true;
  else hash = get_empty_hash(var, PROTOTYPE, t, s);

  if (if_rewrite) { // 上書き
    assert(if_args_rational(*(t[hash].node), *node, s));// 引数のチェック
    if (var -> init < t[hash].token_index) {
      t[hash].str         = s.token[var -> init].str;
      t[hash].token_index = var -> init;
    }
  }
  else { // 新規登録
    t[hash].used        = true;
    t[hash].str         = s.token[var -> init].str;
    t[hash].node        = node;
    t[hash].kind        = PROTOTYPE;   // V_DEC or F_DEC or PROTOTYPE
    t[hash].type        = 0;           // intなら0
    t[hash].size        =-1;           // 関数を解析してサイズが確定してから決定
    t[hash].token_index = var -> init;
    t[hash].root        = s.program;
    t[hash].root_name   = "GLOBAL";    // 宣言の存在する場所の名前
    t[hash].block       = 0;           // 宣言の存在するブロックの深さ (グローバルなら0, 引数なら1, 関数内の宣言なら2, if文内の宣言なら3)
    t[hash].address     = 0;           // 全グローバル変数が決まって、全関数サイズが決まってから割当
  }
}/*}}}*/
static void register_arguments(NODE *f_dec, TABLE *t, SOURCE s) {/*{{{*/
  assert(f_dec -> kind == F_DEC);

  NODE *v_dec = f_dec -> down -> right -> right;

  int address = 2*INT_SIZE;
  while (v_dec -> kind != FUNCTION) {
    NODE *var = v_dec -> down -> right;
    assert( search_hash_by_condition(var, f_dec    , V_DEC, t, s) < 0 ); // 同名のローカル変数がないことを確認
    assert( search_hash_by_condition(var, s.program, V_DEC, t, s) < 0 ); // 同名のローカル変数がないことを確認
    int hash = get_empty_hash(var, V_DEC, t, s);
    t[hash].used        = true;
    t[hash].str         = s.token[var -> init].str;
    t[hash].node        = v_dec;
    t[hash].kind        = V_DEC;       // V_DEC or F_DEC or PROTOTYPE
    t[hash].type        = 0;           // intなら0
    t[hash].size        = INT_SIZE;    // 関数を解析してサイズが確定してから決定
    t[hash].token_index = var -> init;
    t[hash].root        = f_dec;
    t[hash].root_name   = s.token[f_dec -> down -> right -> init].str; // 宣言の存在する場所の名前
    t[hash].block       = 1;           // 宣言の存在するブロックの深さ (グローバルなら0, 引数なら1, 関数内の宣言なら2, if文内の宣言なら3)
    t[hash].address     = address;     // 全グローバル変数が決まって、全関数サイズが決まってから割当
    address += INT_SIZE;
    v_dec = v_dec -> right;
  }
}/*}}}*/
static void register_local_var(NODE *f_dec, TABLE *t, SOURCE s) {/*{{{*/
  assert(f_dec -> kind == F_DEC);

  NODE *func  = f_dec -> down;
  while (func -> right != NULL) func = func -> right;
  assert(func -> kind == FUNCTION);

  int address = -INT_SIZE;

  func = func -> down;
  while (func != NULL) {
    if (func -> kind == V_DEC) {
      NODE *var = func -> down -> right;
      assert(0==strcmp(s.token[func -> down -> init].str, "int"));

      // 同名の変数はないはず
      assert( search_hash_by_condition(var, s.program, V_DEC, t, s) < 0 );
      assert( search_hash_by_condition(var, f_dec    , V_DEC, t, s) < 0 );

      int hash = get_empty_hash(var, V_DEC, t, s);

      // 情報を登録
      t[hash].used        = true;
      t[hash].str         = s.token[var -> init].str;
      t[hash].node        = func;
      t[hash].kind        = V_DEC;       // V_DEC or F_DEC or PROTOTYPE
      t[hash].type        = 0;           // intなら0
      t[hash].size        = INT_SIZE;    // V_DEC intなら4byteなので4
      t[hash].token_index = var -> init; // 変数を参照する際に、未宣言変数でないかチェックする用
      t[hash].root        = f_dec;       // 宣言の存在する関数 (グローバルなら &(func[0]))
      t[hash].root_name   = s.token[f_dec -> down -> right -> init].str; // 宣言の存在する場所の名前
      t[hash].block       = 2;           // 宣言の存在するブロックの深さ (グローバルなら0, 引数なら1, 関数内の宣言なら2, if文内の宣言なら3)
      t[hash].address     = address;     // ローカル変数なら [ebp-address] グローバル変数なら [address]

      address -= INT_SIZE;
    }

    func = func -> right;
  }
}/*}}}*/
static bool if_args_rational(const NODE n1, const NODE n2, SOURCE s) {/*{{{*/
  assert((n1.kind == F_DEC) || (n1.kind == PROTOTYPE));
  assert((n2.kind == F_DEC) || (n2.kind == PROTOTYPE));
  NODE *d1 = n1.down -> right;
  NODE *d2 = n2.down -> right;

  bool ret = true;
  while (true) {
    bool cont1 = false;
    bool cont2 = false;
    if ((d1 -> right != NULL) && (d1 -> right -> kind == V_DEC)) cont1 = true;
    if ((d2 -> right != NULL) && (d2 -> right -> kind == V_DEC)) cont2 = true;

    if (cont1 != cont2) {
      fprintf(stderr, "Argument number is different.\n");
      fprint_node(stderr, *(d1->up), s.token);
      fprint_node(stderr, *(d2->up), s.token);
    }

    if (cont1) {
      d1 = d1 -> right;
      d2 = d2 -> right;
    } else break;

    ret = (0 == strcmp(s.token[d1 -> down -> init].str, s.token[d2 -> down -> init].str));
    if (!ret) {
      fprintf(stderr, "Invalid argument type;");
      fprint_node(stderr, *(d1->up), s.token);
      fprint_node(stderr, *(d2->up), s.token);
      break;
    }
  }
  return ret;
}/*}}}*/
static TABLE initialize_table() {/*{{{*/
  TABLE ret;
  ret.used        = false;
  ret.str         = NULL;
  ret.kind        = 0;
  ret.type        = 0;
  ret.size        = 0;
  ret.token_index = 0;
  ret.root        = NULL;
  ret.block       = 0;
  ret.address     = 0;
  return ret;
}/*}}}*/
static int two_node_to_hash(NODE *var, const int kind, SOURCE s) {/*{{{*/
  char *str = s.token[var  -> init].str;

  int hash = 0;
  for (int i=0; i<strlen(str); i++) {
    hash += str[i] * (((i+1)%2)*7 + ((i-1)%3)*11);
  }

  hash = hash*13 + kind * 23;
  if (hash < 0) hash = -hash;
  hash = hash % MAX_HASH_NUMBER;

  return hash;
}/*}}}*/
static int get_empty_hash(NODE *var, const int kind, const TABLE *t, SOURCE s) {/*{{{*/
  int hash_base = two_node_to_hash(var, kind, s);
  int hash = hash_base;
  while (t[hash].used == true) {
    hash = (hash+1) % MAX_HASH_NUMBER;
    assert(hash != hash_base);
  }
  return hash;
}/*}}}*/
static bool if_same_hash_num(const int a, const int b) {/*{{{*/
  int delta = (a - b + MAX_HASH_NUMBER) % MAX_HASH_NUMBER;
  return 0 == delta;
}/*}}}*/
static int inc_hash(const int a) {/*{{{*/
  return (a + 1) % MAX_HASH_NUMBER;
}/*}}}*/
static bool if_same_hash_info(const TABLE t, NODE *var, NODE *root, const int kind, SOURCE s) {/*{{{*/
  assert(var -> kind == VARIABLE);
  assert( (root -> kind == PROGRAM)
       || (root -> kind == V_DEC)
       || (root -> kind == F_DEC)
       || (root -> kind == PROTOTYPE));
  bool ret = true;
  if      (!t.used) ret = false;
  else if (t.kind != kind) ret = false;
  else if (0 != strcmp(t.str, s.token[var -> init].str)) ret = false;
  else if (t.root != root) ret = false;
  return ret;
}/*}}}*/
static int search_hash_by_condition(NODE *var, NODE *root, const int kind, TABLE *t, SOURCE s) {/*{{{*/
  // 見つかればhash値を、見つからなければ-1を返す
  int base = two_node_to_hash(var, kind, s);
  int hash = base;
  while (true) {
    if      (if_same_hash_info( t[hash], var, root, kind, s)) break;
    else if (if_same_hash_num( base-1, hash)) {
      hash = -1;
      break;
    }
    hash = inc_hash(hash);
  }
  return hash;
}/*}}}*/
extern int search_hash(NODE *var, TABLE *t, SOURCE s) {/*{{{*/
  // 見つかればhash値を、見つからなければ-1を返す
  const int var_kind = get_var_kind(var);
  int hash;
  if      (var_kind == V_KIND_FN_STR) hash = search_hash_by_condition(var, s.program, F_DEC, t, s);
  else if (var_kind == V_KIND_PR_STR) hash = search_hash_by_condition(var, s.program, F_DEC, t, s);
  else if (var_kind == V_KIND_IN_CAL) hash = search_hash_by_condition(var, s.program, F_DEC, t, s);
  else if (var_kind == V_KIND_GB_DEC) hash = search_hash_by_condition(var, s.program, V_DEC, t, s);
  else if (var_kind == V_KIND_FN_ARG) hash = search_hash_by_condition(var, var -> up, V_DEC, t, s);
  else if (var_kind == V_KIND_PR_ARG) hash = search_hash_by_condition(var, var -> up, V_DEC, t, s);
  else if (var_kind == V_KIND_IN_DEC) hash = search_hash_by_condition(var, var -> up, V_DEC, t, s);

  else if (var_kind == V_KIND_IN_REF) {
    // 関数内で変数を参照する場合、まずローカル変数と思って検索し、失敗したらグローバル変数として検索
    NODE *root = var;
    while (root -> kind != F_DEC) root = root -> up;
    hash = search_hash_by_condition(var, root, V_DEC, t, s);
    if (hash < 0) hash = search_hash_by_condition(var, s.program, V_DEC, t, s);
  }

  else EXIT_NOMSG;

  return hash;
}/*}}}*/
static int get_var_kind(NODE *var) {/*{{{*/
  assert(var -> kind == VARIABLE);

  // V_DECの有無とF_DEC/PROTOTYPEの有無を調べる
  bool if_vdec = false;
  bool if_func = false;
  bool if_fdec = false;
  bool if_prot = false;
  bool if_call = false;

  if (((var -> up) -> kind) == V_DEC)  if_vdec = true;
  if (((var -> up) -> kind) == CALL )  if_call = true;

  NODE *tmp = var;
  while (tmp != NULL) {
    int kind = tmp -> kind;
    if (kind == FUNCTION)  if_func = true;
    if (kind == F_DEC)     if_fdec = true;
    if (kind == PROTOTYPE) if_prot = true;
    tmp = tmp -> up;
  }

  int ret;
  if      (   if_vdec &&   if_fdec &&   if_func && ! if_prot && ! if_call) ret = V_KIND_IN_DEC; // 関数内部での変数宣言
  else if (   if_vdec &&   if_fdec && ! if_func && ! if_prot && ! if_call) ret = V_KIND_FN_ARG; // 関数本体引数宣言
  else if (   if_vdec && ! if_fdec && ! if_func &&   if_prot && ! if_call) ret = V_KIND_PR_ARG; // プロトタイプ引数宣言
  else if (   if_vdec && ! if_fdec && ! if_func && ! if_prot && ! if_call) ret = V_KIND_GB_DEC; // グローバル変数宣言
  else if ( ! if_vdec &&   if_fdec &&   if_func && ! if_prot &&   if_call) ret = V_KIND_IN_CAL; // 関数内部での関数呼出
  else if ( ! if_vdec &&   if_fdec &&   if_func && ! if_prot && ! if_call) ret = V_KIND_IN_REF; // 関数内部での変数参照
  else if ( ! if_vdec &&   if_fdec && ! if_func && ! if_prot && ! if_call) ret = V_KIND_FN_STR; // 関数本体の名前
  else if ( ! if_vdec && ! if_fdec && ! if_func &&   if_prot && ! if_call) ret = V_KIND_PR_STR; // プロトタイプ宣言の名前
  else assert(0);

  return ret;
}/*}}}*/
