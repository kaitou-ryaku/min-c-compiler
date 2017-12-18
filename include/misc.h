#ifndef __MISC__
#define __MISC__

char *token_kind_str(const int kind);
void fprint_token(FILE *fp, const TOKEN token);
void fprint_all_token(FILE *fp, const TOKEN *t, const int max_token_num);
void fprint_token_series(FILE *fp, const TOKEN *t, const int init, const int last);
TOKEN initialize_token(void);
bool  compare_token(TOKEN t1, TOKEN t2);
bool if_valid_token(const TOKEN t);

char *node_kind_str(const int kind);
void fprint_node(FILE *fp, const NODE n, const TOKEN *t);
NODE initialize_node( void);
bool compare_node(NODE n1, NODE n2);
bool if_valid_node(const NODE n);

#endif
