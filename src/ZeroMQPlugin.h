#ifndef ZEROMQPLUGIN_H
#define ZEROMQPLUGIN_H

#include "Plugin.h"
#include <zmq.hpp>

class ZeroMQPlugin : public Plugin
{

public:
	ZeroMQPlugin(QApplication* app);

	int run();

private:
	QApplication* app;
};

#endif