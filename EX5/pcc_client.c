
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

#define ONE_MB 1000000

void send_N_to_server(uint32_t N, int sockfd);

void send_file_to_server(uint32_t N, FILE *file_d, int sockfd);

void read_counter_from_server(int sockfd);

int main(int argc, char *argv[]){

    FILE* file_d; /*todo check if not int*/
    uint32_t N; /*Number of bytes in file*/ /*todo check if not uint32_t?*/
    int sockfd;
    char* server_ip = argv[1];
    uint16_t server_port = atoi(argv[2]); /*todo check if not just unsigned int?*/
    char* file_path = argv[3];



    struct sockaddr_in serv_addr; // where we Want to get to
    struct sockaddr_in my_addr;   // where we actually connected through todo check if needed
    struct sockaddr_in peer_addr; // where we actually connected to todo check if needed
    socklen_t addrsize = sizeof(struct sockaddr_in );
    unsigned char buf[sizeof(struct in6_addr)];/*todo check!!!!!!!*/

    if (argc != 4){
        fprintf(stderr, "invalid number of arguments: %s\n", strerror(errno));/*todo check!*/
        exit(1);
    }

    if( (file_d = open(file_path, O_RDONLY)) == -1){
        fprintf(stderr, "file open failed: %s\n", strerror(errno));
        exit(1);
    }

    /*Find the size of a file in bytes in C : https://www.techiedelight.com/find-size-of-file-c/*/
    fseek(file_d, 0L, SEEK_END);    // seek to the EOF
    N = ftell(file_d);              // get the current position
    rewind(file_d);                 // rewind to the beginning of file


    /*SOCK_STREAM,0 ---> TCP*/
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "socket creation in client failed: %s\n", strerror(errno));
        exit(1);
    }


    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port); /*htons for endiannes*/
    serv_addr.sin_addr.s_addr = inet_pton(AF_INET, server_ip, buf); // todo chceck...

    if( connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
        fprintf(stderr, "connection in client failed: %s\n", strerror(errno));
        exit(1);
    }

    send_N_to_server(N, sockfd);
    send_file_to_server(N, file_d, sockfd);
    read_counter_from_server(sockfd);


    // close socket
    close(sockfd);
    exit(0);
}



void send_N_to_server(uint32_t N, int sockfd) {

    uint32_t N_to_send = htonl(N);
    int bytes_sent = -1;
    int total_bytes_sent = 0;

    // keep looping until nothing left to write
    while (total_bytes_sent < 4){
        bytes_sent = write(sockfd, &N_to_send + total_bytes_sent, 4 - total_bytes_sent);/*todo fix!*/
        // todo check if error occured (client closed connection?)
        if (bytes_sent < 0){
            fprintf(stderr, "bytes_sent < 0, write N to server failed: %s\n", strerror(errno));
            exit(1);
        }
        total_bytes_sent  += bytes_sent;
    }

}


void send_file_to_server(uint32_t N, FILE* file_d, int sockfd) {
    char *buff[ONE_MB]; /* todo check about star */
    int bytes_read = 0;//todo check if not long
    int bytes_sent = 0;
    int notwritten = (int) N;
    int total_bytes_sent = 0;

    while ((bytes_read = read(sockfd, buff, ONE_MB)) != 0) {
        notwritten = bytes_read;
        while (notwritten > 0) {
            if ((bytes_sent = write(sockfd, buff, bytes_read) == -1)) {
                fprintf(stderr, "bytes_sent == -1: %s\n", strerror(errno));
                exit(1);
            }
            notwritten -= bytes_sent;
            total_bytes_sent += bytes_sent;
        }
    }
    if (total_bytes_sent > N){
        fprintf(stderr, "total_bytes_sent > N - illegal (client sent more bytes than he said): %s\n", strerror(errno));
        exit(1);
    }
}



void read_counter_from_server(int sockfd) {

    int total_bytes_read = 0;
    int bytes_read = 0;
    uint32_t counter_from_server;
    uint32_t counter;

    while (total_bytes_read < 4){
        bytes_read = read(sockfd, &counter_from_server + total_bytes_read , 4 - total_bytes_read);
        if (bytes_read < 0){
            fprintf(stderr, "bytes_read < 0, read counter from server failed: %s\n", strerror(errno));
            exit(1);
        }
        total_bytes_read += bytes_read;
    }
    close(sockfd);
    counter = ntohl(counter_from_server);
    printf("# of printable characters: %u\n", counter);
    exit(0);
}






