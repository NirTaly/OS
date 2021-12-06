#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <queue>
#include <time.h>
#include <string>
#include <exception>
#include <limits.h> //PATH_MAX
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include "signals.h"

#define PRINT_DEBUG(X) {std::cout << X << std::endl;}

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using std::string;
using std::vector;

class Command {
protected:
  const char* cmd_line;
  char* args[COMMAND_MAX_ARGS];
  int args_size;
  pid_t pid;
  bool isBuiltin;
public:
  Command(const char* cmd_line, bool isBuiltin = false );
  virtual ~Command();
  virtual void execute() = 0;//pure virtual

  const char* getCmd() const { return cmd_line; }
  char** getArgs() { return args; }
  int getPID() { return pid; }
  void setPID(int new_pid) { pid = new_pid; }
};

class BuiltInCommand : public Command {
public:
  BuiltInCommand(const char* cmd_line) : Command(cmd_line,true) {}
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
  bool is_to_cmd;
public:
  ExternalCommand(const char* cmd_line, bool is_to_cmd = false) : Command(cmd_line), is_to_cmd(is_to_cmd) { }
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
private:  
  static const int PIPE_READ = 0;
  static const int PIPE_WRITE = 1;
  string full_str_cmd;
  string first_cmd;
  string second_cmd;
  bool is_stderr_pipe;
public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
private:
  string full_str_cmd;
	string left_cmd;
	string out_file;
	int is_append;
public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
  char** prev_dir;
  ChangeDirCommand(const char* cmd_line, char** plastPwd) : BuiltInCommand(cmd_line), prev_dir(plastPwd){}
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
  GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
  ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
public:
  QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
  virtual ~QuitCommand() {}
  void execute() override;
};

class ChpromptCommand : public BuiltInCommand {
public:
  ChpromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
  virtual ~ChpromptCommand() {};
  void execute() override;
};

class JobsCommand : public BuiltInCommand {
public:
  JobsCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
  KillCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
public:
  ForegroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
public:
  BackgroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class HeadCommand : public BuiltInCommand {
public:
  HeadCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~HeadCommand() {}
  void execute() override;
};

class TimeOutCommand : public BuiltInCommand {
   int duration;
   std::string to_cmd; 
public:
  TimeOutCommand(const char* cmd_line);
  virtual ~TimeOutCommand() {}
  void execute() override;
};

class JobsList;

enum JobState {RUNNING, STOP};
class JobEntry {
public:
  JobEntry(string cmd="smash",int id=0, pid_t pid=getpid(), time_t start_time = time(NULL), JobState state = RUNNING) 
          : uid(id), pid(pid),start_time(start_time), cmd_line(cmd), state(state) {}
  ~JobEntry() = default;
  time_t getStartTime() const { return start_time; }
  string getCmd() const { return cmd_line; }
  void setCmd(std::string new_cmd) { cmd_line = new_cmd; }
  int getUID() const { return uid; }
  JobState getState() const { return state; }
  void setState(JobState new_state) { state = new_state; }
  pid_t getPID() const { return pid; }
private:
  int uid;
  pid_t pid;
  time_t start_time;
  string cmd_line;
  JobState state;
};

struct to_node{
  time_t timestamp;
  time_t duration;
  time_t end_time;
  pid_t pid;
  std::string full_cmd;
  to_node() = default;
  to_node(int timestamp, int duration, int pid, std::string full_cmd) : 
    timestamp(timestamp), duration(duration), end_time(duration+timestamp), pid(pid), full_cmd(full_cmd){}
  bool operator==(const to_node& other) const{
    return (pid == other.pid);
  }
  bool operator<(const to_node& other) const{
    return (end_time < other.end_time);
  }
};

class Compare{
public:
    bool operator() (const to_node& n1, const to_node& n2){
        return n2 < n1;
    }
};

using TO_PQ = std::priority_queue<to_node,std::vector<to_node>,Compare>;
class SmallShell {
public:
  Command* CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
  void setPrompt(std::string new_prompt);
  const std::string& getPrompt() const;
  pid_t getSmashPid() const;
  JobEntry getFGJob() const;
  void setFGJob(JobEntry);
  JobsList* getJobList();
  char** getPrevDir();
  void quit();
  bool isAlive();
  TO_PQ& getPQ();

private:
  std::string prompt;
  pid_t pid;
  JobsList* job_list;
  char* prev_dir;
  bool is_alive;
  JobEntry fg_job;
  std::priority_queue<to_node,std::vector<to_node>,Compare> pq;
  SmallShell();
};


class JobsList {
public:
  class Empty : public std::exception
  {
  public:
    Empty(std::string what): str(what) {}
    virtual const char* what() const throw() {return str.c_str(); }
  private:
    std::string str;
  };

  class NotFound : public std::exception
  {
  public:
    NotFound(std::string what): str(what) {}
    virtual const char* what() const throw() {return str.c_str(); }
  private:
    std::string str;
  };

  JobsList() : job_i(1) { }
  ~JobsList() = default;
  void addJob(Command* cmd, bool isStopped = false);
  void addJob(string cmd_line, pid_t pid, time_t start_time = time(NULL), bool isStopped = false, int jobID = 0);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry& getJobById(int jobId);
  JobEntry& getJobByPID(pid_t pid);
  void removeJobById(int jobId);
  JobEntry& getLastJob();
  JobEntry& getLastStoppedJob();
  int getLastJobIndex() const { return job_i-1; }
private:
  vector<JobEntry> jobs;
  int job_i;
};

#endif //SMASH_COMMAND_H_
