#include <iostream>
#include "StringReplacerLibrary.h"
#include <windows.h>
#include <TlHelp32.h>

#define PATH_TO_DLL "StringReplacerLibrary.dll"
#define FUNCTION_NAME "ReplaceString"

#define PATH_TO_INJECT_DLL "injectableLibrary.dll"
#define PATH_TO_KERNEL_DLL "Kernel32.dll"
#define NAME_LOAD_LIBRARY_FUNCTION "LoadLibraryA"

typedef HMODULE(WINAPI* LPLoadLibrary)(LPCSTR);
typedef void ReplaceData(const char* source, const char* target);

typedef struct
{
    const char* source;
    const char* target;
} TStringReplace;


void ReplaceStringDynamic(const char* source, const char* target)
{
    HMODULE moduleDll = LoadLibraryA(PATH_TO_DLL);
    if (moduleDll != NULL)
    {
        ReplaceData* ReplaceStringFunc = (ReplaceData*)GetProcAddress(moduleDll, FUNCTION_NAME);

        if (ReplaceStringFunc != NULL)
        {
            ReplaceStringFunc(source, target);
        }
        else
        {
            std::string error_message(FUNCTION_NAME);
            std::cout << "[ERROR] Function with name: " + error_message + " was not found" + "\n";
        }

        FreeLibrary(moduleDll);
    }
    else
    {
        std::string error_message(PATH_TO_DLL);
        std::cout << "[ERROR] Cannot load DLL from: " + error_message + "\n";
    }
}

void InjectLibrary(DWORD pid, LPCVOID param, DWORD paramSize, const char* dllName, const char* functionName)
{
    HANDLE hProc = OpenProcess(PROCESS_CREATE_THREAD |
        PROCESS_VM_OPERATION |
        PROCESS_VM_WRITE, FALSE, pid);
    if (hProc)
    {
        PCSTR baseAddress = (PCSTR)VirtualAllocEx(hProc, NULL, paramSize, MEM_COMMIT, PAGE_READWRITE);
        if (baseAddress)
        {
            if (WriteProcessMemory(hProc, (LPVOID)baseAddress, param, paramSize, NULL))
            {
                PTHREAD_START_ROUTINE startRoutine = (PTHREAD_START_ROUTINE)GetProcAddress(LoadLibraryA(dllName), functionName);
                if (startRoutine)
                {
                    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (PTHREAD_START_ROUTINE)startRoutine, (LPVOID)baseAddress, 0, NULL);

                    if (hThread)
                    {
                        WaitForSingleObject(hThread, INFINITE);
                    }
                }
            }
        }
    }

}

int main()
{
    bool exit = false;

    while (!exit)
    {
        std::cout << "Menu:" << "\n";
        std::cout << "1  Static import" << "\n";
        std::cout << "2  Dynamic import" << "\n";
        std::cout << "3  Inject DLL" << "\n";
        std::cout << "0  Exit" << "\n";

        int menu;
        std::cin >> menu;

        const char* source = "Old example String";
        const char* target = "New example String";

        char* buf = new char[strlen(source) + 1];
        strcpy_s(buf, strlen(source) + 1, source);

        switch (menu)
        {
        case 0:
            exit = true;
            break;
        case 1:
            std::cout << "Value before function call: " + std::string(buf) << "\n";
            ReplaceString(source, target);
            std::cout << "Value  after function call: " + std::string(buf) << "\n";
            break;
        case 2:
            std::cout << "Value before function call: " + std::string(buf) << "\n";
            ReplaceStringDynamic(source, target);
            std::cout << "Value  after function call: " + std::string(buf) << "\n";
            break;
        case 3:
        {
            std::cout << "Value before function call: " + std::string(buf) << "\n";

            DWORD pid = GetCurrentProcessId();
            int paramSize = (strlen(PATH_TO_INJECT_DLL) + 1) * sizeof(CHAR);

            TStringReplace stringReplace;
            stringReplace.source = source;
            stringReplace.target = target;

            InjectLibrary(pid, PATH_TO_INJECT_DLL, paramSize, PATH_TO_KERNEL_DLL, NAME_LOAD_LIBRARY_FUNCTION);
            InjectLibrary(pid, &stringReplace, sizeof(stringReplace), PATH_TO_INJECT_DLL, FUNCTION_NAME);

            std::cout << "Value  after function call: " + std::string(buf) << "\n";
            break;
        }
        default:
            std::cout << "Inccorect number" << "\n";
            break;
        }

        std::cout << "\n";
    }
}