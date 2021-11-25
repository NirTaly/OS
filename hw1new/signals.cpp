#include <iostream>
#include <signal.h>

#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	std::cout << "smash: got ctrl-Z" << std::endl;

  SmallShell& smash = SmallShell::getInstance();
  JobEntry fg_job = smash.getFGJob();

  if (fg_job.getPID() != smash.getSmashPid())
  {
    if (kill(fg_job.getPID(), SIGTSTP) == -1)
      perror("smash error: kill failed");
    
    smash.getJobList()->addJob(fg_job.getCmd(),fg_job.getPID(),fg_job.getStartTime(),STOP);
    smash.setFGJob(JobEntry());

    std::cout << "smash: process " << fg_job.getPID() << " was stopped" << std::endl;
  }
}


void ctrlCHandler(int sig_num) {
  std::cout << "smash: got ctrl-C" << std::endl;

  SmallShell& smash = SmallShell::getInstance();
  JobEntry fg_job = smash.getFGJob();

  pid_t smash_pid = smash.getSmashPid();

  if (fg_job.getPID() != smash_pid)
  {
    if (kill(fg_job.getPID(), SIGKILL) == -1)
      perror("smash error: kill failed");
    
    if (kill(smash_pid, SIGCONT) == -1)
      perror("smash error: kill failed");
      
    std::cout << "smash: process " << fg_job.getPID() << " was killed" << std::endl;
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

