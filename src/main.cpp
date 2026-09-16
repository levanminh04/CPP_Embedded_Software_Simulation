#include "traffic/Display.h"
#include "traffic/Output.h"

int main()
{
    const traffic::SystemSnapshot snapshot{};
    const traffic::Output output;
    const traffic::LightOutput lights = output.fromState(snapshot.state);
    traffic::Display display;
    display.show(snapshot, lights);
    return 0;
}
