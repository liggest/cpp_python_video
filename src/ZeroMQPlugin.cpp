#include <QApplication>
#include <zmq.hpp>

#include <iostream>
#include <chrono>
#include <thread>

#include "ZeroMQPlugin.h"

ZeroMQPlugin::ZeroMQPlugin(QApplication* _app) : app(_app) {}

void printVersion() {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    std::cout << "Current 0MQ version is " << major << " " << minor << " " << patch << std::endl;
}

int ZeroMQPlugin::run()
{
    printVersion();
    //  Prepare our context and socket
    zmq::context_t context (2);
    zmq::socket_t socket (context, zmq::socket_type::rep);
    socket.bind ("tcp://*:5555");

    while (true) {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv (request, zmq::recv_flags::none);
        std::cout << "Received Hello" << std::endl;

        //  Do some 'work'
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //  Send reply back to client
        zmq::message_t reply (5);
        memcpy (reply.data (), "World", 5);
        socket.send (reply, zmq::send_flags::none);
    }
    return 0;
}

