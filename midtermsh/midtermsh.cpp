#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <vector>

using namespace std;

vector<pid_t> childs;

void sigctch(int sig, siginfo_t *siginfo, void *context) {
    for (auto child : childs) {
        kill(child, sig);
    }
}

string readline() {
    string s;
    size_t buff_size = 5;
    char in[buff_size];
    bool f = false;
    while (!f) {
        size_t nread = read(0, &in[0], buff_size);
        if (nread == 0 || nread == -1) 
            f = true;
        for (size_t j = 0; j < nread; ++j) {
            s += string{in[j]};
            if (in[j] == '\n') 
                f = true;
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
        write(1, "& ", 2);
        string command;
        //getline(cin, command);
        command = readline();
        size_t e = 0;
        int cnt = 0;

        int first_pipe[2];
        int second_pipe[2];
        if (command == "") 
            break;
        while (e != string::npos) {
            e = command.find('|');
            auto part = command.substr(0, e == string::npos ? command.length() : e);
            command = e == -1 ? "" : command.substr(e + 1);
            const auto strBegin = part.find_first_not_of(" ");
            const auto strEnd = part.find_last_not_of(" \n");
            const auto strRange = strEnd - strBegin + 1;
            part = part.substr(strBegin, strRange);
            size_t space = part.find(" ");
            string com = part.substr(0, space == -1 ? part.length() : space);
            string args = space == -1 ? "" : part.substr(space + 1, part.length());
           
            pipe(second_pipe);
            //cout << "created pipe: out " << second_pipe[0] << " in " << second_pipe[1] << "\n";

            pid_t child = fork();

            if (!child) { //child
                //cout << com << " reading from " << first_pipe[0] << ", write to " << second_pipe[1] << "\n";
                if (cnt) dup2(first_pipe[0], 0); // read from pipe
                if (cnt) close(first_pipe[0]);
                if (cnt) close(first_pipe[1]);
                if (e != -1) dup2(second_pipe[1], 1); // write to pipe
                close(second_pipe[0]);
                close(second_pipe[1]);
                execlp(com.c_str(), com.c_str(), args == "" ? NULL : args.c_str(), NULL);    
            } else {
                if (cnt) close(first_pipe[0]);
                if (cnt) close(first_pipe[1]);
                if (e == -1) close(second_pipe[0]);
                if (e == -1) close(second_pipe[1]);
            } 
        
            childs.push_back(child);
            first_pipe[0] = second_pipe[0];
            first_pipe[1] = second_pipe[1];

            cnt++;    
        }
    
        pid_t wpid;
        int status;
        while ((wpid = wait(&status)) > 0) { } 
        
    }

}
