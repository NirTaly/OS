#include <iostream>
#include <signal.h>

#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	std::cout << "smash: got ctrl-Z" << std::endl;

  SmallShell& smash = SmallShell::getInstance();
  Command* fg_cmd = smash.getFGJob();

  if (fg_cmd)
  {
    if (kill(fg_cmd->getPID(), SIGSTOP) == -1)
      perror("smash error: kill failed");
    
    smash.getJobList()->addJob(fg_cmd,STOP);
    smash.setFGJob(nullptr);

    std::cout << "smash: process " << fg_cmd->getPID() << " was stopped" << std::endl;
  }
  
}
#define PRINT_DEBUG(X) {std::cout << X << std::endl;}

void ctrlCHandler(int sig_num) {
  std::cout << "smash: got ctrl-C" << std::endl;

  SmallShell& smash = SmallShell::getInstance();
  Command* fg_cmd = smash.getFGJob();

  pid_t smash_pid = smash.getSmashPid();

  if (fg_cmd)
  {
    if (kill(fg_cmd->getPID(), SIGKILL) == -1)
      perror("smash error: kill failed");
    
    if (kill(smash_pid, SIGCONT) == -1)
      perror("smash error: kill failed");
      
    std::cout << "smash: process " << fg_cmd->getPID() << " was killed" << std::endl;
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

