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
    
    struct sigaction sig_act;
    bzero(&sig_act, sizeof(struct sigaction));

    sig_act.sa_sigaction = *alarmHandler;
    sig_act.sa_flags |= SA_SIGINFO;

    if (sigaction(SIGALRM, &sig_act, NULL) != 0) 
    {
        printf("error sigaction()");
        return errno;
    }

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
 * %bug sometimes make test failes -> where should put removeAllJobs
 * 
 * %check:
 *      sleep 100& , bg -> print there is no stopped jobs to resume
 */
