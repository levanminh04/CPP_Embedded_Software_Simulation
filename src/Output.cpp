#include "traffic/Output.h"

namespace traffic {

LightOutput Output::fromState(const TrafficState state) const noexcept
{
    LightOutput lights{};

    switch (state) {
    case TrafficState::NS_GREEN:
        lights.ns = VehicleLight::GREEN;
        break;
    case TrafficState::NS_YELLOW:
        lights.ns = VehicleLight::YELLOW;
        break;
    case TrafficState::EW_GREEN:
        lights.ew = VehicleLight::GREEN;
        break;
    case TrafficState::EW_YELLOW:
        lights.ew = VehicleLight::YELLOW;
        break;
    case TrafficState::PED_WALK:
        lights.pedestrian = PedestrianLight::GREEN;
        break;
    case TrafficState::PED_WARNING:
        lights.pedestrian = PedestrianLight::WARNING;
        break;
    case TrafficState::ALL_RED:
    case TrafficState::EMERGENCY:
    default:
        break;
    }

    return lights;
}

} // namespace traffic
