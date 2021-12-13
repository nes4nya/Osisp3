#pragma once

#ifdef STRINGREPLACERLIBRARY_EXPORTS
#define STRINGREPLACERLIBRARY_API __declspec(dllexport)
#else
#define STRINGREPLACERLIBRARY_API __declspec(dllimport)
#endif //STRINGREPLACERLIBRARY_EXPORTS

extern "C" STRINGREPLACERLIBRARY_API void ReplaceString(const char* source, const char* target);