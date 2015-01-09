#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <cstdint>
#if defined(WIN32) || defined(_WIN32)
#define OS_WINDOWS
#define DLLEXPORT __declspec(dllexport)
//__declspec(dllimport)
#else
#define OS_UNIX
#define DLLEXPORT
#endif

using arglist_t = std::unordered_map<std::string, void*>;
using signature_t = std::string;
using signature_map_t = std::unordered_map<std::string, signature_t>;

struct plugin_exports {
    uint32_t version;
    signature_map_t signals;  // name->signature;
    signature_map_t handlers; // name->signature;
};
#endif
