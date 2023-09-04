#ifndef ZEROMQPLUGIN_H
#define ZEROMQPLUGIN_H

#include "Plugin.h"

class ZeroMQPlugin : public Plugin
{

public:
	ZeroMQPlugin(QApplication* app);

	int run();

private:
	QApplication* app;
};

#endif