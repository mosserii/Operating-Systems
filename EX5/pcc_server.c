
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
#include <signal.h>

#define FIRST_CHAR 32
#define LAST_CHAR 126


void SIGINT_handler();

void print_and_close();

int connfd = -1;
int SIGINT_flag = 0; // indicates if a client is connected, so we need to finish with him and then finish with server
unsigned int pcc_total[LAST_CHAR - FIRST_CHAR + 1]; /*counters array todo malloc?*/




void SIGINT_handler() {

    // no client is connected to server
    if (connfd == -1)
        print_and_close();
    else
        SIGINT_flag = 1;
}



void print_and_close() {
    for (int i = 0; i < LAST_CHAR - FIRST_CHAR + 1; ++i) // adding current client counters to total counters
        printf("char '%c' : %u times\n", (char)(i+FIRST_CHAR), pcc_total[i]);
    exit(0);
}

int main(int argc, char *argv[]){


    uint16_t server_port = atoi(argv[1]); /*todo check if not just unsigned int?*/
    int listenfd;
    int rc;


    int total_bytes_read = 0;
    int bytes_read = 0;
    int unread_bytes = 0;/*todo check if it is possible when the declaration with int is here!!!!*/
    int total_bytes_sent = 0;
    int bytes_sent = 0;


    struct sockaddr_in serv_addr;
    socklen_t addrsize = sizeof(struct sockaddr_in );
    memset(pcc_total, 0, (LAST_CHAR - FIRST_CHAR + 1)* sizeof(unsigned int));

    if (argc != 2){
        fprintf(stderr, "invalid number of arguments in server: %s\n", strerror(errno));/*todo check!*/
        exit(1);
    }

    /*todo important*/
    struct sigaction control_SIGINT; /*todo change*/
    control_SIGINT.sa_handler=&SIGINT_handler;
    control_SIGINT.sa_flags=SA_RESTART;
    if( (rc = sigaction(SIGINT,&control_SIGINT,NULL)) == -1) {
        fprintf(stderr, "socket creation in server failed: %s\n", strerror(errno));
        exit(1);
    }
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

    //BIG LOOP - accepting new connection from device, reading N, reading N bytes, updating counters, sending counter
    while (1){
        int error_with_client = 0;
        connfd = -1;
        int current_counter = 0;
        int counter_to_send = 0;
        unsigned int buffer_for_N;
        unsigned int N;



        if( (connfd = accept(listenfd, NULL, NULL)) == -1){ /*todo check of NULL*/
            fprintf(stderr, "accept in server failed: %s\n", strerror(errno));
            exit(1);
        }


        //READ N FROM CLIENT
        bytes_read = 1; // just to make sure we enter the while loop
        total_bytes_read = 0; // we want 4 bytes
        while (bytes_read > 0){

            bytes_read = read(connfd, &buffer_for_N + total_bytes_read, 4-total_bytes_read);
            total_bytes_read += bytes_read;

            if (bytes_read == 0){//client process is killed unexpectedly or we finished reading
                if (total_bytes_read != 4) {//client process is killed unexpectedly
                    fprintf(stderr, "client process is killed unexpectedly: %s\n", strerror(errno));
                    error_with_client = 1;
                }
            }
            else if (bytes_read < 0) {//bytes_read < 0
                if (!(errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)){
                    fprintf(stderr, "read N from client failed: %s\n", strerror(errno));
                    exit(1);
                } else{/*a TCP error*/
                    fprintf(stderr, "TCP error occured in server: %s\n", strerror(errno));
                    /*todo free reading buffer*/
                    error_with_client = 1;
                }
            }
        }
        if (error_with_client){
            close(connfd);
            connfd = -1;
            if (SIGINT_flag)
                print_and_close();
            continue;
        }

        N = ntohl(buffer_for_N); /*N value, todo check*/


        unsigned int pcc_current[LAST_CHAR - FIRST_CHAR + 1]; /*current client counters*/
        memset(pcc_current, 0, (LAST_CHAR - FIRST_CHAR + 1) * sizeof(unsigned int));

        //Todo  READ file content FROM CLIENT and updating counters, reading char after char
        total_bytes_read = 0;
        bytes_read = 1; // just to make sure we enter the while loop
        unread_bytes = N;/*todo check if it is possible when the declaration with int is here!!!!*/


        while (bytes_read > 0){
            char c;
            if( (bytes_read = read(connfd, &c, sizeof(char))) > 0) {//at least 1 bytes is read
                total_bytes_read += bytes_read;//todo check if not just ++ (char is 1 byte)
                unread_bytes -= bytes_read;
                if (FIRST_CHAR <= c && c <= LAST_CHAR) {
                    current_counter++;
                    pcc_current[c - FIRST_CHAR]++;//todo check if its a number
                }
            }
            else if (bytes_read == 0){//client process is killed unexpectedly
                if (total_bytes_read != N) {
                    fprintf(stderr, "client process is killed unexpectedly: %s\n", strerror(errno));
                    error_with_client = 1;
                }
            }
            else if (bytes_read < 0) {//bytes_read < 0
                if (!(errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)){
                    fprintf(stderr, "read file in server failed: %s\n", strerror(errno));
                    exit(1);
                } else{/*a TCP error*/
                    fprintf(stderr, "TCP error occured in server: %s\n", strerror(errno));
                    /*todo free reading buffer*/
                    error_with_client = 1;
                }
            }
            if (unread_bytes < 0){ //safety check, kind of preventing something like heartbleed that we saw in recrition
                fprintf(stderr, "unread_bytes < 0 - illegal (client sent more bytes than he said): %s\n", strerror(errno));
                exit(1);
            }
        }

        if (error_with_client){
            close(connfd);
            connfd = -1;
            if (SIGINT_flag)
                print_and_close();
            continue;
        }



        // sending COUNTER to Client
        counter_to_send = htonl(current_counter);
        total_bytes_sent = 0;
        bytes_sent= 1; // just to make sure we enter the while loop

        while (bytes_sent > 0){

            bytes_sent = write(connfd, &counter_to_send+total_bytes_sent, 4-total_bytes_sent);//TODO big check here
            total_bytes_sent += bytes_sent;

            if (bytes_sent == 0){//client process is killed unexpectedly or we finished writing
                if (total_bytes_sent != 4) {//client process is killed unexpectedly
                    fprintf(stderr, "client process is killed unexpectedly: %s\n", strerror(errno));
                    error_with_client = 1;
                }
            }
            else if (bytes_sent < 0) {//bytes_sent < 0
                if (!(errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)){
                    fprintf(stderr, "write to client from server failed: %s\n", strerror(errno));
                    exit(1);
                } else{/*a TCP error*/
                    fprintf(stderr, "TCP error occured in server: %s\n", strerror(errno));
                    error_with_client = 1;
                }
            }
        }
        if (error_with_client){
            close(connfd);
            connfd = -1;
            if (SIGINT_flag)
                print_and_close();
            continue;
        }


        for (int i = 0; i < LAST_CHAR - FIRST_CHAR + 1; ++i) // adding current client counters to total counters
            pcc_total[i] += pcc_current[i];

        close(connfd);
        connfd = -1;
        if (SIGINT_flag)
            print_and_close();

        }








}



