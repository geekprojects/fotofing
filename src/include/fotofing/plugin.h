#ifndef __FOTOFING_PLUGIN_H_
#define __FOTOFING_PLUGIN_H_

#include <string>
#include <vector>

class FotofingPlugin
{
 private:
    void* m_handle;

 protected:
    std::string m_path;

 public:
    FotofingPlugin();
    virtual ~FotofingPlugin();

    virtual std::string getName() = 0;

    static std::vector<FotofingPlugin*> getPlugins(std::string type);
    static FotofingPlugin* getPlugin(std::string type, std::string name);
};

typedef FotofingPlugin*(*createPlugin_t)();

#define DECLARE_PLUGIN(_type, _class) \
    extern "C" FotofingPlugin* fotofing_ ## _type ## _create() \
    { \
        return new _class(); \
    }


#endif
