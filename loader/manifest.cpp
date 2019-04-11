
#include "manifest.h"

#include <array>
#include <unordered_map>

std::vector<vkld::Manifest> vkld::Manifest::Search()
{
    //SearchAdapters(Type::ImplicitLayer);
    std::vector<SysPath> pnp_paths = SearchPnpRegistry(Type::ImplicitLayer);
    std::vector<SysPath> sft_paths = SearchSoftwareRegistry(Type::ImplicitLayer);
    return std::vector<Manifest>();
}

#if defined(_WIN32)
std::vector<vkld::SysPath> vkld::Manifest::ReadDeviceRegistry(DEVINST device_id, Type type)
{
    const static std::unordered_map<Type, const char*> subkeys = {
        { Type::Driver, IsWowProcess() ? "VulkanDriverNameWow" : "VulkanDriverName" },
        { Type::ExplicitLayer, IsWowProcess() ? "VulkanExplicitLayersWow" : "VulkanExplicitLayers" },
        { Type::ImplicitLayer, IsWowProcess() ? "VulkanImplicitLayersWow" : "VulkanImplicitLayers" },
    };

    HKEY key;
    CONFIGRET status = CM_Open_DevNode_Key(device_id, KEY_QUERY_VALUE, 0, RegDisposition_OpenExisting, &key, CM_REGISTRY_SOFTWARE);
    if (status != CR_SUCCESS) {
        return std::vector<SysPath>();
    }

    DWORD value_size;
    LSTATUS lstatus = RegQueryValueEx(key, "VulkanImplicitLayers", nullptr, nullptr, nullptr, &value_size);
    if (lstatus != ERROR_SUCCESS) {
        if (lstatus == ERROR_FILE_NOT_FOUND) {
            // Message (Debug): Device ID {} does not contain a {} value
        } else {
            // Message (Debug): Could not obtain size of {} value for device ID {}
        }
        return std::vector<SysPath>();
    }

    std::string value;
    value.resize(value_size);

    DWORD value_type;
    lstatus = RegQueryValueEx(key, "VulkanImplicitLayers", nullptr, &value_type, (BYTE*) &value.front(), &value_size);
    if (lstatus != ERROR_SUCCESS) {
        // Message (Error): Failed to obtain {} value for device ID {}
        return std::vector<SysPath>();
    } else if (value_type != REG_SZ && value_type != REG_MULTI_SZ) {
        // Message (Error): The {} value for device ID {} is invalid. It must be a REG_SZ or REG_MULTI_SZ
        return std::vector<SysPath>();
    }

    std::vector<SysPath> paths;
    for (char *path = &value.front(); *path != '\0'; path += strlen(path) + 1) {
        paths.push_back(SysPath(path));
        if (value_type == REG_SZ) {
            break;
        }
    }

    return paths;
}

