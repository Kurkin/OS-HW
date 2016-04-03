#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

void sigctch(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGUSR1)
        printf("SIGUSR1 from %ld\n", (long)siginfo->si_pid);
    else if (sig == SIGUSR2)
        printf("SIGUSR2 from %ld\n", (long)siginfo->si_pid);
    else 
        printf("SIGINT from %ld\n", (long)siginfo->si_pid);
}

int main(int argv, char** args) {
    
    struct sigaction act;
     
    memset (&act, '\0', sizeof(act));
         
    act.sa_sigaction = &sigctch;
    act.sa_flags = SA_SIGINFO | SA_RESETHAND;
                 
    if (sigaction(SIGUSR1, &act, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

    if (sigaction(SIGUSR2, &act, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }
    
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

    int t = sleep(10);
    if (t == 0) {
        printf("No signals were caught\n");
    } else {
    }

}

