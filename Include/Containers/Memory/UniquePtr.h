#pragma once
#include "Misc/Forward.h"

namespace Ember
{
    template<typename T>
    class UniquePtr
    {
    public:
        UniquePtr() : Pointer(nullptr) {}
        explicit UniquePtr(T* Pointer) : Pointer(Pointer) {}
        ~UniquePtr()
        {
            delete Pointer;
            Pointer = nullptr;
        }

        // Copy Ctor and Assignment - Deleted to prevent copies
        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        // Move Constructor
        UniquePtr(UniquePtr<T>&& Other) noexcept : UniquePtr(Other.Pointer)
        {
            Other.Pointer = nullptr;
        }

        // Move Assignment
        UniquePtr& operator=(UniquePtr<T>&& Other) noexcept
        {
            if(this != &Other)
            {
                delete Pointer;
                Pointer = Other.Pointer;
                Other.Pointer = nullptr;
            }

            return (*this);
        }

        T* Get() const { return Pointer; }
        void Reset(T* NewPointer = nullptr)
        {
            if(Pointer)
            {
                delete Pointer;
            }

            Pointer = NewPointer;
        }

        T* Release()
        {
            T* Temp = Pointer;
            Pointer = nullptr;
            return Temp;
        }

        T& operator*() const { return *Pointer; }
        T* operator->() const { return Pointer; }

        bool operator!() const { return Pointer == nullptr; }
        explicit operator bool() const { return Pointer != nullptr; }
        
    private:
        T* Pointer;
    };

    template<typename T, typename... TArgs>
    UniquePtr<T> MakeUnique(TArgs&&... Args)
    {
        return UniquePtr<T>(new T(Forward<TArgs>(Args)...));
    }
}

