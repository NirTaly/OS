#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <time.h>
#include <string>
#include <exception>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define PATH_MAX (200)

using std::string;
using std::vector;

class Command {
// TODO: Add your data members
protected:
  const char* cmd_line;
  char* args[COMMAND_MAX_ARGS];
  int args_size;
  int pid;
public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;//pure virtual

  const char* getCmd() const { return cmd_line; }
  char** getArgs() { return args; }
  int getPID() { return pid; }
  void setPID(int new_pid) { pid = new_pid; }
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line) : Command(cmd_line){}
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  char** prev_dir;
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
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
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
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
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
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

class JobsList;

class SmallShell {
public:
  char* prev_dir;
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
  int getPid() const;
  char** getPrevDir();
  JobsList* getJobList();
  
private:
  std::string prompt;
  int pid;
  JobsList* job_list;

  SmallShell();
};

enum JobState {RUNNING, STOP, DONE};

class JobsList {
public:
  class NotFound : std::exception {};

  class JobEntry {
  public:
    JobEntry(string cmd,size_t id, int pid) : uid(id), pid(pid),start_time(time(NULL)), cmd_line(cmd), state(RUNNING) {}
    ~JobEntry() = default;
    time_t getStartTime() const { return start_time; }
    string getCmd() const { return cmd_line; }
    size_t getUID() const { return uid; }
    JobState getState() const { return state; }
    int getPID() const { return pid==0 ? SmallShell::getInstance().getPid() : pid; }
  private:
    size_t uid;
    int pid;
    time_t start_time;
    string cmd_line;
    JobState state;
  };

  JobsList() : job_i(1) { }
  ~JobsList() = default;
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry& getJobById(size_t jobId);
  void removeJobById(size_t jobId);
  JobEntry& getLastJob();
  JobEntry& getLastStoppedJob();

private:
  vector<JobEntry> jobs;
  size_t job_i;
};

#endif //SMASH_COMMAND_H_