#pragma once
#include "Ember.h"

namespace Ember
{
    template<typename T>
    class DynamicArray
    {
    public:
        DynamicArray() : Data(nullptr), InternalSize(0), InternalCapacity(0) {}
        explicit DynamicArray(size_t InitialSize) : DynamicArray() { Resize(InitialSize); }
        ~DynamicArray() { Memory::Free(Data); }

        void Resize(size_t NewSize)
        {
            if(NewSize > InternalCapacity)
            {
                Reserve(RequestReserveCapacityFromSize(NewSize));
            }

            InternalSize = NewSize;
        }

        void Reserve(size_t NewCapacity)
        {
            if(NewCapacity <= InternalCapacity)
            {
                return;
            }

            T* NewData = Memory::AllocateType<T>(NewCapacity);
            if(Data)
            {
                Memory::Copy(NewData, Data, InternalSize * sizeof(T));
                Memory::Free(Data);
            }

            Data = NewData;
            InternalCapacity = NewCapacity;
        }

        void Add(const T& Value)
        {
            if(InternalSize == InternalCapacity)
            {
                Reserve(RequestReserveCapacityFromSize(InternalSize + 1));
            }
            
            Memory::Copy(&Data[InternalSize], &Value, sizeof(Value));
            ++InternalSize;
        }

        size_t Size() const { return InternalSize; }
        T* GetData() const { return Data; }

        T* begin() { return Data; }
        const T* begin() const { return Data; }
        
        T* end() { return Data + InternalSize; }
        const T* end() const { return Data + InternalSize; }

        T& operator[](size_t Index) { EMBER_ASSERT(Index >= 0 && Index < InternalSize); return Data[Index]; }
        const T& operator[](size_t Index) const { EMBER_ASSERT(Index >= 0 && Index < InternalSize); return Data[Index]; }

       


    private:
        size_t RequestReserveCapacityFromSize(size_t RequestedSize) const
        {
            size_t NewCapacity = InternalCapacity > 0 ? (InternalCapacity + (InternalCapacity / 2)) : 8;
            return NewCapacity > RequestedSize ? NewCapacity : RequestedSize;
        }

    private:
        T* Data;
        size_t InternalSize;
        size_t InternalCapacity;
    };
}
