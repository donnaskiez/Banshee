#pragma once

#include <ntifs.h>
#include <wdf.h>
#include "Misc.hpp"
#include "WinTypes.hpp"

/**
 * Get offset to the access token from the EPROCESS structure, depending on the OS version.
 * Taken from https://github.com/Idov31/Nidhogg/blob/2776908e86c34771d0663e931b1930c64a9d4b15/Nidhogg/WindowsTypes.hpp
 *
 * @return ULONG Offset to Acess Token.
 */
ULONG
BeGetAccessTokenOffset()
{
    RTL_OSVERSIONINFOW osVersion = { sizeof(osVersion) };
    RtlGetVersion(&osVersion);

    DbgPrint("Running on %i", osVersion.dwBuildNumber);

    ULONG tokenOffset = 0;

    switch (osVersion.dwBuildNumber)
    {
    case WIN_1903:
    case WIN_1909:
        tokenOffset = 0x360;
        break;
    case WIN_1507:
    case WIN_1511:
    case WIN_1607:
    case WIN_1703:
    case WIN_1709:
    case WIN_1803:
    case WIN_1809:
        tokenOffset = 0x358;
        break;
    default:
        tokenOffset = 0x4b8;
        break;
    }

    DbgPrint("Token offset: %i", tokenOffset);
    return tokenOffset;
}

/**
 * Get offset to the process list from the EPROCESS structure, depending on the OS version.
 * Taken from https://github.com/Idov31/Nidhogg/blob/2776908e86c34771d0663e931b1930c64a9d4b15/Nidhogg/WindowsTypes.hpp
 *
 * @return ULONG Offset to Process List.
 */
ULONG
BeGetProcessLinkedListOffset()
{
    RTL_OSVERSIONINFOW osVersion = { sizeof(osVersion) };
    RtlGetVersion(&osVersion);

    DbgPrint("Running on %i", osVersion.dwBuildNumber);

    ULONG activeProcessLinks = 0;

    switch (osVersion.dwBuildNumber)
    {
    case WIN_1507:
    case WIN_1511:
    case WIN_1607:
    case WIN_1903:
    case WIN_1909:
        activeProcessLinks = 0x2f0;
        break;
    case WIN_1703:
    case WIN_1709:
    case WIN_1803:
    case WIN_1809:
        activeProcessLinks = 0x2e8;
        break;
    default:
        activeProcessLinks = 0x448;
        break;
    }

    return activeProcessLinks;
}

/*
 * Get the base address of ntoskrnl.exe
 * https://www.unknowncheats.me/forum/general-programming-and-reversing/427419-getkernelbase.html
 *
 * @returns PVOID address of ntoskrnl.exe
 */
PVOID
BeGetKernelBaseAddr(IN PDRIVER_OBJECT DriverObject)
{
    PKLDR_DATA_TABLE_ENTRY entry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
    PKLDR_DATA_TABLE_ENTRY first = entry;

    while ((PKLDR_DATA_TABLE_ENTRY)entry->InLoadOrderLinks.Flink != first)
    {
        if (_strcmpi_w(entry->BaseDllName.Buffer, L"ntoskrnl.exe") == 0)
        {
            return entry->DllBase;
        }
        entry = (PKLDR_DATA_TABLE_ENTRY)entry->InLoadOrderLinks.Flink;
    }
    return NULL;
}

/*
 * Gets offset of EPROCESS ProcessProtection dynamically by parsing PsIsProtectedProcessLight.
 * Shoutout to @never_unsealed and @C5Pider for pointing this out to me.
 *
 * @returns ULONG Offset of EPROCESS ProcessProtection
 */
UINT16
BeGetEprocessProcessProtectionOffset()
{
    UNICODE_STRING psIsPpl = RTL_CONSTANT_STRING(L"PsIsProtectedProcessLight");
    return (UINT16)(*((PUINT16)MmGetSystemRoutineAddress(&psIsPpl) + 0x1));
}

// TODO: implement MmGetSystemRoutineAddress � la HellsGate with ntoskrnl.exe