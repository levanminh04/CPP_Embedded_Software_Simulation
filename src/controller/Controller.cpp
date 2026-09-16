#include "traffic/Controller.h"

namespace traffic {

Controller::Controller(Config config)
    : config(config),
      currentSnapshot(),
      nsReading(),
      ewReading(),
      quitRequestedFlag(false) {}

ControllerResult Controller::apply(const Event& event) {
    (void)event;
    return makeNoChangeResult();
}

ControllerResult Controller::tick() {
    return makeNoChangeResult();
}

ControllerResult Controller::updateSensor(Direction direction, const SensorReading& reading) {
    (void)direction;
    (void)reading;
    return makeNoChangeResult();
}

SystemSnapshot Controller::snapshot() const {
    return currentSnapshot;
}

TrafficState Controller::state() const {
    return currentSnapshot.state;
}

Direction Controller::nextDirection() const {
    return currentSnapshot.nextDirection;
}

int Controller::remainingSeconds() const {
    return currentSnapshot.remainingSeconds;
}

bool Controller::pedestrianRequested() const {
    return currentSnapshot.pedestrianRequested;
}

bool Controller::emergencyPending() const {
    return currentSnapshot.emergencyPending;
}

bool Controller::quitRequested() const {
    return quitRequestedFlag;
}

ControllerResult Controller::makeNoChangeResult() const {
    return {ControllerAction::NONE, currentSnapshot.state, currentSnapshot.state, false, ""};
}

}
