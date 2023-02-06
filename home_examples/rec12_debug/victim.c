#include <stdio.h>

void some_func()
{
  long stack_var = 0;
  while( stack_var < 10 )
  {
    ++stack_var;
    printf("stack_var = %ld\n", stack_var);
  }
}

int main(int argc, char** argv )
{
  some_func();
  return 0;
}
