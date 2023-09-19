#ifndef PYAPIPLUGIN_H
#define PYAPIPLUGIN_H

#include <QApplication>

#include "Plugin.h"

class PyAPIPlugin : public Plugin
{

public:
	PyAPIPlugin(QApplication* app);

	static void playAudio();

	int run();

private:
	QApplication* app;
};

#endif