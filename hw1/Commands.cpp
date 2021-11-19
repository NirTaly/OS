#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <functional>

#include "Commands.h"

using namespace std;

#define WHITESPACE (" \t\n\r\f\v")

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

int _parseCommandLine(const char* cmd_line, vector<std::string>& args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; i++) {
    args.push_back(_trim(s));
    // args[i] = (char*)malloc(s.length()+1);
    // memset(args[i], 0, s.length()+1);
    // strcpy(args[i], s.c_str());
    // args[i] = s;
    // args[++i] = NULL;
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

/****************************************************************************/
/****************************************************************************/
/*                        IMPLEMENTATION                                    */

SmallShell::SmallShell() {
  prompt = "smash";
  built_in_factory.Add("pwd", [](vector<std::string> cmd_line) { return std::shared_ptr<Command>( new GetCurrDirCommand(cmd_line));});
  built_in_factory.Add("showpid", [](vector<std::string> cmd_line) { return std::shared_ptr<Command>( new ShowPidCommand(cmd_line));});
  built_in_factory.Add("head", [](vector<std::string> cmd_line) { return std::shared_ptr<Command>( new HeadCommand(cmd_line));});
}

const std::string& SmallShell::getPrompt()
{
  return prompt;
}

void SmallShell::setPrompt(std::string new_prompt)
{
  prompt = new_prompt;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
std::shared_ptr<Command> SmallShell::CreateCommand(vector<std::string> parsed_cmd) {
	// For example:

  // string cmd_s = _trim(string(cmd_line));
  // string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  std::shared_ptr<Command> cmp_obj;
  string cmd = parsed_cmd[0];

  try
  {
    return built_in_factory.Create(cmd, parsed_cmd);
  }
  catch(const BadKey& e)
  {
    if (cmd.compare("chprompt") == 0){
      if (parsed_cmd.size() == 1) {
        setPrompt("smash");
      }
      else if (parsed_cmd.size() >= 2){
        setPrompt(parsed_cmd[1]);
      }
    }
    // return make_shared<Command>(new ExternalCommand(parsed_cmd));
    return std::shared_ptr<Command>( new ExternalCommand(parsed_cmd));
  }
  
  // if (firstWord.compare("pwd") == 0) {
  //   return new GetCurrDirCommand(cmd_line);
  // }
  // else if (firstWord.compare("showpid") == 0) {
  //   return new ShowPidCommand(cmd_line);
  // }
  // else if (firstWord.compare("chprompt") == 0) {
  // }
  // else {
  //   return new ExternalCommand(cmd_line);
  // }

  return nullptr;
}

void SmallShell::executeCommand(std::string cmd_line) {
  vector<std::string> parsed_cmd;
  _parseCommandLine(cmd_line.c_str(), parsed_cmd);
  shared_ptr<Command> cmd = CreateCommand(parsed_cmd);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

/************************************************************************************************************************/

void GetCurrDirCommand::execute()
{
  char buf[COMMAND_ARGS_MAX_LENGTH] = { 0 };
  std::cout << getcwd(buf,COMMAND_ARGS_MAX_LENGTH) << std::endl; 
}

/************************************************************************************************************************/

void ShowPidCommand::execute()
{
  std::cout << getpid() << std::endl;
}