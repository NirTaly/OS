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
    
    smash.getJobList()->addJob(fg_job.getCmd(),fg_job.getPID(),fg_job.getStartTime(),STOP,fg_job.getUID());
    smash.setFGJob(JobEntry());

    std::cout << "smash: process " << fg_job.getPID() << " was stopped" << std::endl;
  }
}

void ctrlCHandler(int sig_num){
  	std::cout << "smash: got ctrl-C" << std::endl;

	SmallShell& smash = SmallShell::getInstance();
	JobEntry fg_job = smash.getFGJob();

	if(fg_job.getPID() == smash.getSmashPid()){
		return;
	} 

	if(kill(fg_job.getPID(), SIGKILL) == -1){
		perror("smash error: kill failed");
		return;
	}
		
	std::cout << "smash: process " << fg_job.getPID() << " was killed" << std::endl;
	smash.setFGJob(JobEntry());
}

// void ctrlCHandler(int sig_num) {
//   std::cout << "smash: got ctrl-C" << std::endl;

//   SmallShell& smash = SmallShell::getInstance();
//   JobEntry fg_job = smash.getFGJob();

//   pid_t smash_pid = smash.getSmashPid();

//   if (fg_job.getPID() != smash_pid)
//   {
//     if (kill(fg_job.getPID(), SIGKILL) == -1)
//       perror("smash error: kill failed");
    
//     if (kill(smash_pid, SIGCONT) == -1)//why do we need this?
//       perror("smash error: kill failed");
      
//     std::cout << "smash: process " << fg_job.getPID() << " was killed" << std::endl;
//   }
//  //what about setting FGJob?
// }

void alarmHandler(int sig, siginfo_t *siginfo, void *context)
{
  std::cout << "smash got an alarm" << std::endl;

  pid_t send_alarm_pid = siginfo->si_pid;

  kill(send_alarm_pid, SIGKILL);

  SmallShell& smash = SmallShell::getInstance();
  try
  {
    JobEntry& send_job = smash.getJobList()->getJobByPID(send_alarm_pid);
    std::cout << "smash: " << send_job.getCmd() << " timed out!" << std::endl;
  }
  catch(const std::exception& e)
  {
    throw runtime_error("alarm: " + std::string(e.what()));
  }
}

// void alarmHandler(int sig_num) {
//   std::cout << "smash got an alarm" << std::endl;

// }

