
#pragma once

#if defined(__cplusplus)
#undef _DEBUG
#include "sys_path.hpp"
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <cfgmgr32.h>
#endif

namespace vkld
{
    class Manifest
    {
    public:
        enum class Type
        {
            Driver,
            ExplicitLayer,
            ImplicitLayer,
        };

        static std::vector<Manifest> Search();

    private:

#if defined(_WIN32)
        inline static bool IsWowProcess() { BOOL is_wow; IsWow64Process(GetCurrentProcess(), &is_wow); return is_wow; }
        static std::vector<SysPath> ReadDeviceRegistry(DEVINST device_id, Type type);
        static std::vector<SysPath> SearchPnpRegistry(Type type);
        static std::vector<SysPath> SearchSoftwareRegistry(Type type);
#endif
    };
}
#endif

#if defined(__cplusplus)
#define VKLD_C_API extern "C"
#else
#define VKLD_C_API
#endif

VKLD_C_API void CppTest();
