#include "processor.h"
#include "linux_parser.h"
#include <iostream>
using std::cout ;

Processor::Processor() : previousActiveJiffies_(0), previousIdleJiffies_(0){};

// Return the aggregate CPU utilization
float Processor::Utilization() { 
    float cpuUtilization = 0 ;
    long currentActiveJiffies = LinuxParser::ActiveJiffies() ;
    long currentIdleJiffies = LinuxParser::IdleJiffies() ;
    long currentTotalJiffies = currentActiveJiffies + currentIdleJiffies ;

    long previousTotalJiffies = this->previousActiveJiffies_ + this->previousIdleJiffies_ ;

    float netActiveJiffies = currentTotalJiffies - previousTotalJiffies ;
    float netIdleJiffies = currentIdleJiffies -  this->previousIdleJiffies_ ;
    
    if (netActiveJiffies == 0) {
        cpuUtilization = 0 ;
    }
    else {
        cpuUtilization = (netActiveJiffies - netIdleJiffies)/netActiveJiffies ;
    }
 
    this->previousActiveJiffies_ = currentActiveJiffies ;
    this->previousIdleJiffies_ = currentIdleJiffies ;
    return cpuUtilization;
}
