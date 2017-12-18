#ifndef __ASSEMBLER__
#define __ASSEMBLER__

extern void fprint_symbol_table(FILE *fp, SOURCE s, TABLE *tb);
extern void asm_program(FILE *fp, SOURCE s, TABLE *tb);

#endif
