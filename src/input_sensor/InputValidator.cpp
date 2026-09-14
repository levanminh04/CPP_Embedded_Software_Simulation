#include "traffic/InputValidator.h"

#include <sstream>

using namespace std;

namespace traffic {

bool parseVehicleCount(const string& text, int& vehicleCount) {
    string extra;
    stringstream input(text);

    if (!(input >> vehicleCount) || (input >> extra)) {
        return false;
    }

    return true;
}

}
