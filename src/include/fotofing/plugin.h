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
    ~FotofingPlugin();

    static std::vector<FotofingPlugin*> getPlugins(std::string type);
};

typedef FotofingPlugin*(*createPlugin_t)();

#define DECLARE_PLUGIN(_type, _class) \
    extern "C" FotofingPlugin* fotofing_ ## _type ## _create() \
    { \
        return new _class(); \
    }


#endif
