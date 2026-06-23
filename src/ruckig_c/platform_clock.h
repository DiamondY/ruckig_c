#ifndef RUCKIG_C_PLATFORM_CLOCK_H
#define RUCKIG_C_PLATFORM_CLOCK_H

#if !defined(RUCKIG_C_CUSTOM_MONOTONIC_TIME_US) && !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdint.h>

#if defined(RUCKIG_C_PLATFORM_CLOCK_HEADER)
#include RUCKIG_C_PLATFORM_CLOCK_HEADER
#endif

#if !defined(RUCKIG_C_CUSTOM_MONOTONIC_TIME_US)
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <time.h>
#if !defined(CLOCK_MONOTONIC)
#error "Ruckig C requires a monotonic platform clock; define RUCKIG_C_PLATFORM_CLOCK_HEADER and RUCKIG_C_CUSTOM_MONOTONIC_TIME_US for this platform."
#endif
#endif
#endif

static uint64_t ruckig_platform_monotonic_time_us(void) {
#if defined(RUCKIG_C_CUSTOM_MONOTONIC_TIME_US)
    return (uint64_t)(RUCKIG_C_CUSTOM_MONOTONIC_TIME_US());
#elif defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency) && frequency.QuadPart > 0
        && QueryPerformanceCounter(&counter)) {
        const uint64_t ticks = (uint64_t)counter.QuadPart;
        const uint64_t ticks_per_second = (uint64_t)frequency.QuadPart;
        return (ticks / ticks_per_second) * 1000000u
            + ((ticks % ticks_per_second) * 1000000u) / ticks_per_second;
    }
    return 0u;
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint64_t)ts.tv_sec * 1000000u + (uint64_t)(ts.tv_nsec / 1000);
    }
    return 0u;
#endif
}

#endif
