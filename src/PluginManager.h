#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

class PluginManager
{

public:
	PluginManager();
	PluginManager(Plugin *plugin);

	PluginManager* setPlugin(Plugin *plugin);
	int run();

private:
	Plugin *_plugin = nullptr;
};


#endif
