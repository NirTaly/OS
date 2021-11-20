#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <algorithm>
#include "Commands.h"

const std::string WHITESPACE = " \n\r\t\f\v";//from piazza

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

Command::Command(const char* cmd_line) : cmd_line(cmd_line), pid(0) {
  args_size = _parseCommandLine(cmd_line, args);
}
/*****************************************************************************************************************/
//-------------------SMASH IMPLEMENTATION----------------

SmallShell::SmallShell() : prompt("smash"), pid(getpid()), job_list(new JobsList()), prev_dir(new char[PATH_MAX]) {
  strcpy(prev_dir,"");
}

SmallShell::~SmallShell() {
  delete job_list;
  delete[] prev_dir;
}

void SmallShell::setPrompt(std::string new_prompt){
  prompt = new_prompt;
}

const std::string& SmallShell::getPrompt() const{
  return prompt;
}

int SmallShell::getPid() const{
  return pid;
};

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("chprompt") == 0) {
    return new ChpromptCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
    return new ChangeDirCommand(cmd_line,&prev_dir);
  }
  else if (firstWord.compare("jobs") == 0) {
    return new JobsCommand(cmd_line);
  }
  else if (firstWord.compare("kill") == 0) {
    return new KillCommand(cmd_line);
  }
  else {
    return new ExternalCommand(cmd_line);
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char* cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  delete cmd;
}

JobsList* SmallShell::getJobList()
{
 return job_list;
}

char** SmallShell::getPrevDir()
{
  return &prev_dir;
}
/*****************************************************************************************************************/
//-------------------Commands IMPLEMENTATION----------------
Command::~Command(){
  for(int i = 0; i < args_size ; i++){
    free(args[i]);
  }
}

void ChpromptCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  if(args_size > 1){
    smash.setPrompt(args[1]);
  }
  else{
    smash.setPrompt("smash");
  }
}

void ShowPidCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  cout<<"smash pid is "<<smash.getPid()<<endl;
}

void GetCurrDirCommand::execute(){
  char cwd_buff[PATH_MAX];
  if (getcwd(cwd_buff, sizeof(cwd_buff)) != NULL) {
    cout<<cwd_buff<<endl;
  }
  else{
    perror("smash error: getcwd failed");
   }
}

void ChangeDirCommand::execute(){
    if(args_size > 2){
    	cerr<<"smash error: cd: too many arguments"<<endl;
    	return;
    }
    char curr_dir[PATH_MAX];
	char* tmp = getcwd(curr_dir, sizeof(curr_dir));
    if (tmp == NULL) {
		perror("smash error: getcwd failed");
		return;
    }

	if(strcmp(args[1],"-") == 0){
		if(strcmp(*prev_dir,"") == 0){
			cerr<<"smash error: cd: OLDPWD not set"<<endl;
            return;
		}
		else if(chdir(*prev_dir) == -1){
			perror("smash error: chdir failed");
  			return;
		}
	}
    else if(chdir(args[1]) == -1){ // args[1] != "-"
		perror("smash error: chdir failed");
  		return;
    }
	strcpy(*prev_dir,curr_dir);
}

/*****************************************************************************************************************/
//-------------------JobList IMPLEMENTATION----------------
void JobsList::addJob(Command* cmd, bool isStopped)
{
  string cmd_line(cmd->getCmd());
  int job_pid = cmd->getPID();

  jobs.push_back(JobEntry(cmd_line, job_i++, job_pid));
}

void JobsList::printJobsList()
{
  for (const JobEntry& job : jobs)
  {
    std::cout << '[' << job.getUID() << "] " << job.getCmd() << " : " << job.getPID() << " " << difftime(time(NULL),job.getStartTime());
    if (job.getState() == JobState::STOP) // need to validate if vector stay sorted by id
    {
      std::cout << " (stopped)";
    }
    
    std::cout << std::endl;
  }
}

void JobsList::killAllJobs()
{
  for (const JobEntry& job : jobs)
  {
    kill(job.getPID(), SIGKILL);
  }
}

void JobsList::removeFinishedJobs()
{
  std::remove_if(jobs.begin(), jobs.end(), [](JobEntry& job) { return job.getState() == JobState::DONE; });
}

JobsList::JobEntry& JobsList::getJobById(size_t jobId)
{
  auto it = std::find_if(jobs.begin(), jobs.end(), [jobId](JobEntry const& job) { return job.getUID() == jobId; });
  if (it == jobs.end())
  {
    throw NotFound("job-id " + std::string(to_string(jobId)) + " does not exist");
  }

  return *it;
}

void JobsList::removeJobById(size_t jobId)
{
  std::remove_if(jobs.begin(), jobs.end(), [jobId](JobEntry& job) { return job.getUID() == jobId; });
}

JobsList::JobEntry& JobsList::getLastJob()
{
  if (jobs.empty())
  {
    throw Empty();
  }

  return jobs.back();
}

JobsList::JobEntry& JobsList::getLastStoppedJob()
{
  auto it = std::find_if(jobs.begin(), jobs.end(), [](JobEntry& job) { return job.getState() == JobState::STOP; });

  if (it == jobs.end())
  {
    throw NotFound("does not exist");
  }
  else if (jobs.empty())
  {
    throw Empty();
  }

  return *it;
}
/*****************************************************************************************************************/
//-------------------Commands IMPLEMENTATION----------------

void JobsCommand::execute()
{
  JobsList* jlist = SmallShell::getInstance().getJobList();

  jlist->removeFinishedJobs();
  jlist->printJobsList();
}

void KillCommand::execute()
{
  try
  {
    if (args_size != 3 || (args_size == 3 && *(args[1]) != '-'))
    {
      throw invalid_argument("invalid arguments");
    }

    int signum;
    size_t jobID;

    try{
      signum = std::stoi(std::string(args[1]));
      jobID = std::stoull(std::string(args[2]));
    }catch(const std::exception&)
    {
      throw invalid_argument("invalid arguments");
    }
    
    JobsList* jlist = SmallShell::getInstance().getJobList();
  
    JobsList::JobEntry& job = jlist->getJobById(jobID);
    int retval = kill(job.getPID(),-signum);
    if (retval == EINVAL || retval == EPERM || retval == ESRCH || retval <= 0 || signum < -32 || signum >= 0)
      perror("smash error: kill failed");
  }
  catch(const std::exception& e)
  {
    /****************************/
    std::cout << "smash error: kill: " << e.what() << std::endl;
  }
}