#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "format.h"

using std::string;
using std::to_string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
    //std::cout << "seconds are :" << seconds << "\n" ;
    std::stringstream timeFormated;
    if (seconds > 0) {
        
        int HH  = seconds / (60 * 60) ;
        int MM  = seconds % (60 * 60) / 60 ;
        int SS  = (seconds % (60 * 60)) % 60 ;
    
        timeFormated << std::setfill('0') << std::setw(2) << to_string(HH) << ":" ;
        timeFormated   << std::setfill('0') << std::setw(2) << to_string(MM) << ":" ;
        timeFormated  << std::setfill('0') << std::setw(2) << to_string(SS) ;
    }
    else {
        timeFormated << "00:00:00" ;
    }

    return timeFormated.str(); 
}