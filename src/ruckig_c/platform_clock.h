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
#include <time.h>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#endif

#if !defined(RUCKIG_C_CUSTOM_MONOTONIC_TIME_US)
static uint64_t ruckig_platform_clock_fallback_us(void) {
    const clock_t value = clock();
    if (value == (clock_t)-1) {
        return 0u;
    }
    return (uint64_t)(((double)value * 1000000.0) / (double)CLOCKS_PER_SEC);
}
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
    return ruckig_platform_clock_fallback_us();
#else
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint64_t)ts.tv_sec * 1000000u + (uint64_t)(ts.tv_nsec / 1000);
    }
#endif
    return ruckig_platform_clock_fallback_us();
#endif
}

#endif
