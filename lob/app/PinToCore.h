#pragma once

#include <stdexcept>
#include <string>

#ifdef _WIN32

#include <windows.h>
#undef min
#undef max
#undef ERROR

template <DWORD CoreId>
inline void pin_to_core() {
  HANDLE thread = GetCurrentThread();
  DWORD_PTR mask = 1 << CoreId;
  DWORD_PTR result = SetThreadAffinityMask(thread, mask);
  if (result == 0)
    throw std::runtime_error(std::string() + "Failed to set thread affinity. Last error code: " + std::to_string(GetLastError()));
}

#else

#include <pthread.h>

template <int CoreId>
inline void pin_to_core() {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(CoreId, &cpuset);

  pthread_t currentThread = pthread_self();
  int result = pthread_setaffinity_np(currentThread, sizeof(cpu_set_t), &cpuset);
  if (result != 0)
    throw std::runtime_error(std::string() + "Failed to set thread affinity. result: " + std::to_string(result));
}

#endif
