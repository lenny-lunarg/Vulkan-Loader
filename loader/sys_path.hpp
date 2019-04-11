
#pragma once

#undef _DEBUG
#include <string>
#include <vector>

namespace vkld
{
    class SysPath
    {
    public:
        SysPath(const std::string& raw_path);

        std::string String() const;

    private:
        std::vector<std::string> path;

#if defined(_WIN32)
        static constexpr char* delimiter = "\\";
#else
        static constexpr char* delimiter = "/";
#endif
    };
}
