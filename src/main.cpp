#include <QApplication>

#include "PyAPIPlugin.h"
#include "ZeroMQPlugin.h"
#include "ZeroMQPairPlugin.h"
#include "PluginManager.h"


int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

    //PyAPIPlugin plugin = PyAPIPlugin(&app);
	ZeroMQPlugin plugin = ZeroMQPlugin(&app);
	//ZeroMQPairPlugin plugin = ZeroMQPairPlugin(&app);
	PluginManager pm(&plugin);
	
	int ret = pm.run();
	return ret;
}