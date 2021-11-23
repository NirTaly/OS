#include <iostream>
#include <signal.h>

#include "signals.h"
#include "Commands.h"

using namespace std;

// void ctrlZHandler(int sig_num) {
// 	std::cout << "smash: got ctrl-Z" << std::endl;

//   SmallShell& smash = SmallShell::getInstance();
//   pid_t smash_pid = smash.getSmashPid(), fg_pid = smash.getFGJob().getPID();

//   if (smash_pid != fg_pid)
//   {
//     if (kill(fg_pid, SIGSTOP) == -1)
//       perror("smash error: kill failed");
    
//     smash.getJobList()->addJob(smash.getFGJob().getCmd())
//     smash.setFGPid(smash_pid);

//     std::cout << "smash: process " << fg_pid << " was killed" << std::endl;
//   }
  
// }

void ctrlCHandler(int sig_num) {
  std::cout << "smash: got ctrl-C" << std::endl;

  SmallShell& smash = SmallShell::getInstance();
  pid_t smash_pid = smash.getSmashPid(), fg_pid = smash.getFGJob().getPID();

  if (smash_pid != fg_pid)
  {
    if (kill(fg_pid, SIGKILL) == -1)
      perror("smash error: kill failed");
    
    if (kill(smash_pid, SIGCONT) == -1)
      perror("smash error: kill failed");
      
    smash.setFGJob(JobEntry());

    std::cout << "smash: process " << fg_pid << " was killed" << std::endl;
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

