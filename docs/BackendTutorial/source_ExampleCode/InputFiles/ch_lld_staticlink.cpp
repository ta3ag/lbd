
/// start

extern int test_alloc();

int test_staticlink()
{
  char *ptr = "Hello world!";
  char *np = 0;
  int i = 5;
  unsigned int bs = sizeof(int)*8;
  int mi;
  char buf[80];

  int a = 0;

  a = test_alloc();  // 31
  printf("test_alloc() = %d", a);
  if (a == 31)
    printf(", PASS\n");
  else
    printf(", FAIL\n");
  a = test_global();  // gI = 100
  printf("global variable gI = %d", a);
  if (a == 100)
    printf(", PASS\n");
  else
    printf(", FAIL\n");
  a = test_func_arg_struct();
  a = test_contructor();
  a = test_template();
  printf("test_template() = %d", a); // a = 15
  if (a == 15)
    printf(", PASS\n");
  else
    printf(", FAIL\n");
  a = test_inlineasm();
  printf("test_inlineasm() = %d", a); // a = 53
  if (a == 53)
    printf(", PASS\n");
  else
    printf(", FAIL\n");

  return 0;
}
