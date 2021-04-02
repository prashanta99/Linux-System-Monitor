#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// System
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_,
  kProcesses_,
  kProcsRunning_
};

// utime is at position 14 but since vector starts at position 0 we start with position 13
// Same for starttime 
enum PIDCpuStates {
    kUtime_=13,
    kStime_,
    kCUtime_,
    kCStime_,
    kStarttime_=21
};

std::vector<std::string> CpuUtilization();
long Jiffies();
long ActiveJiffies();
long ActiveJiffies(int pid);
long IdleJiffies();

// Processes
std::string Command(int pid);
std::string Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
long UpTime(int pid);
float CalculateCpuUtilization(int pid);

// Helper Functions
long ReadPidStatusFile(int pid, std::string returnstring);
long ReadPidStatFile(int pid, std::string returnflag); 
long ProcessProcStatData(int returnflag);
void ReadProcStatFile(std::vector<std::string> &statRecord);

};
#endif