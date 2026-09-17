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
        void decrementRemainingTime();
        void handleExpiredState();
        void chooseNextVehiclePhase();
        void startPedestrianPhase();

        TrafficDensity readTrafficDensity(Direction direction) const;
        int greenDurationFor(TrafficDensity density) const;
        TrafficState greenStateFor(Direction direction) const;
        Direction opposite(Direction direction) const;

        bool shouldServePedestrian() const;
    };

}
