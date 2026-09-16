#include "traffic/Display.h"
#include "traffic/Output.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace {

void assertLights(
    const traffic::LightOutput& actual,
    const traffic::VehicleLight ns,
    const traffic::VehicleLight ew,
    const traffic::PedestrianLight pedestrian)
{
    assert(actual.ns == ns);
    assert(actual.ew == ew);
    assert(actual.pedestrian == pedestrian);
}

void testOutputMapping()
{
    const traffic::Output output;

    assertLights(
        output.fromState(traffic::TrafficState::NS_GREEN),
        traffic::VehicleLight::GREEN,
        traffic::VehicleLight::RED,
        traffic::PedestrianLight::RED);
    assertLights(
        output.fromState(traffic::TrafficState::NS_YELLOW),
        traffic::VehicleLight::YELLOW,
        traffic::VehicleLight::RED,
        traffic::PedestrianLight::RED);
    assertLights(
        output.fromState(traffic::TrafficState::ALL_RED),
        traffic::VehicleLight::RED,
        traffic::VehicleLight::RED,
        traffic::PedestrianLight::RED);
    assertLights(
        output.fromState(traffic::TrafficState::EW_GREEN),
        traffic::VehicleLight::RED,
        traffic::VehicleLight::GREEN,
        traffic::PedestrianLight::RED);
    assertLights(
        output.fromState(traffic::TrafficState::EW_YELLOW),
        traffic::VehicleLight::RED,
        traffic::VehicleLight::YELLOW,
        traffic::PedestrianLight::RED);
    assertLights(
        output.fromState(traffic::TrafficState::PED_WALK),
        traffic::VehicleLight::RED,
        traffic::VehicleLight::RED,
        traffic::PedestrianLight::GREEN);
    assertLights(
        output.fromState(traffic::TrafficState::PED_WARNING),
        traffic::VehicleLight::RED,
        traffic::VehicleLight::RED,
        traffic::PedestrianLight::WARNING);
    assertLights(
        output.fromState(traffic::TrafficState::EMERGENCY),
        traffic::VehicleLight::RED,
        traffic::VehicleLight::RED,
        traffic::PedestrianLight::RED);
}

void testDisplayRendering()
{
    traffic::SystemSnapshot snapshot;
    snapshot.simulationSecond = 18;
    snapshot.state = traffic::TrafficState::ALL_RED;
    snapshot.pendingNext = traffic::TrafficState::NS_GREEN;
    snapshot.remainingSeconds = 2;
    snapshot.nsVehicleCount = 20;
    snapshot.ewVehicleCount = 4;
    snapshot.pedestrianRequested = true;
    snapshot.emergencyPending = false;

    traffic::LightOutput lights;

    std::ostringstream rendered;
    traffic::Display display(rendered, false);
    display.show(snapshot, lights, "NS 2");

    const std::string text = rendered.str();
    assert(text.find("Simulation Time      : 18 s") != std::string::npos);
    assert(text.find("State                : ALL_RED") != std::string::npos);
    assert(text.find("Pending Next State   : NS_GREEN") != std::string::npos);
    assert(text.find("NS Vehicle LED       : RED") != std::string::npos);
    assert(text.find("EW Vehicle LED       : RED") != std::string::npos);
    assert(text.find("Pedestrian LED       : RED") != std::string::npos);
    assert(text.find("Remaining Time       : 2 s") != std::string::npos);
    assert(text.find("Pedestrian Req.      : YES") != std::string::npos);
    assert(text.find("Emergency            : OFF") != std::string::npos);
    assert(text.find("Traffic NS / EW      : 20 / 4") != std::string::npos);
    assert(text.find("Command              : NS 2") != std::string::npos);
}

void testPendingNextRules()
{
    const traffic::SystemSnapshot defaultSnapshot;
    assert(defaultSnapshot.state == traffic::TrafficState::ALL_RED);
    assert(defaultSnapshot.pendingNext == traffic::TrafficState::NS_GREEN);
}

} // namespace

int main()
{
    constexpr int cxxStandardSmokeValue = 17;
    static_assert(cxxStandardSmokeValue == 17, "C++17 is required");

    testOutputMapping();
    testDisplayRendering();
    testPendingNextRules();

    std::cout << "Output and display tests passed.\n";
    return 0;
}
