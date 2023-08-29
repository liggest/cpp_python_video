#include <QApplication>

#include "PyAPIPlugin.h"
#include "PluginManager.h"


int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

    PyAPIPlugin pyapi = PyAPIPlugin(&app);
	PluginManager pm(&pyapi);
	
	int ret = pm.run();
	return ret;
}