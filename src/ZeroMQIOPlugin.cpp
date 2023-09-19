#include <iostream>
//#include <chrono>
//#include <thread>

#include <QThread>

#include "ZeroMQIOPlugin.h"
#include "QZeroMQIOClient.h"
#include "AudioInputter.h"

ZeroMQIOPlugin::ZeroMQIOPlugin(QApplication* _app) : app(_app) {}

int ZeroMQIOPlugin::run()
{
	QZeroMQIOClient client;
	QThread* clientThread = QThread::create([&]() {
		client.serve();
		});
	client.moveToThread(clientThread);

	clientThread->start();

	int ret = app->exec();

	clientThread->wait();

	return ret;
}

