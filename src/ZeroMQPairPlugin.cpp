#include <iostream>
#include <chrono>
#include <thread>

#include <QThread>

#include "ZeroMQPairPlugin.h"
#include "QZeroMQServer.h"

ZeroMQPairPlugin::ZeroMQPairPlugin(QApplication* _app) : app(_app) {}

int ZeroMQPairPlugin::run()
{
    QZeroMQServer server;
    //QThread audioThread;
    QThread* serverThread = QThread::create([&]() {
        server.serve();
        });
    server.moveToThread(serverThread);
    


    //while (true) {
    //    zmq::message_t request;

    //    //  Wait for next request from client
    //    socket.recv (request, zmq::recv_flags::none);
    //    std::cout << "Received Hello" << std::endl;

    //    //  Do some 'work'
    //    std::this_thread::sleep_for(std::chrono::seconds(1));

    //    //  Send reply back to client
    //    zmq::message_t reply (5);
    //    memcpy (reply.data (), "World", 5);
    //    socket.send (reply, zmq::send_flags::none);
    //}

    serverThread->start();

    int ret = app->exec();

    serverThread->wait();

    return ret;
}

