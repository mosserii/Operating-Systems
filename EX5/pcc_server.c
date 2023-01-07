
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/file.h>
#include <stdatomic.h>

#define FIRST_PRINTABLE_CHAR 32
#define LAST_PRINTABLE_CHAR 126

int main(int argc, char *argv[]){


    uint16_t server_port = atoi(argv[1]); /*todo check if not just unsigned int?*/
    int listenfd;

    struct sockaddr_in serv_addr;
    socklen_t addrsize = sizeof(struct sockaddr_in );

    unsigned int pcc_total[LAST_PRINTABLE_CHAR - FIRST_PRINTABLE_CHAR + 1]; /*counters array*/



    /*todo important*/
    struct sigaction control_SIGINT;
    control_SIGINT.sa_handler=&SIGINT_action;
    control_SIGINT.sa_flags=SA_RESTART;
    return_value=sigaction(SIGINT,&control_SIGINT,NULL);
    error_occured_exit(return_value==-1,SIGACTION_ERROR);
    /*todo */

    /*TCP*/
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket creation in server failed: %s\n", strerror(errno));
        exit(1);
    }


    /*this piece of code is taken from : https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr*/
    if (  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        fprintf(stderr, "setsockopt in server failed: %s\n", strerror(errno));
        exit(1);
    }

    memset(&serv_addr, 0, addrsize);

    serv_addr.sin_family = AF_INET;
    // INADDR_ANY = any local machine address
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(server_port);

    if( bind(listenfd, (struct sockaddr*) &serv_addr, addrsize) != 0){
        fprintf(stderr, "bind in server failed: %s\n", strerror(errno));
        exit(1);
    }

    if( listen(listenfd, 10) != 0){
        fprintf(stderr, "bind in server failed: %s\n", strerror(errno));
        exit(1);
    }

    while (1){
        atomic_int connfd = -1;


        if( (connfd = accept(listenfd, NULL, NULL)) == -1){ /*todo check*/
            fprintf(stderr, "accept in server failed: %s\n", strerror(errno));
            exit(1);
        }




    }





}