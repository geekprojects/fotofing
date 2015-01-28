
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

#include <map>

#include <fotofing/plugin.h>

using namespace std;

map<string, vector<FotofingPlugin*> > g_plugins;

const char* pluginPaths[] = {
    "src/plugins/operations/*/.libs/",
    "src/plugins/taggers/*/.libs/",
    "/usr/lib",
    "/usr/local/lib",
};

#define PLUGIN_LIB_PREFIX "libfotofing-"

static int pluginfilter(const struct dirent* e)
{
    string name = string(e->d_name);
    return (
        name.find(PLUGIN_LIB_PREFIX) == 0 &&
        name.rfind(".so") == name.size() - 3 );
}

FotofingPlugin::FotofingPlugin()
{
}

FotofingPlugin::~FotofingPlugin()
{
}

vector<FotofingPlugin*> FotofingPlugin::getPlugins(string type)
{
    map<string, vector<FotofingPlugin*> >::iterator pluginIt = g_plugins.find(type);
    if (pluginIt != g_plugins.end())
    {
        return pluginIt->second;
    }

    // Expand the plugin paths
    vector<string> expandedPaths;
    int i;
    for (i = 0; pluginPaths[i] != NULL; i++)
    {
        string path = string(pluginPaths[i]);
        unsigned int pos = path.find("/*/");
        if (pos != -1)
        {
            string scanPath = path.substr(0, pos);
            string subDir = path.substr(pos + 3);

            DIR* dir = opendir(scanPath.c_str());

            while (true)
            {
                struct dirent* dirent;
                dirent = readdir(dir);
                if (dirent == NULL)
                {
                    break;
                }

                if (dirent->d_name[0] == '.' || dirent->d_type != DT_DIR)
                {
                    continue;
                }

                path = scanPath + "/" + string(dirent->d_name) + "/" + subDir;
                expandedPaths.push_back(path);
            }

            closedir(dir);
        }
        else
        {
            expandedPaths.push_back(path);
        }
    }

    vector<FotofingPlugin*> results;
    vector<string>::iterator pathIt;
    for (
        pathIt = expandedPaths.begin();
        pathIt != expandedPaths.end();
        pathIt++)
    {
        const char* path = pathIt->c_str();

        struct dirent** namelist;
        int n;
        n = scandir(path, &namelist, pluginfilter, alphasort);
        if (n < 0)
        {
            printf("FotofingPlugin::getPlugins: Failed to scan directory\n");
            continue;
        }

        while (n--)
        {
            string pluginlib = string(path) + "/" + string(namelist[n]->d_name);
            printf("%s\n", pluginlib.c_str());

            void* pluginHandle = dlopen(pluginlib.c_str(), RTLD_LAZY);

            if (pluginHandle == NULL)
            {
                printf("FotofingPlugin::getPlugins Failed to open plugin: %s\n", dlerror());
                continue;
            }

            string symbol = "fotofing_" + type + "_create";

            createPlugin_t pluginCreate;
            pluginCreate = (createPlugin_t)dlsym(pluginHandle, symbol.c_str());
            if (pluginCreate == NULL)
            {
                // Not the right type of plugin
                dlclose(pluginHandle);
                continue;
            }

            FotofingPlugin* plugin = pluginCreate();

            plugin->m_path = pluginlib;
            plugin->m_handle = pluginHandle;
            results.push_back(plugin);

            free(namelist[n]);
        }
        free(namelist);
    }

    g_plugins.insert(make_pair(type, results));

    return results;
}

FotofingPlugin* FotofingPlugin::getPlugin(string type, string name)
{
    vector<FotofingPlugin*> plugins = getPlugins(type);
    vector<FotofingPlugin*>::iterator it;
    for (it = plugins.begin(); it !=plugins.end(); it++)
    {
        FotofingPlugin* plugin = *it;
        if (plugin->getName() == name)
        {
            return plugin;
        }
    }
    return NULL;
}

