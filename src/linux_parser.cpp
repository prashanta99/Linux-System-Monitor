#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
//#include <experimental/filesystem>
#include "linux_parser.h"

//namespace fs = std::experimental::filesystem::v1 ;

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::cout ;
using std::stol ;

// read name of the operating system
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

// Read name of the Kernel and Version
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

// Read all processes directories in /proc
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
  string line = "" ;
  string key ;
  int value;
  float perc_utilization = 0.0 ;
  float total_memory = 0.0 ;
  float avail_memory = 0.0 ;
  int counter = 0 ;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);

  if (filestream.is_open()) {
    while ((std::getline(filestream, line)) && counter < 2) {
      std::replace(line.begin(), line.end(), ':', ' ') ;
      std::istringstream linestream(line) ;
      linestream >> key >> value  ;
      if (key == "MemTotal") {
        total_memory = value ;
        counter += 1 ;
      }
      else if (key == "MemFree") {
        avail_memory = value ;
        counter += 1 ;
      }
    }
  }
  filestream.close();
  if (total_memory > 0) {
      perc_utilization = ((total_memory - avail_memory) / total_memory) ;
  }
  else {
    cout << "From LinuxParse.cpp: MemoryUtilization - Total_memory is 0 or " << total_memory << "\n";
  }
  return perc_utilization; 
  }


// Read and return the system uptime
long LinuxParser::UpTime() { 
  string line;
  long secondsUptime = 0;
  long cpuuptime = 0;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line) ;
    linestream >> secondsUptime >> cpuuptime ;
  }
  filestream.close() ;
  return secondsUptime; 
}

// Read and return the system jiffies
// Jiffies = usr + nice + kernel + idle + iowait + irq + softrq + steal ;
long LinuxParser::Jiffies() { 
  return ProcessProcStatData(0) ;    
}

long LinuxParser::ActiveJiffies(int pid) { 
  return ReadPidStatFile(pid, "PidActiveJiffies") ; 
}

//Read and return the number of active jiffies for the system
// ActiveJiffies= usr + nice + kernel + irq + softrq + steal ;
long LinuxParser::ActiveJiffies() { 
  return ProcessProcStatData(1) ;   
}

//Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  return ProcessProcStatData(2) ;   
}

//Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
    vector<string> cpurecord ;
    ReadProcStatFile(cpurecord); 
    return cpurecord ;
}

//Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
    return  int(ProcessProcStatData(3)) ;  
}

//Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  return  int(ProcessProcStatData(4)) ; 
}

//Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line ;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename) ;
  if (filestream.is_open()) {
      std::getline(filestream, line) ;
  }
  filestream.close() ;
  return line; 
}

//Read and return the memory used by a process
// Memory is calculated in Mb
string LinuxParser::Ram(int pid) { 
  long ram ;
  ram =  ReadPidStatusFile(pid, "VmSize")/1024 ;
  return to_string(ram) ;
}

//Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string uid ;
  uid = to_string(ReadPidStatusFile(pid, "Uid")) ;
  return uid ;
}

//Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
    long pidStartTime = ReadPidStatFile(pid, "StartTime")/sysconf(_SC_CLK_TCK) ;
    long systemUpTime = UpTime() ;
    long pidUpTime = systemUpTime - pidStartTime ;
    return pidUpTime ;
}


//Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  long uid = ReadPidStatusFile(pid, "Uid") ;
  string line ;
  string key = "*" ;
  long value = -1 ;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
          std::replace(line.begin(), line.end(), ':',' ') ;
          std::istringstream linestream(line) ;
          linestream >> key >> value ;
          if (value == uid) {
            break ;
          }
      }     
  }
  filestream.close() ;
  return key ;
}



// Calculate CPU Utilization of a process
float LinuxParser::CalculateCpuUtilization (int pid) {
    float cpuUsage = 0;
    //long  systemUptime = UpTime();
    float totalCpu = ActiveJiffies(pid)/sysconf(_SC_CLK_TCK);
    float startTimeSeconds = UpTime(pid) ;
    //float netCpu = systemUptime - startTimeSeconds ;
    if (startTimeSeconds == 0) {
        cpuUsage = 0 ;
    }
    else {
        cpuUsage = totalCpu / startTimeSeconds ;
    }
    return cpuUsage ;
} 

