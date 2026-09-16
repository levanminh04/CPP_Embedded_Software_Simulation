#pragma once

#include "traffic/Types.h"

namespace traffic {

class Output {
public:
    LightOutput fromState(TrafficState state) const noexcept;
};

} // namespace traffic
