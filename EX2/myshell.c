


#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <signal.h>

#define IGNORE 0
#define DEFAULT 1
#define SIG_H_ERROR "Error : signal handling failed"
#define FORK_ERROR "Error : fork failed"
#define WAIT_ERROR "Error : waitpid failed"
#define PIPE_ERROR "Error : pipe failed"
#define EXECVP_ERROR "Error : execvp failed"
#define DUP_ERROR "Error : dup2 failed"
#define OPEN_FILE_ERROR "Error : file opening failed"


int contains_symbol(char *string);
int background_exe(char **arglist, int count);
int piping_exe(char **arglist, int i);
int redirecting_exe(char **arglist, int i);
int command_exe(char **arglist);
int signal_handling(int signal_type, int i);




int prepare(void){

    if (signal_handling(SIGINT, IGNORE) == -1)/*parent (shell) should not terminate upon SIGINT*/
        return -1;
    if (signal_handling(SIGCHLD, IGNORE) == -1)/*child processes will get killed when they finish - no Zombies*/
        return -1;
    return 0; /*on success*/
}

int process_arglist(int count, char** arglist){
    int i;
    int bool_symbol = 0;

    for (i = 0; i < count; i++){
        if (contains_symbol(arglist[i])){
            bool_symbol = 1;
            break;
        }
    }


    if (bool_symbol){ /*contains a symbol of {&,|,>}*/
        char symbol = *arglist[i];
        switch (symbol) {
            case '&':
                return background_exe(arglist, count);
                break;
            case '|':
                return piping_exe(arglist, i);
                break;
            case '>':
                return redirecting_exe(arglist, i);
                break;
        }
    }
    return command_exe(arglist);
}


int background_exe(char **arglist, int count) {

    pid_t pid = fork();
    if (pid == -1){/*error : fork() failed in parent process*/
        perror(FORK_ERROR);
        return 0;
    }
    if (pid == 0){ /*child process*/
        if (signal_handling(SIGINT, IGNORE) == -1){ /*todo maybe before fork : sigaction instead - important and a must!!!!!!!!*/
            perror(SIG_H_ERROR);/*todo check if needed*/
            exit(1);
        }

        /*not passing '&' to execvp(), the last element of argv must be a NULL pointer*/
        arglist[count-1] = NULL;

        /*1st arg is the command, 2nd arg contains the complete command, along with its arguments*/
        if (execvp(arglist[0],arglist) == -1 ){
            perror(EXECVP_ERROR);
            exit(1);
        }
    }
    return 1; /*parent process will go to this line immediately and does not need to wait for child process to finish*/
}

int piping_exe(char **arglist, int i) {
    /*arglist[i] == '|'*/
    arglist[i] = NULL; /*we need to split arlist into "2 arrays" so we can pass it to execvp*/

    /*child1 is writing , child2 is reading*/
    int pfds[2]; /*pfds[0] - READ , pfds[1] - WRITE*/
    if (pipe(pfds) == -1){/* error : pipe() creation failed in parent process*/
        perror(PIPE_ERROR);
        return 0;
    }
    pid_t child1 = fork();

    if (child1 == -1){/* error : fork() failed in parent process*/
        perror(FORK_ERROR);
        close(pfds[0]);
        close(pfds[1]);
        return 0;
    }

    if (child1 == 0){
        /*default handling - foreground process*/
        if (signal_handling(SIGINT,DEFAULT) == -1){/*signal_handling failed in child process - exit only from child*/
            perror(SIG_H_ERROR);/*todo check if needed*/
            exit(1);
        }

        close(pfds[0]);/*close read side*/

        /*replace STDOUT with file descriptor*/
        if(dup2(pfds[1], STDOUT_FILENO) == -1){
            perror(DUP_ERROR);
            exit(1);
        }
        close(pfds[1]);/*close write side after duplicating it*//*todo big check about this close line*/

        if (execvp(arglist[0], arglist) == -1){
            perror(EXECVP_ERROR);
            exit(1);
        }
    }
    /* (parent) */
    pid_t child2 = fork();
    if (child2 == -1){/*error : fork() failed in parent*/
        perror(FORK_ERROR);
        close(pfds[0]);
        close(pfds[1]);
        return 0;
    }

    if (child2 == 0){
        /*default handling - foreground process*/
        if (signal_handling(SIGINT,DEFAULT) == -1){
            perror(SIG_H_ERROR);/*todo check if needed*/
            exit(1);
        }
        close(pfds[1]);/*close read side*/

        /*replace STDIN with file descriptor*/
        if(dup2(pfds[0], STDIN_FILENO) == -1){
            perror(DUP_ERROR);
            exit(1);
        }
        close(pfds[0]);/*close write side after duplicating it todo check*//*todo big check about this close line*/

        if (execvp(arglist[i+1], &arglist[i+1]) == -1){/*command and args after '|'*/
            perror(EXECVP_ERROR);
            exit(1);
        }
    }
    /*parent*/
    close(pfds[0]);
    close(pfds[1]);

    int status;
    if (waitpid(child1,&status,0) == -1){
        if(errno != EINTR && errno != ECHILD){
            perror(WAIT_ERROR);
            return 0;
        }
    }

    if (waitpid(child2,&status,0) == -1){
        if(errno != EINTR && errno != ECHILD){
            perror(WAIT_ERROR);
            return 0;
        }
    }
    return 1;
}



