


#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/fcntl.h>

/*todo checks about unistd.h for fork*/

int contains_symbol(char *string);

int background_exe(char **arglist, int count);

int piping_exe(char **arglist, int i);

int redirecting_exe(char **arglist, int i);

int prepare(void){

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
        switch (symbol) { //todo check if last row is necessary and if the breaks are
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
    if (pid == -1){/* todo error : fork() failed in parent process so we need to print error and return 0 */
        /*todo print error!!!!*/
        return 0;
    }
    if (pid == 0){ /*child process*/
        if (signal(SIGINT, SIG_IGN)){ /*todo sigaction instead - important and a must!!!!!!!!*/

        }

        /*not passing '&' to execvp(), the last element of argv must be a NULL pointer*/
        arglist[count-1] = NULL;

        /*1st arg is the command, 2nd arg contains the complete command, along with its arguments*/
        if (execvp(arglist[0],arglist) == -1 ){
            /*the command fails for some reason, make child process exits*/
            /*fprintf(stderr, "%s\n", "execvp in background_exe failed"); *todo print_error() check*/
            fprintf(stderr, "%s\n", strerror(errno));/*todo print_error() check*/
            exit(1);
        }
    }
    return 1; /*parent process will go to this line immediately*/
}

int piping_exe(char **arglist, int i) {
    /*arglist[i] == '|'*/
    arglist[i] = NULL; /*we need to split arlist into "2 arrays" so we can pass it to execvp*/
    /*child1 is writing , child2 is reading*/
    int pfds[2]; /*pfds[0] - READ , pfds[1] - WRITE*/
    if (pipe(pfds) == -1){/* todo error : pipe() failed in parent process so we need to print error and return 0 */
        /*todo print error!!!!*/
        return 0;
    }
    pid_t child1 = fork();

    if (child1 == -1){/* todo error : fork() failed in parent process so we need to print error and return 0 */

        /*todo print error!!!!*/

        close(pfds[0]);
        close(pfds[1]);
        return 0;
    }

    if (child1 == 0){
        /*todo
        if(signal_DFL(SIGINT)==-1)
        { Error handling 4-5
            exit(1);
        }*/


        close(pfds[0]);/*close read side*/
        dup2(pfds[1], STDOUT_FILENO); /*replace STDOUT with file descriptor*/
        close(pfds[1]);/*close write side after duplicating it*/

        if (execvp(arglist[0], arglist) == -1){
            /*todo
         ERROR
             exit(1);
         }*/
        }
    } else{ /*(parent) todo check */
        pid_t child2 = fork();
        if (child2 == -1){/* todo error : fork() failed in parent process so we need to print error and return 0 */

            /*todo print error!!!!*/

            close(pfds[0]);
            close(pfds[1]);
            return 0;
        }
        if (child2 == 0){
            /*todo
        if(signal_DFL(SIGINT)==-1)
        { Error handling 4-5
            exit(1);
        }*/
            close(pfds[1]);/*close read side*/

            dup2(pfds[0], STDIN_FILENO);/*replace STDIN with file descriptor*/
            close(pfds[0]);/*close write side after duplicating it todo check*/

            if (execvp(arglist[i+1], &arglist[i+1]) == -1){/*command and args after '|'*/
                /*todo
                ERROR
                    exit(1);
                    }*/
            }
        }
        /*parent*/
        close(pfds[0]);
        close(pfds[1]);

        int status;
        if (waitpid(child1, &status, 0) != -1 || waitpid(child2, &status, 0) != -1){
            if(errno != EINTR && errno != ECHILD)
            {
                /*TODO ERROR
                perror(WAIT_ERROR);
                return 0;*/
            }
        }
        return 1;
    }
}


int redirecting_exe(char **arglist, int i) {

    arglist[i] = NULL;
    int fd;
    fd = open(arglist[i+1], O_WRONLY | O_APPEND | O_CREAT, 0644); /*arglist[i+1] = filename*/

    if (fd < 0){
        /*todo error*/
    }


    pid_t child = fork();
    if (child == -1){
        /* todo error : fork() failed in parent process so we need to print error and return 0 */

        return 0;
    }
    if (child == 0){

        if(signal_DFL(SIGINT)==-1){
            /*todo
                ERROR
                    exit(1);
                    }*/
        }


        if (dup2(fd, STDOUT_FILENO) == -1){
            /*todo
                ERROR
                    exit(1);
                    }*/
        }
        close(fd);


        if (execvp(arglist[0], arglist) == -1){
            /*todo
                ERROR
                    exit(1);
                    }*/
        }

    }






    return 1;
}


int contains_symbol(char *string) {
    return (*string == '&' || *string == '|'|| *string == '>');
}


int finalize(void){

}