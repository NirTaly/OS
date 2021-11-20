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

Command::Command(const char* cmd_line) : cmd_line(cmd_line){
  args_size = _parseCommandLine(cmd_line, args);
}

//-------------------SMASH IMPLEMENTATION----------------

SmallShell::SmallShell() {
// TODO: add your implementation
  prompt = "smash";
  pid = getpid();
  prev_dir = new char[PATH_MAX];
  strcpy(prev_dir, "");
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

char** SmallShell::getPrevDir(){
  return &prev_dir;
}
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
  else if (firstWord.compare("cd") == 0) {
    SmallShell& smash = SmallShell::getInstance();
    char** prev_dir = smash.getPrevDir();
    return new ChangeDirCommand(cmd_line, prev_dir);
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
		perror("smash error: getcwd failed");
      	return;
   }
}

ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd) : BuiltInCommand(cmd_line), prev_dir(plastPwd){}

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