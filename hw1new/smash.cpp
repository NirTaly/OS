#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "Commands.h"
#include "signals.h"


int main(int argc, char* argv[]) {
    if (signal(SIGTSTP , ctrlZHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if (signal(SIGINT , ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    struct sigaction new_act;
    bzero(&new_act, sizeof(struct sigaction));
    //
    // alarm(15);
    //

    new_act.sa_sigaction = alarmHandler;
    new_act.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGALRM, &new_act, NULL) != 0) 
    {
        printf("error sigaction()");
        return errno;
    }
    SmallShell& smash = SmallShell::getInstance();
    while(smash.isAlive()) {
        std::cout<< smash.getPrompt() <<"> ";
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
 * %bug sometimes make test failes -> where should put removeAllJobs
 * 
 * %check:
 *      sleep 100& , bg -> print there is no stopped jobs to resume
 * 
 * 
 * 
 * %tests:
 * [V] cd
 * [V] chprompt
 * [V] jobs
 * [V] quit
 * [V] showpid
 * [V] kill
 * [V] pipe

 * [X] redirect
 * 
 * [] timeout
 */
