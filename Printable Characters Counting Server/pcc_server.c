
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdatomic.h>
#include <signal.h>

#define FIRST_CHAR 32
#define LAST_CHAR 126


void SIGINT_handler();

void print_and_close();

atomic_int connfd = -1;
int SIGINT_flag = 0; // indicates if a client is connected, so we need to finish with him and then finish with server
uint32_t pcc_total[LAST_CHAR - FIRST_CHAR + 1]; /*counters array*/

uint32_t pcc_current[LAST_CHAR - FIRST_CHAR + 1]; /*current client counters*/





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

    if (argc != 2){
        fprintf(stderr, "invalid number of arguments in server: %s\n", strerror(errno));
        exit(1);
    }

    uint16_t server_port = atoi(argv[1]);
    int listenfd;
    int total_bytes_read;
    int bytes_read;
    int unread_bytes;
    int total_bytes_sent;
    int bytes_sent;


    struct sockaddr_in serv_addr;
    socklen_t addrsize = sizeof(struct sockaddr_in );
    memset(pcc_total, 0, (LAST_CHAR - FIRST_CHAR + 1)* sizeof(uint32_t));


    struct sigaction control_SIGINT;
    control_SIGINT.sa_handler=&SIGINT_handler;
    control_SIGINT.sa_flags=SA_RESTART;
    if( (sigaction(SIGINT,&control_SIGINT,NULL)) == -1) {
        fprintf(stderr, "sigaction (handling setting) in server failed: %s\n", strerror(errno));
        exit(1);
    }


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
    serv_addr.sin_port = htons(server_port); // server port address in network byte order

    //connect port to socket
    if( bind(listenfd, (struct sockaddr*) &serv_addr, addrsize) != 0){
        fprintf(stderr, "bind in server failed: %s\n", strerror(errno));
        exit(1);
    }

    //listen on the socket, queue size of 10 clients
    if( listen(listenfd, 10) != 0){
        fprintf(stderr, "bind in server failed: %s\n", strerror(errno));
        exit(1);
    }

    //BIG LOOP - accepting new connection from device, reading N, reading N bytes, updating counters, sending counter
    while (1){
        int error_with_client = 0;
        int current_counter = 0;
        int counter_to_send = 0;
        uint32_t buffer_for_N;
        uint32_t N;


        // accepting new connection
        connfd = accept(listenfd, NULL, NULL);
        if(connfd == -1){
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
                    fprintf(stderr, "TCP error occurred in server: %s\n", strerror(errno));
                    error_with_client = 1;
                }
            }
        }
        // an error has occurred, we need to close the client and not keeping his scores
        if (error_with_client){
            close(connfd);
            connfd = -1;
            //if we received SIGINT while processing this client, we need to finish
            if (SIGINT_flag)
                print_and_close();
            continue;
        }

        N = ntohl(buffer_for_N); /*N value in host byte order*/

        memset(pcc_current, 0, (LAST_CHAR - FIRST_CHAR + 1) * sizeof(uint32_t));

        //READ file content FROM CLIENT and updating counters, reading char by char
        total_bytes_read = 0;
        bytes_read = 1;
        unread_bytes = N;


        while (unread_bytes > 0){
            char c;
            if( (bytes_read = read(connfd, &c, sizeof(char))) > 0) {// if at least 1 bytes is read
                total_bytes_read += bytes_read;
                unread_bytes -= bytes_read;
                if (FIRST_CHAR <= c && c <= LAST_CHAR) {
                    current_counter++;
                    pcc_current[c - FIRST_CHAR]++;
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
                    fprintf(stderr, "TCP error occurred in server: %s\n", strerror(errno));
                    error_with_client = 1;
                }
            }
            if (unread_bytes < 0){ //safety check, kind of preventing something like heartbleed that we saw in recreation
                fprintf(stderr, "unread_bytes < 0 - illegal (client sent more bytes than he said): %s\n", strerror(errno));
                exit(1);
            }
        }

        if (error_with_client){
            close(connfd);
            connfd = -1;
            //if we received SIGINT while processing this client, we need to finish
            if (SIGINT_flag)
                print_and_close();
            continue;
        }



        // sending COUNTER to Client
        counter_to_send = htonl(current_counter);
        total_bytes_sent = 0;
        bytes_sent= 1; // just to make sure we enter the while loop

        while (bytes_sent > 0){

            bytes_sent = write(connfd, &counter_to_send+total_bytes_sent, 4-total_bytes_sent);
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
                    fprintf(stderr, "TCP error occurred in server: %s\n", strerror(errno));
                    error_with_client = 1;
                }
            }
        }
        if (error_with_client){
            close(connfd);
            connfd = -1;
            //if we received SIGINT while processing this client, we need to finish
            if (SIGINT_flag)
                print_and_close();
            continue;
        }


        for (int i = 0; i < LAST_CHAR - FIRST_CHAR + 1; ++i) // adding current client counters to total counters
            pcc_total[i] += pcc_current[i];

        close(connfd);
        connfd = -1;
        if (SIGINT_flag)
            //if we received SIGINT while processing this client, we need to finish
            print_and_close();

        }

}



