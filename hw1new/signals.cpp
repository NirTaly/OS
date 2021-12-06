#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void alarmHandler(int sig, siginfo_t *siginfo, void *context)
{
  std::cout << "smash: got an alarm" << std::endl;
  SmallShell& smash = SmallShell::getInstance();

  TO_PQ& pq =  smash.getPQ();
  bool is_timeout = false;
  to_node top_node;
  if(!pq.empty()){

    top_node = pq.top();
    is_timeout = (difftime(time(NULL),top_node.end_time) >= 0);
    
    if(!is_timeout){
      alarm(top_node.end_time - time(NULL));
    }
    else{
      pq.pop();
      if(!pq.empty()){
        to_node new_top_node = pq.top();
        alarm(new_top_node.end_time - time(NULL));
      }
    }
  }

  pid_t send_alarm_pid = is_timeout ? top_node.pid : siginfo->si_pid;

  if (is_timeout)
  {
    int status;
    int retval = waitpid(top_node.pid, &status, WNOHANG);

    if ((retval != 0))
      return;
  }

  if(smash.getSmashPid() != send_alarm_pid){
    if(kill(send_alarm_pid, SIGKILL) == -1){
        perror("smash error: kill failed");
		    return;
    }
  }

  try
  { 
    string sender_cmd;
    JobEntry fg_job = smash.getFGJob();

    if(fg_job.getPID() == send_alarm_pid){
      sender_cmd = is_timeout ? top_node.full_cmd : fg_job.getCmd();
    }
    else{
      JobEntry& send_job = smash.getJobList()->getJobByPID(send_alarm_pid);
      sender_cmd = send_job.getCmd();
    }
    std::cout << "smash: " << sender_cmd << " timed out!" << std::endl;
  }
  catch(const std::exception& e)
  {
    throw runtime_error("alarm: " + std::string(e.what()));
  }
}
