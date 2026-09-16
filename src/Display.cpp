#include "traffic/Display.h"

namespace traffic {
namespace {

const char* stateName(const TrafficState state) noexcept
{
    switch (state) {
    case TrafficState::STARTUP_ALL_RED:
        return "STARTUP_ALL_RED";
    case TrafficState::NS_GREEN:
        return "NS_GREEN";
    case TrafficState::NS_YELLOW:
        return "NS_YELLOW";
    case TrafficState::ALL_RED:
        return "ALL_RED";
    case TrafficState::EW_GREEN:
        return "EW_GREEN";
    case TrafficState::EW_YELLOW:
        return "EW_YELLOW";
    case TrafficState::PED_WALK:
        return "PED_WALK";
    case TrafficState::PED_WARNING:
        return "PED_WARNING";
    case TrafficState::EMERGENCY:
        return "EMERGENCY";
    default:
        return "UNKNOWN";
    }
}

const char* vehicleLightName(const VehicleLight light) noexcept
{
    switch (light) {
    case VehicleLight::RED:
        return "RED";
    case VehicleLight::GREEN:
        return "GREEN";
    case VehicleLight::YELLOW:
        return "YELLOW";
    default:
        return "UNKNOWN";
    }
}

const char* pedestrianLightName(const PedestrianLight light) noexcept
{
    switch (light) {
    case PedestrianLight::RED:
        return "RED";
    case PedestrianLight::GREEN:
        return "GREEN";
    case PedestrianLight::WARNING:
        return "WARNING";
    default:
        return "UNKNOWN";
    }
}

} // namespace

Display::Display(std::ostream& stream, const bool clearScreen)
    : stream_(stream), clearScreen_(clearScreen)
{
}

void Display::show(
    const SystemSnapshot& snapshot,
    const LightOutput& lights,
    const std::string_view command)
{
    if (clearScreen_) {
        stream_ << "\x1B[2J\x1B[H";
    }

    stream_ << "===== SMART TRAFFIC CONTROLLER =====\n"
            << "Simulation Time      : " << snapshot.simulationSecond << " s\n"
            << "State                : " << stateName(snapshot.state) << '\n'
            << "NS Vehicle LED       : " << vehicleLightName(lights.ns) << '\n'
            << "EW Vehicle LED       : " << vehicleLightName(lights.ew) << '\n'
            << "Pedestrian LED       : " << pedestrianLightName(lights.pedestrian) << '\n'
            << "Remaining Time       : " << snapshot.remainingSeconds << " s\n"
            << "Pedestrian Req.      : "
            << (snapshot.pedestrianRequested ? "YES" : "NO") << '\n'
            << "Emergency            : "
            << (snapshot.emergencyPending ? "ON" : "OFF") << '\n'
            << "Traffic NS / EW      : " << snapshot.nsVehicleCount << " / "
            << snapshot.ewVehicleCount << '\n'
            << "Command              : " << command << "\n"
            << "====================================\n"
            << "Nhap lenh roi nhan Enter\n"
            << "P = Pedestrian request | E = Emergency ON/OFF | Q = Quit\n"
            << "NS 20 = NS sensor     | EW 4 = EW sensor\n"
            << std::flush;
}

} // namespace traffic
