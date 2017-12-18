#ifndef __TABLE__
#define __TABLE__

void make_global_table(TABLE *t, SOURCE s);
void make_local_table(NODE *f_dec, TABLE *t, SOURCE s);
void delete_local_var(TABLE *t);
void fprint_table(FILE *fp, TABLE *t);
int  search_hash(NODE *var, TABLE *t, SOURCE s);

#endif
