#include "traffic/EventSimulator.h"
#include "traffic/InputValidator.h"

#include <sstream>

using namespace std;

namespace traffic {

Event EventSimulator::parseLine(string line) {
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
