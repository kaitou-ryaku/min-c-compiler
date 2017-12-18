#include "../include/common.h"
#include "../include/misc.h"
#include "../include/node_edit.h"

extern void fresh_tree(SOURCE s) {/*{{{*/
  for (int i=0; i<s.max_node_num; i++) {
    s.program[i].arrived = false;
  }
}/*}}}*/
extern NODE *go_next_node(NODE *node) {/*{{{*/
  NODE *ret;
  if (node -> arrived) {
    if      (node -> right != NULL) ret = node -> right; // 関数をvoidにして node =  node -> right にするとエラーになる
    else if (node -> up    != NULL) ret = node -> up;
    else                            ret = NULL; // node[0] に帰還
  } else {
    node -> arrived = true;
    if      (node -> down  != NULL) ret = node -> down;
    else if (node -> right != NULL) ret = node -> right;
    else if (node -> up    != NULL) ret = node -> up;
    else                            ret = NULL;
  }
  return ret;
}/*}}}*/
extern void delete_node_recursive(NODE *node) {/*{{{*/
  NODE *up    = node -> up;
  assert(up != NULL); // node[0]のPROGRAMは消せない仕様

  NODE *left  = node -> left;
  NODE *right = node -> right;
  if        (left != NULL && right != NULL) {
    left  -> right = right;
    right -> left  = left;
  } else if (left != NULL && right == NULL) {
    left  -> right = NULL;
  } else if (left == NULL && right != NULL) {
    up    -> down  = right;
    right -> left  = NULL;
  } else if (left == NULL && right == NULL) {
    up    -> down  = NULL;
  }
}/*}}}*/
extern void replace_by_solitary_node(NODE *node) {/*{{{*/
  //   A   B   C     A   S   C
  //       S     -->    DEF
  //      DEF

  NODE *up   = node -> up;
  if ((up != NULL) && (node -> left == NULL) && (node -> right == NULL)) {
    up -> kind = node -> kind;   // 上を孤独な自分で置き換えて、自分を削除
    up -> init = node -> init;
    up -> last = node -> last;
    up -> down = node -> down;   // 関数コールの場合は引数を上にコピーする必要がある

    NODE *down = node -> down;
    while (down != NULL) {
      down -> up = up;
      down = down -> right;
    }
  }
}/*}}}*/
extern int count_horizontal(NODE *node) {/*{{{*/
  NODE *left = node;
  while (left -> left != NULL) left = left -> left;
  int count = 1;
  while (left -> right != NULL) {
    left = left -> right;
    count++;
  }
  return count;
}/*}}}*/
extern int count_vertical(NODE *node) {/*{{{*/
  int count = 0;
  NODE *up_n = node -> up;
  while (up_n != NULL) {
    count++;
    up_n = up_n -> up;
  }
  return count;
}/*}}}*/
