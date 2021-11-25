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
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    while(smash.isAlive()) {
        std::cout << smash.getPrompt() <<"> ";
        std::string cmd_line;        
        
        std::getline(std::cin, cmd_line);
        try{
            smash.executeCommand(cmd_line.c_str());
        } catch(const std::exception& e)
        {
            std::cerr << "smash error: " << e.what() << '\n';
        }
    }

    return 0;
}


/***    BUGS:
 * %bug fg after ctrlZ
 * %bug job-id not kept when backgroundCommand->fg->ctrlZ
 * 
 * %bug pipe
 * 
 * %work: bg, fg, ctrlZ, ctrlC - regular cases
 * %work: bg after ctrlZ
 */
