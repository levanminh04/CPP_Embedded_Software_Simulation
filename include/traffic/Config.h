#pragma once

namespace traffic {

struct Config {
    int tickMs = 1000;
    int greenLowTime = 15;
    int greenMediumTime = 20;
    int greenHighTime = 25;
    int yellowTime = 3;
    int allRedTime = 2;
    int pedestrianWalkTime = 10;
    int pedestrianWarningTime = 5;
    int densityLowMax = 5;
    int densityMediumMax = 15;

    static Config defaults();
    bool isValid();
};

} 
