#include <iostream>
//#include <chrono>
//#include <thread>

#include <QThread>

#include "ZeroMQPlugin.h"
#include "QZeroMQClient.h"

ZeroMQPlugin::ZeroMQPlugin(QApplication* _app) : app(_app) {}

int ZeroMQPlugin::run()
{
	QZeroMQClient client;
	//QThread audioThread;
	QThread* clientThread = QThread::create([&]() {
		client.serve();
		});
	client.moveToThread(clientThread);



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


	clientThread->start();

	int ret = app->exec();

	clientThread->wait();

	return ret;
}

