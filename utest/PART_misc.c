#include "../src/misc.c"

void TEST_initialize_token() {/*{{{*/
  TOKEN a = initialize_token();
  mu_assert(a.kind == OTHER_TOKEN);
  mu_assert(strlen(a.str) == 0);
}/*}}}*/
void TEST_compare_token() {/*{{{*/
  TOKEN a = {0,"hoge"};
  TOKEN b = {0,"hoge"};
  TOKEN c = {0,"piyo"};
  TOKEN d = {1,"hoge"};
  TOKEN e = {2,"hogepiyo"};

  mu_assert( compare_token(a,a) == true ) ;
  mu_assert( compare_token(a,b) == true ) ;
  mu_assert( compare_token(a,c) == false) ;
  mu_assert( compare_token(a,d) == false) ;
  mu_assert( compare_token(a,e) == false) ;

  mu_assert( compare_token(b,a) == true ) ;
  mu_assert( compare_token(b,b) == true ) ;
  mu_assert( compare_token(b,c) == false) ;
  mu_assert( compare_token(b,d) == false) ;
  mu_assert( compare_token(b,e) == false) ;

  mu_assert( compare_token(c,a) == false) ;
  mu_assert( compare_token(c,b) == false) ;
  mu_assert( compare_token(c,c) == true ) ;
  mu_assert( compare_token(c,d) == false) ;
  mu_assert( compare_token(c,e) == false) ;

  mu_assert( compare_token(d,a) == false) ;
  mu_assert( compare_token(d,b) == false) ;
  mu_assert( compare_token(d,c) == false) ;
  mu_assert( compare_token(d,d) == true ) ;
  mu_assert( compare_token(d,e) == false) ;

  mu_assert( compare_token(e,a) == false) ;
  mu_assert( compare_token(e,b) == false) ;
  mu_assert( compare_token(e,c) == false) ;
  mu_assert( compare_token(e,d) == false) ;
  mu_assert( compare_token(e,e) == true ) ;
}/*}}}*/
void TEST_if_valid_token() {/*{{{*/
  TOKEN t;
  t = initialize_token();
  mu_assert(if_valid_token(t));
  strcpy(t.str,"1234567890");
  mu_assert(if_valid_token(t));
}/*}}}*/
void TEST_initialize_node() {/*{{{*/
  NODE ret = initialize_node();

  mu_assert( ret.kind         == OTHER_NODE);
  mu_assert( ret.init         <  0);
  mu_assert( ret.last         <  0);
  mu_assert( ret.left         == NULL);
  mu_assert( ret.right        == NULL);
  mu_assert( ret.up           == NULL);
  mu_assert( ret.down         == NULL);
}/*}}}*/
void TEST_compare_node() {/*{{{*/
  NODE n[5];
  NODE a = {7,1,1,false,&(n[0]),&(n[1]),&(n[2]),&(n[3])};
  NODE b = {7,1,1,false,&(n[0]),&(n[1]),&(n[2]),&(n[3])};
  NODE c = {0,1,1,false,&(n[0]),&(n[1]),&(n[2]),&(n[3])};
  NODE d = {7,2,1,false,&(n[0]),&(n[1]),&(n[2]),&(n[3])};
  NODE e = {7,1,1,true ,&(n[1]),&(n[1]),&(n[2]),&(n[3])};

  mu_assert( compare_node(a,a) == true ) ;
  mu_assert( compare_node(a,b) == true ) ;
  mu_assert( compare_node(a,c) == false) ;
  mu_assert( compare_node(a,d) == false) ;
  mu_assert( compare_node(a,e) == false) ;

  mu_assert( compare_node(b,a) == true ) ;
  mu_assert( compare_node(b,b) == true ) ;
  mu_assert( compare_node(b,c) == false) ;
  mu_assert( compare_node(b,d) == false) ;
  mu_assert( compare_node(b,e) == false) ;

  mu_assert( compare_node(c,a) == false) ;
  mu_assert( compare_node(c,b) == false) ;
  mu_assert( compare_node(c,c) == true ) ;
  mu_assert( compare_node(c,d) == false) ;
  mu_assert( compare_node(c,e) == false) ;

  mu_assert( compare_node(d,a) == false) ;
  mu_assert( compare_node(d,b) == false) ;
  mu_assert( compare_node(d,c) == false) ;
  mu_assert( compare_node(d,d) == true ) ;
  mu_assert( compare_node(d,e) == false) ;

  mu_assert( compare_node(e,a) == false) ;
  mu_assert( compare_node(e,b) == false) ;
  mu_assert( compare_node(e,c) == false) ;
  mu_assert( compare_node(e,d) == false) ;
  mu_assert( compare_node(e,e) == true ) ;
}/*}}}*/
void TEST_if_valid_node() {/*{{{*/
  NODE n;
  n = initialize_node();
  mu_assert( !if_valid_node(n) );
  n.init = 2;
  mu_assert( !if_valid_node(n) );
  n.last = 1;
  mu_assert( !if_valid_node(n) );
  n.last = 2;
  mu_assert(  if_valid_node(n) );
  n.last = 3;
  mu_assert(  if_valid_node(n) );
}/*}}}*/
