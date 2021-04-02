#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;
using std::cout ;

Process::Process(int pid) {
   this->pid_ = pid ;
   cpuUtilization_ = LinuxParser::CalculateCpuUtilization (this->pid_) ;
   command_ = LinuxParser::Command(this->pid_) ;
   ram_ = LinuxParser::Ram(this->pid_) ;
   username_ = LinuxParser::User(this->pid_) ;
   uptime_ = LinuxParser::UpTime(this->pid_) ;
}

// Return this process's ID
int Process::Pid() { return this->pid_ ; }

// Return this process's CPU utilization
float Process::CpuUtilization() { return this->cpuUtilization_; }  

// Return the command that generated this process
string Process::Command() { return this->command_ ; }

// Return this process's memory utilization
string Process::Ram() { return this->ram_ ; }

// Return the user (name) that generated this process
string Process::User() { return this->username_ ; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return this->uptime_ ; }

// Overload the "less than" comparison operator for Process objects
// To compare two processes with their cpuUtilizations
bool Process::operator<(Process const& a) const { 
    if (this->cpuUtilization_ > a.cpuUtilization_) {
        return true ; 
    }
    return false ;    
}
