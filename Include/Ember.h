#pragma once
#include <cstdint>

// TODO(HO): Configure based on actual Platform, once we add more. SDL is the initial Platform Layer for now.
#define PLATFORM_SDL
#define RENDERER_VULKAN

// Type definitions for signed integers
typedef int8_t     s8;
typedef int16_t    s16;
typedef int32_t    s32;
typedef int64_t    s64;

// Type definitions for unsigned integers
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

// Macro to explicitly mark an argument as unused to avoid compiler warnings
#define UNUSED_ARG(x) (void)(x)

#define DEBUG_BREAK() __debugbreak();
#define EMBER_ASSERT(condition) \
    do { \
        if(!(condition)) { \
            EMBER_LOG(ELogCategory::Error, "Assertion Failed: (%s), Function: (%s), File (%s), Line (%i)", #condition, __FUNCTION__, __FILE__, __LINE__); \
            DEBUG_BREAK(); \
        } \
    } while (false);

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

