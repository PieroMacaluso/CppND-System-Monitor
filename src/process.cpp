#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {
  user_ = LinuxParser::User(pid_);
  command_ = LinuxParser::Command(pid_);
  ram_ = LinuxParser::Ram(pid_);
  uptime_ = LinuxParser::UpTime(pid_);

  long processUptime = LinuxParser::UpTime() - uptime_;
  long activeTime = LinuxParser::ActiveJiffies(pid_);
  try {
    cpuUtilization_ = float(activeTime) / float(processUptime);
  } catch (...) {
    cpuUtilization_ = 0.0;
  }
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() { return cpuUtilization_; }

// Return the command that generated this process
string Process::Command() { return command_; }

// Return this process's memory utilization
string Process::Ram() { return ram_; }

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return uptime_; }

// put CPU
// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return this->cpuUtilization_ > a.cpuUtilization_;
}