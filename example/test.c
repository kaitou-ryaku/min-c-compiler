int func( int a, int b);
int x;

int main() {
  int a;
  int e;
  int fg;
  e  = 2*(3+4*(5%6+7)%8+9);
  fg = ((2+e/e*(e+3*e)%(4*(5*6+7+e+func(e,e)))));

  func(3,a);

  while (e > fg) {
    fg = fg + 1;
    if (fg == e) {
      fg = e;
    } else {
      e = (0-1)*fg;
      fg = fg + 1;
    }
  }

  {{{{a=3;}}}}

  return 0;
}

int y;
int z;

int func( int c, int b) {
  int f;
  f=3;
  x = 4;
  return (c+b*x)%f;
}
