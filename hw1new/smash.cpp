#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"


int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    // if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
    //     perror("smash error: failed to set ctrl-C handler");
    // }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    int i = 0;
    while(true) {
        std::cout << smash.getPrompt() <<"> ";
        std::string cmd_line;        
        //my tests
        if(i > 3){
            break;
        }
        i++;
        //end of tests
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    
    return 0;
}