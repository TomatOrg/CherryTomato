#pragma once

#include "printf.h"

#include <intrin.h>

typedef enum log_level {
    /**
     * Detailed information, only for actual debugging sessions,
     * the code should not have any such logs in it
     */
    LOG_LEVEL_DEBUG,

    /**
     * Detailed information, typically only of interest to a
     * developer trying to diagnose a problem
     */
    LOG_LEVEL_TRACE,

    /**
     * Confirmation that things are working as expected.
     */
    LOG_LEVEL_INFO,

    /**
     * An indication that something unexpected happened
     * or that a problem might occur in the near future.
     * The software is still working as expected.
     */
    LOG_LEVEL_WARNING,

    /**
     * Due to a more serious problem, the software has not
     * been able to perform some function.
     */
    LOG_LEVEL_ERROR,

    /**
     * A serious error, indicating that the program itself may
     * be unable to continue running.
     */
    LOG_LEVEL_CRITICAL,
} log_level_t;

#define __MIN_LOG_LEVEL__ LOG_LEVEL_DEBUG

// even if platform doesn't have uart we want to call this function in order
// to have the format string be checked, so when uart is not available
// we will just wrap it with an if(0)
#ifdef __HAS_CONSOLE__
    #define __LOG_CONSOLE(fmt, ...) \
        do { \
            disable_interrupts(); \
            printf_(fmt, ##__VA_ARGS__); \
            enable_interrupts(); \
        } while (0)
#else
    #define __LOG_CONSOLE(fmt, ...) \
        do { \
            if (0) \
                printf_(fmt, ##__VA_ARGS__); \
        } while (0)
#endif

#define __LOG_CONSOLE_DEBUG(fmt, ...)      __LOG_CONSOLE("[?] " fmt "\n", ##__VA_ARGS__)
#define __LOG_CONSOLE_TRACE(fmt, ...)      __LOG_CONSOLE("[*] " fmt "\n", ##__VA_ARGS__)
#define __LOG_CONSOLE_INFO(fmt, ...)       __LOG_CONSOLE("[+] " fmt "\n", ##__VA_ARGS__)
#define __LOG_CONSOLE_WARNING(fmt, ...)    __LOG_CONSOLE("[!] " fmt "\n", ##__VA_ARGS__)
#define __LOG_CONSOLE_ERROR(fmt, ...)      __LOG_CONSOLE("[-] " fmt "\n", ##__VA_ARGS__)
#define __LOG_CONSOLE_CRITICAL(fmt, ...)   __LOG_CONSOLE("[~] " fmt "\n", ##__VA_ARGS__)

// TODO: add log-id for more compact logging on uart-less devices and low memory devices
#define __LOG(LEVEL, FMT, ...) \
    do { \
        if (LOG_LEVEL_##LEVEL >= __MIN_LOG_LEVEL__) { \
            __LOG_CONSOLE_##LEVEL(FMT, ##__VA_ARGS__); \
        } \
    } while (0)

//----------------------------------------------------------------------------------------------------------------------
// High-level logging APIs
//----------------------------------------------------------------------------------------------------------------------

#define LOG_DEBUG(fmt, ...)         __LOG(DEBUG, fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...)         __LOG(TRACE, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)          __LOG(INFO, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)       __LOG(WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)         __LOG(ERROR, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...)      __LOG(CRITICAL, fmt, ##__VA_ARGS__)
