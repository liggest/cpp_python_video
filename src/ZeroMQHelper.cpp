#include <iostream>

#include "ZeroMQHelper.h"

void ZeroMQHelper::printVersion()
{
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    std::cout << "Current 0MQ version is " << major << "." << minor << "." << patch << std::endl;
}
