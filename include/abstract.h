#ifndef __ABSTRACT__
#define __ABSTRACT__

SOURCE abstract_tree(SOURCE s);
void fprint_tree(FILE *fp, NODE *node, SOURCE s);
void fprint_tree_dot_style(FILE *fp, NODE *node, SOURCE s);

#endif
