#ifndef PYAPIPLUGIN_H
#define PYAPIPLUGIN_H

#include "Plugin.h"

class PyAPIPlugin : public Plugin
{

public:
	PyAPIPlugin(QApplication *app);

	static void play_audio();

	int run();

private:
	QApplication *app;
};

#endif