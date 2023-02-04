
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/file.h>

#define ONE_MB 1000000

void send_N_to_server(uint32_t N, int sockfd);

void send_file_to_server(uint32_t N, int file_d, int sockfd);

void read_counter_from_server(int sockfd);

int main(int argc, char *argv[]){

    if (argc != 4){
        fprintf(stderr, "invalid number of arguments: %s", strerror(errno));
        exit(1);
    }

    int file_d;
    uint32_t N; /*Number of bytes in file*/
    int sockfd;
    char* server_ip = argv[1];
    uint16_t server_port = atoi(argv[2]);
    char* file_path = argv[3];
    off_t fsize;

    if( (file_d = open(file_path, O_RDONLY)) == -1){
        fprintf(stderr, "file open failed: %s", strerror(errno));
        exit(1);
    }

    /*Find the size of a file in bytes in C : https://stackoverflow.com/questions/6537436/how-do-you-get-file-size-by-fd*/
     fsize = lseek(file_d, 0, SEEK_END);
     N = (uint32_t) fsize;
     lseek(file_d, 0, SEEK_SET);


    /*SOCK_STREAM,0 ---> TCP*/
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "socket creation in client failed: %s", strerror(errno));
        exit(1);
    }


    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port); /*htons for endiannes*/
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr.s_addr); /*translate IP address to binary*/


    if( connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){/*trying to connect to SERVER*/
        fprintf(stderr, "connection in client failed: %s", strerror(errno));
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


    while (total_bytes_sent < 4){/*N is a 4-bytes number*/
        bytes_sent = write(sockfd, &N_to_send + total_bytes_sent, 4 - total_bytes_sent);
        if (bytes_sent < 0){
            fprintf(stderr, "bytes_sent < 0, write N to server failed: %s", strerror(errno));
            exit(1);
        }
        total_bytes_sent  += bytes_sent;
    }
}


void send_file_to_server(uint32_t N, int file_d, int sockfd) {
    char buff[ONE_MB];
    int bytes_read = 1;
    int bytes_sent = 0;
    int notwritten = (int) N;
    int total_bytes_sent = 0;


    while (total_bytes_sent < N) {
        bytes_read = read(file_d, buff, ONE_MB-1);

        if (bytes_read < 0){
            fprintf(stderr, "bytes_read < 0: %s", strerror(errno));
            exit(1);
        }

        notwritten = bytes_read;
        bytes_sent = 0;
        while (notwritten > 0) {
            bytes_sent = write(sockfd, buff+bytes_sent, bytes_read);
            if (bytes_sent < 0) {
                fprintf(stderr, "bytes_sent == -1: %s", strerror(errno));
                exit(1);
            }
            notwritten -= bytes_sent;
            total_bytes_sent += bytes_sent;
        }
    }
    if (total_bytes_sent > N){
        fprintf(stderr, "total_bytes_sent > N - illegal (client sent more bytes than he said): %s", strerror(errno));
        exit(1);
    }
}


void read_counter_from_server(int sockfd) {
    /*receive the counter of printable chars in the file he sent*/

    int total_bytes_read = 0;
    int bytes_read = 0;
    uint32_t counter_from_server;
    uint32_t counter;

    while (total_bytes_read < 4){ /*counter is a 4-bytes number*/
        bytes_read = read(sockfd, &counter_from_server + total_bytes_read , 4 - total_bytes_read);
        if (bytes_read < 0){
            fprintf(stderr, "bytes_read < 0, read counter from server failed: %s", strerror(errno));
            exit(1);
        }
        total_bytes_read += bytes_read;
    }
    close(sockfd);
    counter = ntohl(counter_from_server);
    printf("# of printable characters: %u\n", counter);
    exit(0);
}






