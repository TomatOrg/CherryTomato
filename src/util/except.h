#pragma once

#include "log.h"

typedef enum err {
    NO_ERROR,

    // generic errors
    ERROR_CHECK_FAILED,
} err_t;

#define IS_ERROR(err) ((err) != NO_ERROR)

#define CHECK_ERROR(expr, error) \
    do { \
        if (!(expr)) { \
            err = error; \
            LOG_ERROR("Check failed at %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__); \
            goto cleanup; \
        } \
    } while (0)

#define CHECK(expr) CHECK_ERROR(expr, ERROR_CHECK_FAILED)

#define CHECK_FAIL_ERROR(error) CHECK_ERROR(0, error)
#define CHECK_FAIL() CHECK_FAIL_ERROR(ERROR_CHECK_FAILED)

#define CHECK_AND_RETHROW(error) \
    do { \
        err = error; \
        if (IS_ERROR(err)) { \
            LOG_ERROR("\trethrown at %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__); \
            goto cleanup; \
        } \
    } while (0)

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            LOG_ERROR("Assert `" #expr "` failed!"); \
        } \
    } while(0)
