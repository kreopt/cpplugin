#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>

#include "common.hpp"

class plugin {
    std::unordered_map<std::string, std::function<void(const arglist_t&)>> handlers;
    plugin() = delete;
public:
    // TODO: remove from public
    plugin(const arglist_t& _args){}
    inline static std::shared_ptr<plugin> instance(const arglist_t& _args){
        static std::shared_ptr<plugin> instance;
        if (!instance) {
            instance = std::make_shared<plugin>(_args);
        }
        return instance;
    }
};

extern "C" {
    void DLLEXPORT get_exports(plugin_exports &_exports){
        std::cout << "getting exports..." << std::endl;
        _exports.version = 1;
    }

    void DLLEXPORT init(const arglist_t& _args){
        plugin::instance(_args);
        std::cout << "hello from init" << std::endl;
    }

    void DLLEXPORT exec(const std::string& _name, const arglist_t& _args){
        std::cout << "execute " << _name << std::endl;
    }
}
