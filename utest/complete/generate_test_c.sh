#!/bin/sh

if echo $1 |egrep -v '\.c$' > /dev/null ;then echo 'Not C File' ;fi
BASE=$(echo $1 |sed -e 's:../PART_::')
TARGET=TEST_$BASE

echo '#include "mu_test_framework.h"' > $TARGET

egrep '.*void\s+TEST[^(]*\(\s*v?o?i?d?\s*\).*$' $1 |\
  sed -e 's/).*$/);/' \
  >> $TARGET

cat << EOS >> $TARGET
#include "$1"

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;
int mu_if_fail=1;

int main(int argc, char **argv) {
  printf("--------------- UNIT TEST: $BASE ---------------\\n");
EOS

egrep '^.*TEST[^(]*\(\s*v?o?i?d?\s*\).*$' $1 |\
  sed -e 's/(.*$/);/' |\
  sed -e 's/^.*TEST/  mu_run_test\(TEST/' \
  >> $TARGET

cat << EOS >> $TARGET
  mu_show_failures();
  return !(mu_nfail==0);
}
EOS
