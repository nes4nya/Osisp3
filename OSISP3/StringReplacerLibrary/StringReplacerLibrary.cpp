#include "pch.h"
#include "StringReplacerLibrary.h"
#include <iostream>

void ReplaceString(const char* source, const char* target)
{
    SIZE_T sourceLen = strlen(source);
    SIZE_T targetLen = strlen(target);

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    MEMORY_BASIC_INFORMATION info;
    auto baseAddress = (LPSTR)si.lpMinimumApplicationAddress;

    while (baseAddress < si.lpMaximumApplicationAddress)
    {
        if (VirtualQuery(baseAddress, &info, sizeof(info)) == sizeof(info))
        {
            if (info.State == MEM_COMMIT && info.AllocationProtect == PAGE_READWRITE)
            {
                baseAddress = (LPSTR)info.BaseAddress;
                char* memory = (char*)malloc(info.RegionSize);
                SIZE_T bytesRead;

                if (memory != 0)
                {
                    if (ReadProcessMemory(GetCurrentProcess(), baseAddress, memory, info.RegionSize, &bytesRead))
                    {
                        for (SIZE_T i = 0; i < bytesRead - sourceLen; i++)
                        {
                            if (strcmp(baseAddress + i, source) == 0)
                            {
                                memcpy(baseAddress + i, target, targetLen + 1);
                            }
                        }
                    }
                    free(memory);
                }

            }
        }
        baseAddress += info.RegionSize;
    }
}