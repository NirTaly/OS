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
<<<<<<< Updated upstream
=======
    //
    // alarm(15);
    //
>>>>>>> Stashed changes

    new_act.sa_sigaction = alarmHandler;
    new_act.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGALRM, &new_act, NULL) != 0) 
    {
        printf("error sigaction()");
        return errno;
    }
    SmallShell& smash = SmallShell::getInstance();
    while(smash.isAlive()) {

        //dell
        
        // // test SIGALRM handler
        // JobsList* jlist = smash.getJobList();
        // try{
        //     JobEntry je = jlist->getLastJob();
        //     int jid = je.getPID();
        //     std::cout<<"job id is: "<<jid<<std::endl;
        //     if(jid != smash.getSmashPid()){
        //         kill(jid,SIGALRM);
        //     }
        // }catch(const std::exception& e){
        //     std::cerr << "smash error: " << e.what() << '\n';
        // }
        // 
        // int pid = fork();
        // if(pid == 0){
        //     sleep(5);
        //     kill(getppid(),SIGALRM);
        // }
        // else{
        //     std::cout<<"child pid: "<<pid<<std::endl;
        //     std::cout<<"parent pid: "<<getpid()<<std::endl;
        //     waitpid(pid,nullptr,WUNTRACED);
        // }
        // else{
        //     std::cout<<"child's pid is "<< pid<<std::endl;
        //     wait(NULL);
        // }
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
 */
