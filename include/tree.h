#ifndef __TREE__
#define __TREE__
NODE *make_down_tree(const int init, const int last, NODE *up, NODE *left, int *node_index, TOKEN *token, NODE *node);
NODE  make_root_node(const int max_node_num);
#endif
