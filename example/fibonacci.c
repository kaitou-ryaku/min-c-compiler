int fib(int n);

int main() {
  int a;
  a = fib(10);
  return a;
}

int fib(int n) {
  int ret;
  if (n > 2) {
    ret = fib(n-2) + fib(n-1);
  } else {
    if (n == 2) {ret = 1;}
    else {
      if (n == 1) {ret = 1;}
    }
  }

  return ret;
}
