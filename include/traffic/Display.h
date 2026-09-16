#pragma once

#include "traffic/Types.h"

#include <iostream>
#include <ostream>
#include <string_view>

namespace traffic {

class Display {
public:
    explicit Display(std::ostream& stream = std::cout, bool clearScreen = true);

    void show(
        const SystemSnapshot& snapshot,
        const LightOutput& lights,
        std::string_view command = {});

private:
    std::ostream& stream_;
    bool clearScreen_;
};

} // namespace traffic
