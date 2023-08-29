#include <iostream>

#include "Plugin.h"
#include "PluginManager.h"


PluginManager::PluginManager() {}

PluginManager::PluginManager(Plugin *plugin) : _plugin(plugin) {}

PluginManager* PluginManager::setPlugin(Plugin *plugin)
{
    _plugin = plugin;
    return this;
}


int PluginManager::run()
{
    if (!_plugin) {
        std::cerr << "No plugin to run in PluginManager" << std::endl;
        return 0;
    }
    return _plugin->run();
}
