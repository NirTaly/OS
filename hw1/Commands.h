#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>

#include "factory.h"

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
public:
  Command(std::vector<std::string> cmd_line) : cmd_line(cmd_line) {}
  virtual ~Command() = default;
  virtual void execute() = 0;
  std::vector<std::string>& getCmd() { return cmd_line; }
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
private:
  std::vector<std::string> cmd_line;
};

class BuiltInCommand : public Command {
public:
  BuiltInCommand(std::vector<std::string> cmd_line) : Command(cmd_line) { }
  virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
public:
  ExternalCommand(std::vector<std::string> cmd_line) : Command(cmd_line) { }
  virtual ~ExternalCommand() = default;
  void execute() override {}
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(std::vector<std::string> cmd_line) : Command(cmd_line) { }
  virtual ~PipeCommand() = default;
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(std::vector<std::string> cmd_line) : Command(cmd_line) { }
  virtual ~RedirectionCommand() = default;
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
// TODO: Add your data members public:
  ChangeDirCommand(std::vector<std::string> cmd_line, std::string plastPwd) : BuiltInCommand(cmd_line),lastPwd(plastPwd) { }
  virtual ~ChangeDirCommand() = default;
  void execute() override;
private:
  std::string lastPwd;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
  GetCurrDirCommand(std::vector<std::string> cmd_line) : BuiltInCommand(cmd_line) { }
  virtual ~GetCurrDirCommand() = default;
  void execute() override; 
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(std::vector<std::string> cmd_line) : BuiltInCommand(cmd_line) { }
  virtual ~ShowPidCommand() = default;
  void execute() override;
};

// class JobsList {
//  public:
//   class JobEntry {
//    // TODO: Add your data members
//   };
//  // TODO: Add your data members
//  public:
//   JobsList();
//   ~JobsList();
//   void addJob(Command* cmd, bool isStopped = false);
//   void printJobsList();
//   void killAllJobs();
//   void removeFinishedJobs();
//   JobEntry * getJobById(int jobId);
//   void removeJobById(int jobId);
//   JobEntry * getLastJob(int* lastJobId);
//   JobEntry *getLastStoppedJob(int *jobId);
//   // TODO: Add extra methods or modify exisitng ones as needed
// };

// class QuitCommand : public BuiltInCommand {
// // TODO: Add your data members public:
//   QuitCommand(std::vector<std::string> cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) { }
//   virtual ~QuitCommand() = default;
//   void execute() override;
// };

// class JobsCommand : public BuiltInCommand {
//  // TODO: Add your data members
//  public:
//   JobsCommand(std::vector<std::string> cmd_line, JobsList* jobs);
//   virtual ~JobsCommand() = default;
//   void execute() override;
// };

// class KillCommand : public BuiltInCommand {
//  // TODO: Add your data members
//  public:
//   KillCommand(std::vector<std::string> cmd_line, JobsList* jobs);
//   virtual ~KillCommand() = default;
//   void execute() override;
// };

// class ForegroundCommand : public BuiltInCommand {
//  // TODO: Add your data members
//  public:
//   ForegroundCommand(std::vector<std::string> cmd_line, JobsList* jobs);
//   virtual ~ForegroundCommand() = default;
//   void execute() override;
// };

// class BackgroundCommand : public BuiltInCommand {
//  // TODO: Add your data members
//  public:
//   BackgroundCommand(std::vector<std::string> cmd_line, JobsList* jobs);
//   virtual ~BackgroundCommand() = default;
//   void execute() override;
// };

class HeadCommand : public BuiltInCommand {
public:
  HeadCommand(std::vector<std::string> cmd_line) : BuiltInCommand(cmd_line) { }
  virtual ~HeadCommand() = default;
  void execute() override {}
};

// option to add all commands to factory
struct Params
{
  std::string cmd_line;

};

class SmallShell {
public:
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell() = default;
  
  std::shared_ptr<Command> CreateCommand(std::vector<std::string> parsed_cmd);
  void executeCommand(std::string cmd_line);
  
  const std::string& getPrompt();
  void setPrompt(std::string new_prompt);

  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator

private:
  std::string prompt;
  Factory<Command,std::string,std::vector<std::string>> built_in_factory;

  SmallShell();
};

#endif //SMASH_COMMAND_H_
