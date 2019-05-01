/*
 * This header file redefines APIs that are provided in Microsoft's WDK. This is
 * explicitly allowed by Microsoft so that non-DX APIs do not need to require a
 * WDK. Per Microsoft Documentation, any redefined APIs must change the name to
 * avoid any possible header conflicts. This is the reason for putting a Vulkan
 * prefix before everything. The nonstandard VULKAN_ prefix is used because the
 * more standard VK_ and Vk prefixes would imply that this functionality is a
 * part of Vulkan, which it is not.
 *
 * See: <insert a link here>
 */

#pragma once

#include <windows.h>

// KMT APIs

typedef UINT VULKAN_D3DKMT_HANDLE;

typedef struct _VULKAN_D3DKMT_ADAPTERINFO {
    VULKAN_D3DKMT_HANDLE hAdapter;
    LUID AdapterLuid;
    ULONG NumOfSources;
    BOOL bPresentMoveRegionsPreferred;
} VULKAN_D3DKMT_ADAPTERINFO;

typedef struct _VULKAN_D3DKMT_ENUMADAPTERS2 {
    ULONG NumAdapters;
    VULKAN_D3DKMT_ADAPTERINFO* pAdapters;
} VULKAN_D3DKMT_ENUMADAPTERS2;

typedef NTSTATUS (*PFN_VulkanD3DKMTEnumAdapters2)(const VULKAN_D3DKMT_ENUMADAPTERS2 *Arg1);

// This enum is VERY incomplete. This only includes the values actually used by the loader
typedef enum _VULKAN_KMTQUERYADAPTERINFOTYPE
{
    VULKAN_KMTQAITYPE_QUERYREGISTRY = 48,
} VULKAN_KMTQUERYADAPTERINFOTYPE;

typedef struct _VULKAN_D3DKMT_QUERYADAPTERINFO {
    VULKAN_D3DKMT_HANDLE hAdapter;
    VULKAN_KMTQUERYADAPTERINFOTYPE Type;
    VOID *pPrivateDriverData;
    UINT PrivateDriverDataSize;
} VULKAN_D3DKMT_QUERYADAPTERINFO;

typedef NTSTATUS (*PFN_VulkanD3DKMTQueryAdapterInfo)(const VULKAN_D3DKMT_QUERYADAPTERINFO *Arg1);

// DDI APIs

typedef enum _VULKAN_D3DDDI_QUERYREGISTRY_TYPE
{
    VULKAN_D3DDDI_QUERYREGISTRY_SERVICEKEY = 0,
    VULKAN_D3DDDI_QUERYREGISTRY_ADAPTERKEY = 1,
    VULKAN_D3DDDI_QUERYREGISTRY_DRIVERSTOREPATH = 2,
    VULKAN_D3DDDI_QUERYREGISTRY_MAX,
} VULKAN_D3DDDI_QUERYREGISTRY_TYPE;

typedef enum _VULKAN_D3DDDI_QUERYREGISTRY_STATUS
{
    VULKAN_D3DDDI_QUERYREGISTRY_STATUS_SUCCESS = 0,
    VULKAN_D3DDDI_QUERYREGISTRY_STATUS_BUFFER_OVERFLOW      = 1,
    VULKAN_D3DDDI_QUERYREGISTRY_STATUS_FAIL                 = 2,
    VULKAN_D3DDDI_QUERYREGISTRY_STATUS_MAX,
} VULKAN_D3DDDI_QUERYREGISTRY_STATUS;

typedef struct _VULKAN_D3DDDI_QUERYREGISTRY_FLAGS {
    union {
        struct {
            UINT TranslatePath : 1;
            UINT MutableValue : 1;
            UINT Reserved : 30;
        };
        UINT Value;
    };
} VULKAN_D3DDDI_QUERYREGISTRY_FLAGS;

typedef struct _VULKAN_D3DDDI_QUERYREGISTRY_INFO {
    VULKAN_D3DDDI_QUERYREGISTRY_TYPE QueryType;
    VULKAN_D3DDDI_QUERYREGISTRY_FLAGS QueryFlags;
    WCHAR ValueName[MAX_PATH];
    ULONG ValueType;
    ULONG PhysicalAdapterIndex;
    ULONG OutputValueSize;
    VULKAN_D3DDDI_QUERYREGISTRY_STATUS Status;
    union {
        DWORD OutputDword;
        UINT64 OutputQword;
        WCHAR OutputString[1];
        BYTE OutputBinary[1];
    };
} VULKAN_D3DDDI_QUERYREGISTRY_INFO;
