#ifndef ZEROMQPAIRPLUGIN_H
#define ZEROMQPAIRPLUGIN_H

#include <QApplication>

#include "Plugin.h"

class ZeroMQPairPlugin : public Plugin
{

public:
	ZeroMQPairPlugin(QApplication* app);

	int run();

private:
	QApplication* app;
};

#endif
