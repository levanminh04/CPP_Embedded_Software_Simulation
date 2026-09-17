#pragma once

#include "traffic/Config.h"
#include "traffic/ConsoleInput.h"
#include "traffic/Controller.h"
#include "traffic/Display.h"
#include "traffic/Logger.h"
#include "traffic/Output.h"
#include "traffic/Sensor.h"
#include "traffic/Timer.h"
#include "traffic/Types.h"

#include <string>

namespace traffic
{

class Application
{
public:
    Application();

    int run();

private:
    Config config_;

    ConsoleInput input_;
    TrafficDensitySensor sensor_;

    // Timer và Logger hiện tại đang ở global namespace.
    ::Timer timer_;
    Controller controller_;

    Output output_;
    Display display_;
    ::Logger logger_;

    bool running_ = true;

    std::string lastCommandBuffer_;

    void handleInputEvent(const Event &event);

    void handleSensorUpdate(const Event &event);

    void handleControllerResult(
        const ControllerResult &result);

    void render(
        const std::string &command);
};

} // namespace traffic