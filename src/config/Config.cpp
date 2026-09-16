#include "traffic/Config.h"

namespace traffic {

Config Config::defaults() {
    return Config();
}

bool Config::isValid() {
    return tickMs > 0 && greenLowTime > 0 && greenMediumTime > 0 && greenHighTime > 0 &&
           yellowTime > 0 && allRedTime > 0 && pedestrianWalkTime > 0 &&
           pedestrianWarningTime > 0 && densityLowMax >= 0 &&
           densityMediumMax > densityLowMax;
}

}