int redirecting_exe(char **arglist, int i) {

    arglist[i] = NULL;
    int fd;
    /*fd = open(arglist[i+1], O_WRONLY | O_APPEND | O_CREAT, 0644);*/ /*arglist[i+1] = filename*/
    fd = open(arglist[i+1], O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (fd < 0){
        perror(OPEN_FILE_ERROR);
        return 0;
    }

    pid_t child = fork();
    if (child == -1){/*error : fork() failed in parent process*/
        perror(FORK_ERROR);
        return 0;
    }
    if (child == 0){
        if (signal_handling(SIGINT,DEFAULT) == -1){
            perror(SIG_H_ERROR);/*todo check if needed*/
            exit(1);
        }


        if (dup2(fd, STDOUT_FILENO) == -1){
            perror(DUP_ERROR);
            exit(1);
        }
        close(fd); /*we just duplicated it with STDOUT so we can close it*//*todo big check about this close line*/


        if (execvp(arglist[0], arglist) == -1){
            perror(EXECVP_ERROR);
            exit(1);
        }

    }
    /*(parent)*/
    int status;
    if (waitpid(child, &status, 0) == -1){
        if(errno != EINTR && errno != ECHILD){
            perror(WAIT_ERROR);
            return 0;
        }
    }
    return 1;
}


int command_exe(char **arglist) {

    pid_t child = fork();
    if (child == -1){
        perror(FORK_ERROR);
        return 0;
    }
    if (child == 0){
        if (signal_handling(SIGINT,DEFAULT) == -1){
            perror(SIG_H_ERROR);/*todo check if needed*/
            exit(1);
        }

        if (execvp(arglist[0], arglist) == -1){
            perror(EXECVP_ERROR);
            exit(1);
        }

    }
    /*(parent)*/
    int status;
    if (waitpid(child, &status, 0) == -1){
        if(errno != EINTR && errno != ECHILD){
            perror(WAIT_ERROR);
            return 0;
        }
    }
    return 1;
}


int contains_symbol(char *string) {/*todo check if not const char*/
    return (*string == '&' || *string == '|'|| *string == '>');
}

int signal_handling(int signal_type, int i) {
    /*signal handling code is inspired by this forum :
     * https://stackoverflow.com/questions/40601337/what-is-the-use-of-ignoring-sigchld-signal-with-sigaction2/40601403#40601403*/
    struct sigaction sa;
    sa.sa_handler = (i == IGNORE) ? SIG_IGN : SIG_DFL; /*ignore signal*/
    sa.sa_flags = SA_RESTART;/*todo check, espically about DFL*/
    if(sigaction(signal_type, &sa, NULL) == -1)
    {
        /*todo perror();*/
        return -1;
    }
    return 1;
}





int finalize(void){

    return 0;
}