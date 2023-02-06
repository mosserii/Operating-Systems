/* Code sample: using ptrace for simple tracing of a child process.
**
** Note: this was originally developed for a 32-bit x86 Linux system;
** some changes may be required to port to x86-64.
**
** Eli Bendersky (http://eli.thegreenplace.net)
** This code is in the public domain.
*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>

//-------------------------------------------------------------------
void run_target( const char* str_debuggee_name )
{
    /* Allow tracing of this process */
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        perror("ptrace");
        return;
    }

    //Replace this process's image with the given program
    execl(str_debuggee_name, str_debuggee_name, NULL);
}

//-------------------------------------------------------------------
void run_debugger(pid_t child_pid)
{
  int wait_status;
  unsigned long n_instr = 0;

  // The next offset is of addl instruction, 
  // from disaseembler.
  long break_pt_offset = 0x0000000000401151;

  // Wait for child to stop on its first instruction
  wait(&wait_status);
    
  while (WIFSTOPPED(wait_status)) 
  {
    ++n_instr;
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
	
    if( regs.rip == break_pt_offset )
    { 
      // We hit the watchpoint EIP. Get the variable value.
      int curr_val = ptrace( PTRACE_PEEKTEXT, child_pid, 
                             regs.rbp - 8, 0 );
      printf( "\t\tcurr_val = %d\n", curr_val );
    }

    // Make the child execute another instruction
    if( ptrace( PTRACE_SINGLESTEP, child_pid, 0, 0 ) < 0 ) 
    {
      perror("ptrace");
      return;
    }
    
    // Wait for child to stop on its next instruction
    wait(&wait_status);
  }

  printf("Number of instructions %ld\n", n_instr);

}
//-------------------------------------------------------------------
int main(int argc, char** argv)
{
  pid_t child_pid;

  if( argc < 2 )
  {
    printf( "Expected a program name as argument\n" );
    return -1;
  }

  child_pid = fork();
  if( 0 == child_pid )
    run_target( argv[1] );
  else if( child_pid > 0 )
    run_debugger(child_pid);

  return 0;
}
//=========================== END OF FILE ===========================
