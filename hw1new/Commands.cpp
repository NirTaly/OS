#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
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

//-------------------SMASH IMPLEMENTATION----------------

SmallShell::SmallShell() {
// TODO: add your implementation
  prompt = "smash";
  pid = getpid();
}

SmallShell::~SmallShell() {
// TODO: add your implementation
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
//-------------------------------------------------------

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
  
  // For example:
/*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
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
    cout<<"cwd error"<<endl;
    //perror("getcwd() error");
    //return 1;
   }
}

/*************************************************************************************************/
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
    if (job.getState() == JobState::STOP)
    {
      std::cout << " (stopped)";
    }
    
    std::cout << std::endl;
  }
}