#pragma once
#include <cstdlib>
#include <cstring>

// TODO(HO): Custom Allocators/Arenas

inline void* EmberMemoryAllocate(size_t Size)
{
    return malloc(Size);
}

inline void EmberMemoryZero(void* Dest, size_t Size)
{
    memset(Dest, 0, Size);
}

template<typename T, bool ZeroMem = false>
T* EmberMemoryAllocateType(size_t Count = 1)
{
    size_t Size = sizeof(T) * Count;
    T* Result = (T*)EmberMemoryAllocate(Size);
    if constexpr (ZeroMem)
    {
        EmberMemoryZero(Result, Size);
    }

    return Result;
}

inline void EmberMemoryFree(void* Block)
{
    if(Block)
    {
        free(Block);
    }
}

inline void* EmberMemoryCopy(void* Destination, void const* Source, size_t Size)
{
    return memcpy(Destination, Source, Size);
}


