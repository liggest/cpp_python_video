#ifndef ZEROMQIOPLUGIN_H
#define ZEROMQIOPLUGIN_H

#include <QApplication>

#include "Plugin.h"

class ZeroMQIOPlugin : public Plugin
{

public:
	ZeroMQIOPlugin(QApplication* app);

	int run();

private:
	QApplication* app;
};

#endif
