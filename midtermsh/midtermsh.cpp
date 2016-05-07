#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <vector>

using namespace std;

int pipe2(int fields[2], int flag) { // amazing OSX 
   
    if (flag != O_NONBLOCK && flag != FD_CLOEXEC) {
        errno = EINVAL;
        return -1;
    }

    if (pipe(fields)) 
        return -1;

    if (fcntl(fields[0], F_SETFL, flag)) {
        errno = EFAULT;
        return -1;
    }

    if (fcntl(fields[1], F_SETFL, flag)) {
        errno = EFAULT;
        return -1;
    }

    return 0;
}

vector<pid_t> childs;

void sigctch(int sig, siginfo_t *siginfo, void *context) {
    for (auto child : childs) 
        kill(child, sig);
}

const size_t buff_size = 1024;
size_t overred_size = 0;
char overred[buff_size];

string readline() {
    string s{overred, overred_size};
    overred_size = 0;
    char buff[buff_size];
    while (true) {
        size_t nread = read(0, &buff[0], buff_size);
        if (nread == 0 || nread == string::npos) // npos is just to supress warning
            break;
        string red = string{buff, nread}; 
        size_t cr = red.find("\n"); 
        if (cr == string::npos) {
            s += red;
        } else {
            s += red.substr(0, cr);
            for (size_t j = cr + 1; j < nread; j++) { // find lib function
                overred[j - cr - 1] = buff[j];
            }
            overred_size = nread - cr - 1;
            break;
        }
    }
    return s;
}

int main () {
    
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_sigaction = &sigctch;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

    while (true) {
        write(1, "&", 1); // just for sure that "& " will be printed
        write(1, " ", 1);
        string command = readline();
        if (command == "") 
            break;
        size_t e = 0;
        size_t cnt = 0;
        int first_pipe[2];
        int second_pipe[2];
        while (e != string::npos) {
            e = command.find('|');
            auto part = command.substr(0, e == string::npos ? command.length() : e);
            command = e == string::npos ? "" : command.substr(e + 1);
            const auto strBegin = part.find_first_not_of(" ");
            const auto strEnd = part.find_last_not_of(" \n");
            const auto strRange = strEnd - strBegin + 1;
            part = part.substr(strBegin, strRange);
            size_t space = part.find(" ");
            string com = part.substr(0, space == string::npos ? part.length() : space);
            string args = space == string::npos ? "" : part.substr(space + 1, part.length());
        
            if (e != string::npos) 
                if (pipe2(second_pipe, FD_CLOEXEC)) 
                    return 1;

            pid_t child = fork();
            if (child == -1) 
                return 1; // enable to fork, nothing to do here

            if (!child) { //child
                //cout << com << " first pipe " << first_pipe[0] << " " << first_pipe[1] << ", second " << second_pipe[0] << " " <<second_pipe[1] << "\n";
                if (cnt) dup2(first_pipe[0], 0); // read from pipe
                if (e != string::npos) dup2(second_pipe[1], 1); // write to pipe
                if (cnt) { close(first_pipe[0]); close(first_pipe[1]); } // magic
                const char* a[3] = {com.c_str(), args == "" ? NULL : args.c_str(), NULL};
                if (execvp(com.c_str(), (char* const*)a)) 
                    cerr << "Unsupperted command: " << com << "\n";
                return 0;
            }
            
            if (cnt) close(first_pipe[0]); // can be interrupted by signal, TODO: check for EINTR
            if (cnt) close(first_pipe[1]);
            childs.push_back(child);
            first_pipe[0] = second_pipe[0];
            first_pipe[1] = second_pipe[1];
            cnt++;    
        }
        
        pid_t wpid;
        int status;
        while ((wpid = wait(&status)) > 0) { } 
        childs.clear();
    }

}