std::vector<vkld::SysPath> vkld::Manifest::SearchPnpRegistry(Type type)
{
    static const wchar_t *component_guid = L"{5c4c3332-344d-483c-8739-259e934c9cc8}";
    static const wchar_t *display_guid = L"{4d36e968-e325-11ce-bfc1-08002be10318}";
    static const ULONG flags = CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT;

    std::wstring device_names;
    do {
        ULONG names_size;
        CM_Get_Device_ID_List_SizeW(&names_size, display_guid, flags);
        device_names.resize(names_size);
    } while (CM_Get_Device_ID_ListW(display_guid, &device_names.front(), device_names.size(), flags) == CR_BUFFER_SMALL);

    if (device_names.empty()) {
        return std::vector<SysPath>();
    }

    for (wchar_t *device = &device_names.front(); *device != '\0'; device += wcslen(device) + 1) {
        DEVINST device_id;
        CONFIGRET status = CM_Locate_DevNodeW(&device_id, device, CM_LOCATE_DEVNODE_NORMAL);
        if (status != CR_SUCCESS) {
            // Message (Error): Failed to open device node {}
            continue;
        }

        ULONG ul_status, ul_problem;
        status = CM_Get_DevNode_Status(&ul_status, &ul_problem, device_id, 0);
        if (status != CR_SUCCESS) {
            // Message (Error): Failed to probe device status on {}
            continue;
        }
        if ((ul_status & DN_HAS_PROBLEM) && (ul_problem == CM_PROB_NEED_RESTART || ul_problem == DN_NEED_RESTART)) {
            // Message (Info): Device {} is pending a reboot. It will be skipped
            continue;
        }

        // Message (Info): Opening device {} to find drivers

        // call into loaderGetDeviceRegistryEntry and continue if found
        std::vector<SysPath> paths = ReadDeviceRegistry(device_id, type);
        if (!paths.empty()) {
            return paths;
        }

        status = CM_Get_Child(&device_id, device_id, 0);
        if (status != CR_SUCCESS) {
            // Message (Debug): Failed to open child device for {}
            continue;
        }

        do {
            wchar_t buffer[MAX_DEVICE_ID_LEN];
            CM_Get_Device_IDW(device_id, buffer, MAX_DEVICE_ID_LEN, 0);

            // Message (Info): Opening child device {} to find drivers

            wchar_t guid[MAX_GUID_STRING_LEN + 2]; // Add two to account for brackets
            ULONG guid_size = sizeof(guid);
            status = CM_Get_DevNode_Registry_PropertyW(device_id, CM_DRP_CLASSGUID, nullptr, &guid, &guid_size, 0);
            if (status != CR_SUCCESS) {
                // Message (Error): Unable to read GUID for child device {}
                continue;
            }
            if (wcscmp(guid, component_guid) != 0) {
                // Message (Debug): Child device {} is not a software component. It will be skipped
                continue;
            }

            // call into loaderGetDeviceRegistryEntry and exit if it was found
            std::vector<SysPath> paths = ReadDeviceRegistry(device_id, type);
            if (!paths.empty()) {
                return paths;
            }

        } while (CM_Get_Sibling(&device_id, device_id, 0) == CR_SUCCESS);
    }

    return std::vector<SysPath>();
}

std::vector<vkld::SysPath> vkld::Manifest::SearchSoftwareRegistry(Type type)
{
    const static std::array<HKEY, 2> hives = {
        HKEY_LOCAL_MACHINE,
        HKEY_CURRENT_USER
    };
    const static std::unordered_map<Type, const char*> subkeys = {
        { Type::Driver, "SOFTWARE\\Khronos\\Vulkan\\Drivers" },
        { Type::ExplicitLayer, "SOFTWARE\\Khronos\\Vulkan\\ExplicitLayers" },
        { Type::ImplicitLayer, "SOFTWARE\\Khronos\\Vulkan\\ImplicitLayers" },
    };

    std::vector<SysPath> manifest_paths;
    for (HKEY hive : hives) {
        HKEY key;
        LSTATUS status = RegOpenKeyEx(hive, subkeys.at(type), 0, KEY_QUERY_VALUE, &key);
        if (status != ERROR_SUCCESS) {
            // Message (Debug): Failed to open hive {} with code {}
            continue;
        }

        for (DWORD i = 0; ; ++i) {
            // A registry value is (as of 2019) limited to 16383 chars. Therefore, this is always sufficient
            char name[16384];
            DWORD name_size = sizeof(name);
            DWORD type;
            DWORD value;
            DWORD value_size = sizeof(value);
            status = RegEnumValue(key, i, name, &name_size, nullptr, &type, (LPBYTE) &value, &value_size);
            if (status == ERROR_NO_MORE_ITEMS) {
                break;
            }

            if (status == ERROR_SUCCESS && type == REG_DWORD && value == 0) {
                // Message (Debug): Found manifest file {} from registry {}\\{}
                manifest_paths.push_back(SysPath(name));
            }
        }

        RegCloseKey(key);
    }

    return manifest_paths;
}
#endif

void CppTest()
{
    vkld::SysPath path("C:\\\\\\Users\\Lenny//////Documents/Demo/../Idk/../..\\\\.");
    std::string repr = path.String();

    vkld::Manifest::Search();
    printf("Break\n");
}
