
#if defined(_WIN32) && !defined(__MINGW64__)
#include <windows.h>
#else
#include <pthread.h>
#endif
#include "ThreadUtils.h"

using namespace std;

namespace SysUtils
{
void SetThreadName(thread& t, const string& name)
{
#if defined(_WIN32) && !defined(__MINGW64__)
    DWORD threadId = ::GetThreadId(static_cast<HANDLE>(t.native_handle()));
    SetThreadName(threadId, name.c_str());
#elif defined(__APPLE__)
    /*
    // Apple pthread_setname_np only set current thread name and 
    // No other API can set thread name from other thread
    if (name.length() > 15)
    {
        string shortName = name.substr(0, 15);
        pthread_setname_np(shortName.c_str());
    }
    else
    {
        pthread_setname_np(name.c_str());
    }
    */
#else
    auto handle = t.native_handle();
    if (name.length() > 15)
    {
        string shortName = name.substr(0, 15);
        pthread_setname_np(handle, shortName.c_str());
    }
    else
    {
        pthread_setname_np(handle, name.c_str());
    }
#endif
}
}
