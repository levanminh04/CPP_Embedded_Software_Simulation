#include "traffic/Application.h"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        traffic::Application app;

        return app.run();
    }
    catch (const std::exception &exception)
    {
        std::cerr
            << "Fatal error: "
            << exception.what()
            << '\n';

        return 1;
    }
}