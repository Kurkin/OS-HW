#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int main(int argv, char** args) {

    int fd = 0;

    if (argv == 2) {
        fd = open(args[1], O_RDONLY);
        if (fd == -1) {
            perror("очень жаль");
            return -1;
        }       
    } 

    while (!0) {
        size_t buff_size = 5;
        char in[buff_size];
        size_t nread = read(fd, &in[0], buff_size);
        if (nread == -1 && errno == EINTR) {
            printf("EINTR\n");
            continue;
        }
        if (nread == -1) {
            perror("очень жаль");
            return -1;
        }
 
        if (nread <= 0) {
            break;
        } else {
            size_t written = write(1, &in[0], nread);
            while (nread - written != 0) {
                written += write(1, &in[written], nread - written);
            }
        }
    }

    if (fd)
        close(fd);
}
