
#include "sys_path.hpp"

vkld::SysPath::SysPath(const std::string& raw_path)
{
    for (auto it = raw_path.begin(); it != raw_path.end(); ) {
        if (*it == '/' || *it == '\\') {
            ++it;
            continue;
        }

        std::string::const_iterator end;
        for (end = it; end != raw_path.end() && *end != '/' && *end != '\\'; ++end) { }
        std::string name(it, end);
        
        if (name == ".." && !path.empty()) {
            path.pop_back();
        } else if (name == "." && !path.empty()) {
            // Drop this item
        } else {
            path.push_back(name);
        }

        it = end;
    }
}

std::string vkld::SysPath::String() const
{
    std::string output;
    for (const std::string &item : path) {
        if (!output.empty()) {
            output += delimiter;
        }
        output += item;
    }
    return output;
}
