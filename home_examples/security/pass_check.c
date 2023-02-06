#include <stdio.h>
#include <string.h>

//
// gcc pass_check.c -fno-stack-protector -o pass_check
// python -c 'print "a"*5' | ./pass_check
//
int main(void)
{
  int pass = 0;
  char buff[4];

  puts( "Password:" );
  scanf( "%s", buff );
  //gets(buff);

  if( strcmp( buff, "123" ) )
    puts( "Wrong Password" );
  else
  {
    puts( "Correct Password" );
    pass = 1;
  }

  if( pass )
    puts("Root permissions granted");

  printf( "buff[0]  address is %p\n", &buff[0]  );
  printf( "buff[3]  address is %p\n", &buff[3] );
  printf( "pass     address is %p\n", &pass     );
  printf( "pass = %d\n", pass );
  
  return 0;
}