// Helper function to collect stats in vector of strings
void LinuxParser::ReadProcStatFile(vector<string> &statRecord) {
  string key;
  string value ;
  string line ;
  std::ifstream filestream(kProcDirectory + kStatFilename) ;
  int recordCounter = 0 ;
  if (filestream.is_open()) {
    while(std::getline(filestream, line) && recordCounter < 3) {
        std::istringstream linestream(line) ;
        linestream >> key ;
        if (key == "cpu") {
            while(linestream >> value) {
              statRecord.emplace_back(value) ; 
            }
            recordCounter += 1;
        }
        if (key == "processes") {
          linestream >> value ;
          statRecord.emplace_back(value) ;
          recordCounter += 1;
        }
        if (key == "procs_running") {
          linestream >> value ;
          statRecord.emplace_back(value) ;
          recordCounter += 1;
         }
    }
  }
  filestream.close() ;
} 

/*
LinuxParser::ProcessStatsFile --> Helper Function to Process /proc/stat file 
returnflag is used as below
0 ->  total_jiffies, 
1 ->  Total Active Jiffies, 
2 ->  Idle Jiffies, 
3 ->  Number of processes and 
4 ->  Number of procs_running 
*/
long LinuxParser::ProcessProcStatData(int returnflag) {
  vector<string> cpustat ;
  ReadProcStatFile(cpustat) ;
  long usr, nice, system, idle, iowait, irq, softirq, steal, processes, procs_running ;
  long total_jiffies = 0 ;
  long total_active_jiffies = 0;
  long idle_jiffies = 0 ;
  // guest and guest_nice are already counted in usr and nice thats why they is excluded
  usr = stol(cpustat[LinuxParser::kUser_]) ;
  nice = stol(cpustat[LinuxParser::kNice_]) ;
  system = stol(cpustat[LinuxParser::kSystem_]) ;
  idle = stol(cpustat[LinuxParser::kIdle_]) ;
  iowait = stol(cpustat[LinuxParser::kIOwait_]) ;
  irq = stol(cpustat[LinuxParser::kIRQ_]) ;
  softirq = stol(cpustat[LinuxParser::kSoftIRQ_]) ;
  steal = stol(cpustat[LinuxParser::kSteal_]) ;
  processes = stol(cpustat[LinuxParser::kProcesses_]);  
  procs_running = stol(cpustat[LinuxParser::kProcsRunning_]);
    
    switch(returnflag) {
      case 0:
            total_jiffies = usr + nice + system + idle + iowait + irq + softirq + steal ;
            return total_jiffies ;
      case 1:
            total_active_jiffies = usr + nice + system + irq + softirq + steal ;
            return total_active_jiffies ;
      case 2:
            idle_jiffies = idle + iowait ;
            return idle_jiffies;
      case 3:
            return processes ;
      case 4:
            return procs_running ; 
    } 
    return -1 ;
} 

/*
ReadPidStatsFile --> Helper Function to Read /proc/pid/stat file
returnflag = "PidActiveJiffies" : returns Total Active jiffies of the PID from proc/<pid>/stat file
returnflag = "StartTime" : returns starttime of the PID from /proc/<pid>/stat file 
*/
long LinuxParser::ReadPidStatFile(int pid, string returnflag) {
  string line ;
  string value ;
  long pid_activejiffies = 0 ;
  vector<string> statRecord {} ;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename) ;
  if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
          std::istringstream linestream(line) ;
          while (linestream >> value) {
              statRecord.emplace_back(value) ;
          }
      }
  }
  if (returnflag == "PidActiveJiffies") {
      pid_activejiffies = stol(statRecord[kUtime_]) + stol(statRecord[kStime_]) + stol(statRecord[kCUtime_]) + stol(statRecord[kCStime_]) ;
      return pid_activejiffies ;
  }
  if (returnflag == "StartTime") {
      return std::stol(statRecord[kStarttime_]) ;
  }
  return -1 ;
} 

/*
ReadPidStatusFile - Helper function to process /proc/pid/status file
returnstring = ""
returnstring = ""
*/
long LinuxParser::ReadPidStatusFile(int pid, string returnstring) {
  string line ;
  std::ifstream filestring(kProcDirectory + to_string(pid) + kStatusFilename) ;
  string key ;
  long value = -1 ;
  if (filestring.is_open()) {
    while (std::getline(filestring, line)) {
      std::replace(line.begin(), line.end(), ':', ' ') ;
      std::istringstream linestream(line) ;
      linestream >> key >> value ;
      if (key == returnstring) {
        return value ;
        break ;
      }
    }
  }
  return value ;
}