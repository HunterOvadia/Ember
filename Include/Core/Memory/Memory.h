#pragma once
#include <cstdlib>
#include <cstring>

// TODO(HO): Custom Allocators/Arenas
namespace Ember::Memory
{
    inline void* Allocate(size_t Size)
    {
        return malloc(Size);
    }

    template<typename T>
    T* AllocateType(size_t Count = 1)
    {
        return (T*)Allocate(sizeof(T) * Count);
    }

    inline void Free(void* Block)
    {
        free(Block);
    }

    inline void* Copy(void* Destination, void const* Source, size_t Size)
    {
        return memcpy(Destination, Source, Size);
    }

    inline void MemZero(void* Dest, size_t Size)
    {
        memset(Dest, 0, Size);
    }
}
