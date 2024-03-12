#pragma once
#include <cstdlib>
#include <cstring>

// TODO(HO): Custom Allocators/Arenas

inline void* EmberMemoryAllocate(size_t Size)
{
    return malloc(Size);
}

template<typename T>
T* EmberMemoryAllocateType(size_t Count = 1)
{
    return (T*)EmberMemoryAllocate(sizeof(T) * Count);
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

inline void EmberMemoryZero(void* Dest, size_t Size)
{
    memset(Dest, 0, Size);
}
