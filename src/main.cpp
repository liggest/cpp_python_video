#include <QApplication>

#include "PyAPIPlugin.h"
#include "PluginManager.h"


int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	PluginManager pm(&PyAPIPlugin(&app));
	
	int ret = pm.run();
	return ret;
}