
extern int foo(int x1, int x2);
extern int bar();

int main()
{
  int a = foo(1, 2);
  a += bar();
  
  return a;
}
