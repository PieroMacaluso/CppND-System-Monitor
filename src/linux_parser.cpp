
#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>
using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key, value;
  float memTotal = 1, memFree = 1;
  // float memAvailable = 0, buffers = 0;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          memTotal = std::stod(value);
        } else if (key == "MemFree:") {
          memFree = std::stod(value);
        }
        // else if (key == "MemAvailable:") {
        //   memAvailable = std::stod(value);
        // } else if (key == "Buffers:") {
        //   buffers = std::stod(value);
        // }
        else {
          break;
        }
      }
    }
  }
  return (memTotal - memFree) / memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime;
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return std::stol(uptime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  long utime = 0;
  long stime = 0;
  long cutime = 0;
  long cstime = 0;
  string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream linestream(line);
    for (int i = 1; i <= 17; i++) {
      std::string valueStr;
      std::getline(linestream, valueStr, ' ');
      switch (i) {
        case 14:
          utime = std::stol(valueStr);
          break;
        case 15:
          stime = std::stol(valueStr);
          break;
        case 16:
          cutime = std::stol(valueStr);
          break;
        case 17:
          cstime = std::stol(valueStr);
          break;
      }
    }
  }
  return (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::vector<std::string> current_status = LinuxParser::CpuUtilization();
  long active = stol(current_status[LinuxParser::CPUStates::kUser_]) +
                stol(current_status[LinuxParser::CPUStates::kNice_]) +
                stol(current_status[LinuxParser::CPUStates::kSystem_]) +
                stol(current_status[LinuxParser::CPUStates::kIRQ_]) +
                stol(current_status[LinuxParser::CPUStates::kSoftIRQ_]) +
                stol(current_status[LinuxParser::CPUStates::kSteal_]);
  return active;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::vector<std::string> current_status = LinuxParser::CpuUtilization();
  long idle = std::stol(current_status[LinuxParser::CPUStates::kIdle_]) +
              std::stol(current_status[LinuxParser::CPUStates::kIOwait_]);
  return idle;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line;
  vector<string> values;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream.ignore(5, ' ');  // ignore the 'cpu' prefix
    string value;
    while (linestream >> value) {
      values.push_back(value);
    }
  }
  return values;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    return line;
  }
  return string("");
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value;
  long vmSize = 0;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          vmSize = std::stol(value);
        }
      }
    }
  }
  return std::to_string(vmSize / 1000);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid [[maybe_unused]]) {
  string line;
  string key;
  string uid;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> uid) {
        if (key == "Uid:") return uid;
      }
    }
  }
  return string("");
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line;
  string uid = LinuxParser::Uid(pid);
  string username;
  string sth;
  string uidSearch;

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> sth >> uidSearch) {
        if (uidSearch == uid) return username;
      }
    }
  }
  return string("");
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  std::string valueStr;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream linestream(line);
    for (int i = 1; i <= 22; i++) {
      std::getline(linestream, valueStr, ' ');
      if (i == 22) {
        return std::stol(valueStr) / sysconf(_SC_CLK_TCK);
        break;
      }
    }
  }
  try {
    return std::stol(valueStr) / sysconf(_SC_CLK_TCK);
  } catch (...) {
    return 0;
  }
}
