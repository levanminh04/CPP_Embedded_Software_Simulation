#pragma once

#include "traffic/Types.h"

#include <string>

namespace traffic {

class ConsoleInput {
private:
    std::string buffer;

    Event parseLine(std::string line);

public:
    ConsoleInput();

    bool poll(Event& event);
    bool feedKey(char key, Event& event);
    std::string currentBuffer();
};

}
