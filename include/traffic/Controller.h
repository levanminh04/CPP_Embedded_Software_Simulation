#pragma once

#include "traffic/Config.h"
#include "traffic/Types.h"

#include <string>

namespace traffic
{

    enum class ControllerAction
    {
        NONE,
        STATE_CHANGE,
        PEDESTRIAN_REQUEST_RECORDED,
        PEDESTRIAN_SERVICE_STARTED,
        PEDESTRIAN_SERVICE_FINISHED,
        EMERGENCY_ON_REQUESTED,
        EMERGENCY_OFF_REQUESTED,
        SENSOR_UPDATE_RECORDED,
        SENSOR_ERROR_RECORDED,
        INVALID_INPUT_IGNORED,
        QUIT_REQUESTED
    };

    struct ControllerResult
    {
        ControllerAction action = ControllerAction::NONE;
        TrafficState previousState = TrafficState::STARTUP_ALL_RED;
        TrafficState currentState = TrafficState::STARTUP_ALL_RED;
        bool stateChanged = false;
        std::string detail;
    };

    class Controller
    {
    public:
        explicit Controller(Config config = Config::defaults());

        ControllerResult apply(const Event &event);
        ControllerResult tick();
        ControllerResult updateSensor(Direction direction, const SensorReading &reading);

        SystemSnapshot snapshot() const;
        TrafficState state() const;
        Direction nextDirection() const;
        int remainingSeconds() const;
        bool pedestrianRequested() const;
        bool emergencyPending() const;
        bool quitRequested() const;

    private:
        Config config;
        SystemSnapshot currentSnapshot;
        SensorReading nsReading;
        SensorReading ewReading;
        bool quitRequestedFlag = false;
        bool pedestrianCooldown = false;

        ControllerResult makeNoChangeResult() const;
        ControllerResult makeStateChangeResult(TrafficState previousState,
                                               TrafficState nextState) const;

        void requestPedestrian();
        void requestEmergencyToggle();
        void requestQuit();
        void storeSensorReading(Direction direction, const SensorReading &reading);

        void enterState(TrafficState nextState, int durationSeconds);
        void decrementTimer();
        void handleExpiredState();
        void chooseNextVehiclePhase();
        void chooseAllRedDestination();
        void startPedestrianPhase();
        void startEmergencyClearance();

        int durationFor(TrafficState state, Direction direction) const;
        TrafficDensity readTrafficDensity(Direction direction) const;
        int greenDurationFor(TrafficDensity density) const;
        TrafficState greenStateFor(Direction direction) const;
        TrafficState yellowStateFor(Direction direction) const;
        TrafficState allRedStateToward(Direction direction) const;
        Direction opposite(Direction direction) const;

        bool isVehicleGreen(TrafficState state) const;
        bool isVehicleYellow(TrafficState state) const;
        bool isAllRed(TrafficState state) const;
        bool isPedestrianPhase(TrafficState state) const;
        bool isEmergencyState(TrafficState state) const;
        bool shouldServePedestrian() const;
    };

}
