
/// start
int foo(int x1, int x2)
{
  int sum = x1 + x2;
  
  return sum; 
}

int foo1(int x1, int x2)
{
  int sum = x1 + x2;
  
  return sum; 
}

int bar()
{
  int a = foo(2, 2);
  a = foo1(2, 2);
//  int a = 5;

  return a;
}

