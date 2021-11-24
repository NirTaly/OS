#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <algorithm>
#include "Commands.h"
#include <fcntl.h>//redirection

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

Command::Command(const char* cmd_line) : cmd_line(cmd_line), pid(0) {
  args_size = _parseCommandLine(cmd_line, args);
}

Command::~Command(){
  for(int i = 0; i < args_size ; i++){
    free(args[i]);
  }
}
/*****************************************************************************************************************/
//-------------------SMASH IMPLEMENTATION----------------

SmallShell::SmallShell() : prompt("smash"), pid(getpid()), job_list(new JobsList()), 
                      prev_dir(new char[PATH_MAX]), is_alive(true)
{
  strcpy(prev_dir,"");
}

SmallShell::~SmallShell() {
  delete job_list;
  delete[] prev_dir;
}

void SmallShell::setPrompt(std::string new_prompt){  prompt = new_prompt; }

const std::string& SmallShell::getPrompt() const {  return prompt; }

int SmallShell::getSmashPid() const {  return pid; }

JobEntry SmallShell::getFGJob() const { return fg_job; }

void SmallShell::setFGJob(JobEntry job) {  fg_job = job; }

void SmallShell::quit() { is_alive = false; }

bool SmallShell::isAlive() { return is_alive; }

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line) {
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  
  if(strstr(cmd_line, "|") != NULL){
	return new PipeCommand(cmd_line);  
  }
  else if(strstr(cmd_line, ">") != NULL) {//redirection command
    return new RedirectionCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
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
  else if (firstWord.compare("fg") == 0) {
    return new ForegroundCommand(cmd_line);
  }
  else if (firstWord.compare("bg") == 0) {
    return new BackgroundCommand(cmd_line);
  }
  else if (firstWord.compare("quit") == 0) {
    return new QuitCommand(cmd_line);
  }
  else {
    return new ExternalCommand(cmd_line);
  }
}

void SmallShell::executeCommand(const char* cmd_line) {

  Command* cmd = CreateCommand(cmd_line);
  try
  {
    job_list->removeFinishedJobs();
    cmd->execute();
  }
  catch(const std::exception& e)  
  { 
    delete cmd;
    throw std::runtime_error(e.what());
  }
  
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

RedirectionCommand::RedirectionCommand(const char* cmd_line) : Command(cmd_line), is_append(0) {
	full_str_cmd = cmd_line;
	const char* p = strstr(cmd_line, ">"); 
	int i = 0;
	while((cmd_line+i) != p){
		i++;
	}
	if(i != 0){
		left_cmd = full_str_cmd.substr(0,i);
	}
	if(strstr(cmd_line, ">>") != NULL){
		is_append = 1;
	}
	out_file = full_str_cmd.substr(i+1+is_append,full_str_cmd.size()-1-i-is_append);
	left_cmd = _trim(left_cmd);

	//ignore &
	for(int i = left_cmd.size()-1; i > 0; i--){
        if(left_cmd[i] == '&'){
            left_cmd[i] = ' ';
            break;
        }
    }

	out_file = _trim(out_file);
}

PipeCommand::PipeCommand(const char* cmd_line) : Command(cmd_line) , is_stderr_pipe(false){
	full_str_cmd = cmd_line;
	const char* p = strstr(cmd_line, "|"); 
	unsigned int i = 0;
	while((cmd_line+i) != p){
		i++;
	}
	if(i != 0){
		first_cmd = full_str_cmd.substr(0,i);
	}
	//ignore &
	if(i+1 < full_str_cmd.size() && full_str_cmd[i+1] == '&'){
		is_stderr_pipe = true;
		second_cmd = full_str_cmd.substr(i+2,full_str_cmd.size()-2-i);
	}
	else{
		second_cmd = full_str_cmd.substr(i+1,full_str_cmd.size()-1-i);
	}
	first_cmd = _trim(first_cmd);
	second_cmd = _trim(second_cmd);
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
  cout<<"smash pid is "<<smash.getSmashPid()<<endl;
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
    	cerr<<"cd: too many arguments"<<endl;
    	return;
    }
	else if(args_size == 1){
		return;
	}
    char curr_dir[PATH_MAX];
	char* tmp = getcwd(curr_dir, sizeof(curr_dir));
    if (tmp == NULL) {
		perror("getcwd failed");
		return;
    }

	if(strcmp(args[1],"-") == 0){
		if(strcmp(*prev_dir,"") == 0){
			cerr<<"cd: OLDPWD not set"<<endl;
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


void ExternalCommand::execute(){
	int pid = fork();
	if(pid < 0){
		perror("smash error: fork failed");
        return;
	}
	char clean_cmd_copy[COMMAND_ARGS_MAX_LENGTH];//just because passing cmd_line to helper functions doesnt work
	strcpy(clean_cmd_copy,cmd_line);
	bool bg_run = false;
	if(_isBackgroundComamnd(clean_cmd_copy)){
        bg_run = true;
		_removeBackgroundSign(clean_cmd_copy);
	}
	if(pid == 0){
		if(setpgrp() != -1){
			char* const execv_argv[4] = {(char*)"/bin/bash",(char*)"-c",clean_cmd_copy,nullptr}; 
			if(execv(execv_argv[0],execv_argv) == -1){
				perror("smash error: exec failed");
            	return;
			}
		}
		else{
      perror("smash error: setpgrp failed");
      return;
		}
	}
	else{
		if(bg_run == true){
			//parent do not need to wait
			JobsList* jlist = SmallShell::getInstance().getJobList();
			int tmp_pid = this->pid; 
			this->pid = pid;//to get son's pid and not the parent's
			jlist->addJob(this);	
			this->pid = tmp_pid;//restore shell pid
		}
		else{//parent(shell) need to wait
			int status;
			if(waitpid(pid,&status,WUNTRACED) == -1 ){//WUNTRACED make father stop waiting when the son was stopped
                perror("smash error: waitpid failed");
                return;
            }
		}
		return;
	}
}

void RedirectionCommand::execute(){
	/*CAN PROBABLY DELETE THIS PART

	string str_cmd = cmd_line;
	string left_cmd;
	string out_file;
	int is_append = 0;
	const char* p = strstr(cmd_line, ">"); 
	if(strstr(cmd_line, ">>") != NULL){
		is_append = 1;
	}
	int i = 0;
	while((cmd_line+i) != p){
		i++;
	}
	if(i != 0){
		left_cmd = str_cmd.substr(0,i);
	}
	out_file = str_cmd.substr(i+1+is_append,str_cmd.size()-1-i-is_append);
	left_cmd = _trim(left_cmd);

	//ignore &
	for(int i = left_cmd.size()-1; i > 0; i--){
        if(left_cmd[i] == '&'){
            left_cmd[i] = ' ';
            break;
        }
    }

	out_file = _trim(out_file);
	*/
	
	//we want to close stdout, change it to point to the file that the output has to go to,
	//create and execute the given command. but we want the restore fdt(1) to point to the screen
	//at the end of the process so we have to duplicate it.
	int fdt_screen_ptr = dup(1);
	if(fdt_screen_ptr == -1){
		perror("smash error: dup failed");
		return;
	}
	int fd;
	if(is_append == 0){
		fd = open(out_file.c_str(),(O_WRONLY| O_CREAT),0666); 
	}
	else{
		fd = open(out_file.c_str(),(O_WRONLY| O_CREAT | O_APPEND),0666);
	}
	if(fd == -1){
		perror("smash error: open failed");
		return;
	}

	if(dup2(fd,1)  == -1){//set fdt[1] to point to the opened file
		perror("smash error: dup2 failed");
		return;
	}
	SmallShell& smash = SmallShell::getInstance();
	smash.executeCommand(left_cmd.c_str());
	//smash.CreateCommand(left_cmd.c_str())->execute();
	//note: command may fail and still a file will be created, but it's the same in bash
	
	//reset stdout to point to the screen obj file
	if(dup2(fdt_screen_ptr,1)  == -1){
		perror("smash error: dup2 failed");
		return;
	}

	if(close(fd) == -1){
		perror("smash error: dup2 failed");
		return;
	}
	if(close(fdt_screen_ptr) == -1){
		perror("smash error: dup2 failed");
		return;
	}
}

void PipeCommand::execute(){
/**
*Idea:
*pipeline command format is command1 | command2
*1)smash create the pipeline
*2)smash uses fork
*3)smash(parent) run command1 and output it to the buffer
*4)child run command2 with input from the buffer
*/
	if(strstr(first_cmd.c_str(), "&") != NULL){
		cerr<<"pipe: invalid command"<<endl;//need to ask what to do in this case
  	}

	//questions:
	//how do i make sure that the reader read only after the writed finished?(parent is reader and waiting?)
	//for general knowledge, what happens if the reader read before the writer completely finished writing?
	int fd_stdin_save = dup(0);
	int fd_stdout_save = dup(1);
	int my_pipe[2];//0 for reading, 1 for writing from the pipe
	if(pipe(my_pipe) == -1){
		perror("smash error: pipe failed");
		return;
	}
	int pid = fork();
	if(pid < 0){
		perror("smash error: fork failed");
		return;
	}
	else if(pid == 0){//son(write)
		if(setpgrp() == -1){
			perror("smash error: setpgrp failed");
            return;
		}
		if(dup2(my_pipe[1],is_stderr_pipe ? 2 : 1) == -1){
			perror("smash error: dup2 failed");
            return;
		}
		if((close(my_pipe[0]) == -1) || (close(my_pipe[1]) == -1)){
			perror("smash error: close failed");
            return;	
		}
		SmallShell& smash = SmallShell::getInstance();
		smash.executeCommand(first_cmd.c_str());
		// return;
	}
	else{//parent(read)
		if(dup2(my_pipe[0],0) == -1){
			perror("smash error: dup2 failed");
            return;
		}
		if((close(my_pipe[0]) == -1) || (close(my_pipe[1]) == -1)){
			perror("smash error: close failed");
            return;	
		}
		if(waitpid(pid,nullptr,WUNTRACED) == -1){
			perror("smash error: close failed");
            return;
		}
		else{
			SmallShell& smash = SmallShell::getInstance();
			smash.executeCommand(second_cmd.c_str());
		}

	}
	//now we want to restore parent's(smash) fdt

	if(dup2(fd_stdin_save,0) == -1){
		perror("smash error: dup2 failed");
		return;
	}

	if(dup2(fd_stdout_save,1) == -1){
		perror("smash error: dup2 failed");
		return;
	}

	if(close(fd_stdin_save) == -1){
		perror("smash error: close failed");
		return;
	}

	if(close(fd_stdout_save) == -1){
		perror("smash error: close failed");
		return;
	}
}


/*****************************************************************************************************************/
//------------------JobList IMPLEMENTATION----------------
JobEntry::JobEntry() : uid(0), pid(getpid()) { }

void JobsList::addJob(Command* cmd, bool isStopped)
{
  string cmd_line(cmd->getCmd());
  int job_pid = cmd->getPID();
  JobState state = isStopped ? JobState::STOP : JobState::RUNNING;

  jobs.push_back(JobEntry(cmd_line, job_i++, job_pid,state));
}

void JobsList::printJobsList()
{
  for (const JobEntry& job : jobs)
  {
    std::cout << '[' << job.getUID() << "] " << job.getCmd() << " : " << job.getPID() << " " << difftime(time(NULL),job.getStartTime()) << " secs";
    if (job.getState() == JobState::STOP) // need to validate if vector stay sorted by id
    {
      std::cout << " (stopped)";
    }
    
    std::cout << std::endl;
  }
}

void JobsList::killAllJobs()
{
  std::cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << std::endl;
  for (const JobEntry& job : jobs)
  {
    std::cout << job.getPID() << ": " << job.getCmd() << std::endl;
    
    if (kill(job.getPID(), SIGKILL) == -1)
      perror("smash error: kill failed");
  }
}

void JobsList::removeFinishedJobs()
{
  for(unsigned int i = 0; i < jobs.size(); i++)
  { 
    pid_t retval = waitpid(jobs[i].getPID(), nullptr, WNOHANG);
    if( retval != 0 && retval != -1 )
    {
      jobs.erase(jobs.begin() + i);
      i--;  // because erase invalidate iterator of vector
    }
    else if (retval == -1)
    {
      perror("smash error: waitpid failed");
    }
  }
}

JobEntry& JobsList::getJobById(size_t jobId)
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
  for (size_t i = 0; i < jobs.size(); i++)
    if (jobs[i].getUID() == jobId)
    {
      jobs.erase(jobs.begin() + i);
      i--;  // because erase invalidate iterator of vector
    }
}

JobEntry& JobsList::getLastJob()
{
  if (jobs.empty())
  {
    throw Empty("jobs list is empty");
  }

  return jobs.back();
}

JobEntry& JobsList::getLastStoppedJob()
{
  auto it = std::find_if(jobs.rbegin(), jobs.rend(), [](JobEntry& job) { return job.getState() == JobState::STOP; });

  if (it == jobs.rend())
  {
    throw NotFound("does not exist");
  }
  else if (jobs.empty())
  {
    throw Empty("jobs list is empty");
  }

  return *it;
}
/*****************************************************************************************************************/
//-------------------Commands IMPLEMENTATION----------------

void JobsCommand::execute()
{
  JobsList* jlist = SmallShell::getInstance().getJobList();

  // jlist->removeFinishedJobs();
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
  
    JobEntry& job = jlist->getJobById(jobID);
    int job_pid = job.getPID();
    int retval = kill(job.getPID(),-signum);
    if (retval == -1 || signum < -32 || signum >= 0)
      perror("smash error: kill failed");

    jlist->removeJobById(jobID);
    
    std::cout << "signal number " << -signum << " was sent to pid " << job_pid << std::endl;
  }
  catch(const std::exception& e)
  {
    throw runtime_error(std::string("kill: ") + std::string(e.what()));
  }
}

enum JobType {FG, BG};
JobEntry& jobToExec(int args_size, char* args[COMMAND_MAX_ARGS], JobType job_type)
{
  JobsList* jlist = SmallShell::getInstance().getJobList();

  size_t jobID;

  if (args_size == 2)
  {
    try{
      jobID = std::stoull(std::string(args[1]));
    }catch(const std::exception&)    {
      throw invalid_argument("invalid arguments");
    }

    return jlist->getJobById(jobID);
  }
  else if (args_size == 1) 
  {
    if (JobType::FG == job_type)
    {
      return jlist->getLastJob();
    }
    else if (JobType::BG == job_type) // Bg Command
    {
      try {
        return jlist->getLastStoppedJob();
      } catch(const std::exception& e) {
        throw runtime_error("there is no stopped jobs to resume");
      }
    }
  }
  else
  {
    throw invalid_argument("invalid arguments");
  }

}

void ForegroundCommand::execute()
{
  try
  {
    JobsList* jlist = SmallShell::getInstance().getJobList();
    JobEntry& job = jobToExec(args_size, args, JobType::FG);
    
    std::cout << job.getCmd() << " : " << job.getPID() << " " << std::endl;
    jlist->removeJobById(job.getUID());

    if (kill(job.getPID(),SIGCONT) == -1)
      perror("smash error: kill failed");

    
    if (waitpid(job.getPID(), nullptr, WCONTINUED) == -1)
      perror("smash error: waitpid failed");

  }
  catch(const std::exception& e)
  {
    throw runtime_error(std::string("fg: ") + std::string(e.what()));
  }
}

void BackgroundCommand::execute()
{
  try
  {
    SmallShell& smash = SmallShell::getInstance();

    JobsList* jlist = smash.getJobList();
    JobEntry& job = jobToExec(args_size, args, JobType::BG);
    
    if (job.getState() == JobState::RUNNING)
    {
      throw runtime_error("job-id " + std::string(to_string(job.getUID())) + " is already running in the background");
    }

    std::string old_cmd = job.getCmd();
    std::cout << old_cmd << " : " << job.getPID() << " " << std::endl;
    jlist->removeJobById(job.getUID());

    // from now on we want to destroy previous job and call the command again, just now
    // as background cmd
    if (kill(job.getPID(),SIGKILL) == -1)
    {
      perror("smash error: kill failed");
    }

    std::string new_bg_cmd = old_cmd;
    bool is_fg_cmd = false;
    if (!_isBackgroundComamnd(old_cmd.c_str()))
    {
      new_bg_cmd = old_cmd + "&";
      is_fg_cmd = true;
    }

    smash.executeCommand(new_bg_cmd.c_str()); // not good enough, we need specific external command
    // set job name to the first (without '&')
    if (is_fg_cmd)
    {
      JobEntry& new_job = jlist->getJobById(jlist->getLastJobIndex());
      new_job.setCmd(old_cmd); // set jobs name to the one without
    }
  }
  catch(const std::exception& e)
  {
    throw runtime_error(std::string("bg: ") + std::string(e.what()));
  }
}

void QuitCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    JobsList* jlist = smash.getJobList();

    jlist->removeFinishedJobs();
    if (args_size >= 2 && std::string(args[1]) == "kill")
    {
      jlist->killAllJobs();
    }

    smash.quit();
}