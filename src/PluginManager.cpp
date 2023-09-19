#include <iostream>

#include "Plugin.h"
#include "PluginManager.h"


PluginManager::PluginManager() {}

PluginManager::PluginManager(Plugin* _plugin) : plugin(_plugin) {}

PluginManager* PluginManager::setPlugin(Plugin* _plugin)
{
	plugin = _plugin;
	return this;
}


int PluginManager::run()
{
	if (!plugin) {
		std::cerr << "No plugin to run in PluginManager" << std::endl;
		return 0;
	}
	return plugin->run();
}
