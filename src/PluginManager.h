#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

class PluginManager
{

public:
	PluginManager();
	PluginManager(Plugin* _plugin);

	PluginManager* setPlugin(Plugin* _plugin);
	int run();

private:
	Plugin* plugin = nullptr;
};


#endif
