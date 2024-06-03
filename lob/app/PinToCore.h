#pragma once

#include <string>
#include <stdexcept>

#ifdef _WIN32

#include <windows.h>

inline void pin_to_core(DWORD coreId) {
    HANDLE thread = GetCurrentThread();
    DWORD_PTR mask = 1 << coreId;
    DWORD_PTR result = SetThreadAffinityMask(thread, mask);
    if (result == 0) 
        throw std::runtime_error(std::string() + "Failed to set thread affinity. Last error code: " + std::to_string(GetLastError()));
}

#else

#include <pthread.h>

inline void pin_to_core(int coreId) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);

    pthread_t currentThread = pthread_self();
    int result = pthread_setaffinity_np(currentThread, sizeof(cpu_set_t), &cpuset);
    if (result != 0) 
        throw std::runtime_error(std::string() + "Failed to set thread affinity. result: " + std::to_string(result));
}

#endif
