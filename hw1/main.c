#include<stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

int main() {
    while (!0) {
        size_t buff_size = 5;
        char in[buff_size];
        size_t nread = read(0, &in[0], buff_size);
        if (nread <= 0) {
            break;
        } else {
            size_t written = write(1, &in[0], nread);
            while (nread - written != 0) {
                written += write(1, &in[written], nread - written);
            }
        }
    }
}
