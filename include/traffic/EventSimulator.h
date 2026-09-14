#pragma once

#include "traffic/Types.h"

#include <string>

namespace traffic {

class EventSimulator {
public:
    Event parseLine(std::string line);
};

} 
