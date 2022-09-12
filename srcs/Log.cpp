#include "Log.hpp"

LOG::LOG()
:
    opened(false)
{}

LOG::LOG(typelog type)
:
    opened(false)
{
    // Calls LOG operator<< (important for opened = true)
    operator<< ("["+loglevel[type]+"] ");
}

LOG::~LOG() {
    if (opened) {
        std::cout << std::endl;
    }
    opened = false;
}
