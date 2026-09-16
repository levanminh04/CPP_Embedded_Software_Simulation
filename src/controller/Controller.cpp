#include "traffic/Controller.h"

namespace traffic {

Controller::Controller(Config config)
    : config(config),
      currentSnapshot(),
      nsReading(),
      ewReading(),
      quitRequestedFlag(false) {
    if (!this->config.isValid()) {
        this->config = Config::defaults();
    }

    currentSnapshot.state = TrafficState::STARTUP_ALL_RED;
    currentSnapshot.nextDirection = Direction::NS;
    currentSnapshot.remainingSeconds = this->config.allRedTime;
    currentSnapshot.nsVehicleCount = nsReading.vehicleCount;
    currentSnapshot.ewVehicleCount = ewReading.vehicleCount;
}

ControllerResult Controller::apply(const Event& event) {
    if (event.type == EventType::QUIT) {
        requestQuit();
        return {ControllerAction::QUIT_REQUESTED, currentSnapshot.state, currentSnapshot.state,
                false, event.detail};
    }

    if (event.type == EventType::INVALID_INPUT) {
        return {ControllerAction::INVALID_INPUT_IGNORED, currentSnapshot.state,
                currentSnapshot.state, false, event.detail};
    }

    return makeNoChangeResult();
}

ControllerResult Controller::tick() {
    const TrafficState previousState = currentSnapshot.state;

    ++currentSnapshot.simulationSecond;
    decrementTimer();

    if (currentSnapshot.remainingSeconds == 0) {
        handleExpiredState();
    }

    if (currentSnapshot.state != previousState) {
        return makeStateChangeResult(previousState, currentSnapshot.state);
    }

    return makeNoChangeResult();
}

ControllerResult Controller::updateSensor(Direction direction, const SensorReading& reading) {
    if (!reading.valid) {
        return {ControllerAction::SENSOR_ERROR_RECORDED, currentSnapshot.state,
                currentSnapshot.state, false, "invalid sensor reading"};
    }

    storeSensorReading(direction, reading);
    return {ControllerAction::SENSOR_UPDATE_RECORDED, currentSnapshot.state,
            currentSnapshot.state, false, ""};
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

ControllerResult Controller::makeStateChangeResult(TrafficState previousState,
                                                   TrafficState nextState) const {
    return {ControllerAction::STATE_CHANGE, previousState, nextState, true, ""};
}

void Controller::requestPedestrian() {}

void Controller::requestEmergencyToggle() {}

void Controller::requestQuit() {
    quitRequestedFlag = true;
}

void Controller::storeSensorReading(Direction direction, const SensorReading& reading) {
    if (direction == Direction::NS) {
        nsReading = reading;
        currentSnapshot.nsVehicleCount = reading.vehicleCount;
        return;
    }

    ewReading = reading;
    currentSnapshot.ewVehicleCount = reading.vehicleCount;
}

void Controller::enterState(TrafficState nextState, int durationSeconds) {
    currentSnapshot.state = nextState;
    currentSnapshot.remainingSeconds = durationSeconds;
}

void Controller::decrementTimer() {
    if (currentSnapshot.remainingSeconds > 0) {
        --currentSnapshot.remainingSeconds;
    }
}

void Controller::handleExpiredState() {
    // Normal Cycle trong bước này chỉ xử lý xe NS/EW.
    // GREEN luôn đi qua YELLOW cùng hướng trước khi mất quyền.
    // Mỗi lần đổi hướng bắt buộc có ALL_RED để tạo clearance an toàn.
    // P/E được bỏ qua, sẽ nối vào các điểm quyết định ALL_RED ở bước sau.
    switch (currentSnapshot.state) {
    case TrafficState::STARTUP_ALL_RED:
    case TrafficState::ALL_RED_TO_NS:
    case TrafficState::ALL_RED_TO_EW:
        chooseNextVehiclePhase();
        break;
    case TrafficState::NS_GREEN:
        enterState(TrafficState::NS_YELLOW, config.yellowTime);
        break;
    case TrafficState::EW_GREEN:
        enterState(TrafficState::EW_YELLOW, config.yellowTime);
        break;
    case TrafficState::NS_YELLOW:
    case TrafficState::EW_YELLOW:
        chooseAllRedDestination();
        break;
    case TrafficState::ALL_RED_TO_PED:
    case TrafficState::PED_WALK:
    case TrafficState::PED_WARNING:
    case TrafficState::ALL_RED_FROM_PED:
    case TrafficState::EMERGENCY:
        break;
    }
}

void Controller::chooseNextVehiclePhase() {
    const Direction activeDirection = currentSnapshot.nextDirection;
    const TrafficDensity sampledDensity = readTrafficDensity(activeDirection);
    const TrafficState nextGreenState = greenStateFor(activeDirection);

    // Density is sampled exactly before a vehicle GREEN starts.
    // The selected duration is fixed for this GREEN turn.
    // Sensor updates during the turn are kept for the next turn.
    enterState(nextGreenState, greenDurationFor(sampledDensity));
    currentSnapshot.nextDirection = opposite(activeDirection);
}

void Controller::chooseAllRedDestination() {
    enterState(allRedStateToward(currentSnapshot.nextDirection), config.allRedTime);
}

void Controller::startPedestrianPhase() {}

void Controller::startEmergencyClearance() {}

int Controller::durationFor(TrafficState state, Direction direction) const {
    if (state == TrafficState::NS_GREEN || state == TrafficState::EW_GREEN) {
        return greenDurationFor(readTrafficDensity(direction));
    }

    if (state == TrafficState::NS_YELLOW || state == TrafficState::EW_YELLOW) {
        return config.yellowTime;
    }

    if (isAllRed(state)) {
        return config.allRedTime;
    }

    if (state == TrafficState::PED_WALK) {
        return config.pedestrianWalkTime;
    }

    if (state == TrafficState::PED_WARNING) {
        return config.pedestrianWarningTime;
    }

    return 0;
}

TrafficDensity Controller::readTrafficDensity(Direction direction) const {
    const SensorReading& reading = direction == Direction::NS ? nsReading : ewReading;
    return reading.density;
}

int Controller::greenDurationFor(TrafficDensity density) const {
    if (density == TrafficDensity::HIGH) {
        return config.greenHighTime;
    }

    if (density == TrafficDensity::MEDIUM) {
        return config.greenMediumTime;
    }

    return config.greenLowTime;
}

TrafficState Controller::greenStateFor(Direction direction) const {
    return direction == Direction::NS ? TrafficState::NS_GREEN : TrafficState::EW_GREEN;
}

TrafficState Controller::yellowStateFor(Direction direction) const {
    return direction == Direction::NS ? TrafficState::NS_YELLOW : TrafficState::EW_YELLOW;
}

TrafficState Controller::allRedStateToward(Direction direction) const {
    return direction == Direction::NS ? TrafficState::ALL_RED_TO_NS : TrafficState::ALL_RED_TO_EW;
}

Direction Controller::opposite(Direction direction) const {
    return direction == Direction::NS ? Direction::EW : Direction::NS;
}

bool Controller::isVehicleGreen(TrafficState state) const {
    return state == TrafficState::NS_GREEN || state == TrafficState::EW_GREEN;
}

bool Controller::isVehicleYellow(TrafficState state) const {
    return state == TrafficState::NS_YELLOW || state == TrafficState::EW_YELLOW;
}

bool Controller::isAllRed(TrafficState state) const {
    return state == TrafficState::STARTUP_ALL_RED || state == TrafficState::ALL_RED_TO_NS ||
           state == TrafficState::ALL_RED_TO_EW || state == TrafficState::ALL_RED_TO_PED ||
           state == TrafficState::ALL_RED_FROM_PED;
}

bool Controller::isPedestrianPhase(TrafficState state) const {
    return state == TrafficState::PED_WALK || state == TrafficState::PED_WARNING;
}

bool Controller::isEmergencyState(TrafficState state) const {
    return state == TrafficState::EMERGENCY;
}

bool Controller::shouldServePedestrian() const {
    return false;
}

}
