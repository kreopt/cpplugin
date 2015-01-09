#include <iostream>
#include <unordered_map>
#include <functional>
#include <sstream>

#include "common.hpp"

#ifdef OS_WINDOWS
    #include <windows.h>
    #define get_proc_address GetProcAddress
#else
    #include <dlfcn.h>
    #define get_proc_address dlsym
#endif

const std::string PLUGIN_PATH = "";

namespace log {
    auto &debug = std::cerr;
    auto &error = std::cerr;
    auto &info  = std::cout;
}

struct plugin_interface{
#ifdef OS_WINDOWS
    HMODULE handle;
#else
    void *handle;
#endif

    std::function<void(const arglist_t&)> init;
    std::function<void(const std::string&, const arglist_t&)> exec;

    plugin_exports exports;
};

typedef void(*exports_func_sig)(decltype(plugin_interface::exports)&);
using exports_func_t = std::function<void(decltype(plugin_interface::exports)&)>;

typedef void(*init_func_sig)(const arglist_t&);
typedef void(*exec_func_sig)(const std::string&, const arglist_t&);


std::unordered_map<std::string, plugin_interface> plugin_registry;

std::string get_error_string(){
#ifdef OS_WINDOWS
        LPTSTR lpErrorText = NULL;
        auto dwErrorCode = GetLastError();

        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            0, dwErrorCode, 0, lpErrorText, MAX_PATH, 0 );

        std::string reason;
        if (lpErrorText){
            reason = lpErrorText;
        }
        LocalFree( lpErrorText );
#else
        std::string reason(dlerror());
#endif
        return reason;
}

// TODO: resolve name
inline const char* get_plugin_filename(const std::string &_name){
    auto ext =
#ifdef OS_WINDOWS
    ".dll";
#else
    ".so";
#endif
    return (PLUGIN_PATH+"lib"+_name+ext).c_str();}

plugin_interface load_plugin(const char* _name){
    std::string name(_name);
    if (plugin_registry.count(name) > 0) {
        log::info << "plugin " << name << " is already loaded" << std::endl;
        return plugin_registry.at(name);
    }
    log::debug << "loading plugin " << name << "..." << std::endl;

    plugin_interface plugin;
    auto plugin_fname = get_plugin_filename(name.c_str());
#ifdef OS_WINDOWS
    plugin.handle = LoadLibrary(plugin_fname);
#else
    plugin.handle = dlopen(plugin_fname, RTLD_LAZY);
#endif
    if (!plugin.handle) {
        std::stringstream ss;
        ss << "failed to load plugin " << name << "! " << std::endl << get_error_string();
        throw std::runtime_error(ss.str());
    }

    exports_func_t get_exports = (exports_func_sig)get_proc_address(plugin.handle, "get_exports");

    if (!get_exports) {
        std::stringstream ss;
        ss << "failed to get exports for plugin " << name << "! " << std::endl << get_error_string();
        throw std::runtime_error(ss.str());
    }

    get_exports(plugin.exports);

    plugin.init = (init_func_sig)get_proc_address(plugin.handle, "init");

    if (!plugin.init) {
        std::stringstream ss;
        ss << "failed to get initializer for plugin " << name << "! " << std::endl << get_error_string();
        throw std::runtime_error(ss.str());
    }

    plugin.exec = (exec_func_sig)get_proc_address(plugin.handle, "exec");

    if (!plugin.exec) {
        std::stringstream ss;
        ss << "failed to get entry for plugin " << name << "! " << std::endl << get_error_string();
        throw std::runtime_error(ss.str());
    }

    plugin_registry.insert(std::make_pair(name, plugin));
    log::debug << "plugin " << name << " loaded. version = " << plugin.exports.version << std::endl ;
    return plugin;
}

void free_plugin(const char* _name){
    std::string name(_name);
    plugin_interface plugin = plugin_registry.at(name);
    plugin_registry.erase(name);

#ifdef OS_WINDOWS
    FreeLibrary(plugin.handle);
#else
    dlclose(plugin.handle);
#endif
}

int main()
{
    try {
        auto plugin = load_plugin("dummy");
        plugin.init(arglist_t());
        plugin.exec("test", arglist_t());
        free_plugin("dummy");
    } catch (std::runtime_error &e){
        log::error << e.what() << std::endl;
    }

    return 0;
}

