#ifndef __NODE_EDIT__
#define __NODE_EDIT__

NODE *go_next_node(NODE *node);
void fresh_tree(SOURCE s);
void delete_node_recursive(NODE *node);
void replace_by_solitary_node(NODE *node);
int  count_horizontal(NODE *node);
int  count_vertical(  NODE *node);

#endif
