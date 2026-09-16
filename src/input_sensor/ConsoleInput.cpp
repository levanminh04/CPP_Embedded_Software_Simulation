#include "traffic/ConsoleInput.h"
#include "traffic/InputValidator.h"

#include <conio.h>
#include <sstream>

using namespace std;

namespace traffic {

ConsoleInput::ConsoleInput() {}

bool ConsoleInput::poll(Event& event) {
    while (_kbhit()) {
        char key = static_cast<char>(_getch());
        if (feedKey(key, event)) {
            return true;
        }
    }

    return false;
}

bool ConsoleInput::feedKey(char key, Event& event) {
    if (key == '\r' || key == '\n') {
        event = parseLine(buffer);
        buffer.clear();
        return true;
    }

    if (key == '\b') {
        if (!buffer.empty()) {
            buffer.pop_back();
        }
        return false;
    }

    if (key == 27) {
        buffer.clear();
        return false;
    }

    buffer += key;
    return false;
}

string ConsoleInput::currentBuffer() {
    return buffer;
}

Event ConsoleInput::parseLine(string line) {
    stringstream input(line);
    string command;
    string valueText;
    string extra;
    int vehicleCount;

    if (!(input >> command)) {
        return {EventType::INVALID_INPUT, Direction::NS, 0, "empty input"};
    }

    if (command == "P" && !(input >> extra)) {
        return {EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""};
    }

    if (command == "E" && !(input >> extra)) {
        return {EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""};
    }

    if (command == "Q" && !(input >> extra)) {
        return {EventType::QUIT, Direction::NS, 0, ""};
    }

    if ((command == "NS" || command == "EW") && input >> valueText && !(input >> extra) &&
        parseVehicleCount(valueText, vehicleCount)) {
        Direction direction = command == "NS" ? Direction::NS : Direction::EW;
        return {EventType::SENSOR_UPDATE, direction, vehicleCount, ""};
    }

    return {EventType::INVALID_INPUT, Direction::NS, 0, "invalid input"};
}

}
