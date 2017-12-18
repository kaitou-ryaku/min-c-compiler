#ifndef __MUNIT_H__
#define __MUNIT_H__
#include <stdio.h>

#define COLOR_RED "\x1b[31m"
#define COLOR_GRN "\x1b[32m"
#define COLOR_DEF "\x1b[39m"

#define MU_FAILED(file,line,expr)  if (mu_if_fail==0) {mu_if_fail=1;printf("\n");} printf("%sFAILED %s:%u: `%s'%s\n",COLOR_RED,file,line,expr,COLOR_DEF);
#define MU_TESTED(test,tot,fail)   printf("%-30s %s(%02d/%02d) ... %s%s\n",test,(fail)?COLOR_RED:COLOR_GRN,tot-fail,tot,(fail)?"FAILED\n":"OK",COLOR_DEF);
#define mu_assert(expr)            do{mu_nassert++;if (!(expr)) {++mu_nfail;MU_FAILED(__FILE__,__LINE__,#expr);}}while(0)
#define mu_run_test(test)          do{++mu_ntest; int mu_tmp_a=mu_nassert, mu_tmp_f=mu_nfail; test(); mu_if_fail=mu_nfail-mu_tmp_f; MU_TESTED(#test,(mu_nassert-mu_tmp_a),mu_if_fail);}while(0)
#define mu_show_failures()         printf("### TOTAL: %d TEST%*s%s(%d/%d) ... %s%s\n",mu_ntest,14,"",(mu_nfail)?COLOR_RED:COLOR_GRN,mu_nassert-mu_nfail,mu_nassert,(mu_nfail)?"FAILED":"ALL TEST CLEAR",COLOR_DEF);
extern int mu_nfail;
extern int mu_ntest;
extern int mu_nassert;
extern int mu_if_fail;
#endif /* __munit_h__ */
