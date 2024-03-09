#pragma once
#include "Misc/Forward.h"

namespace Ember
{
    template<typename T>
    class UniquePtr
    {
    public:
        UniquePtr() noexcept : Pointer(nullptr) {}
        explicit UniquePtr(T* Pointer) noexcept : Pointer(Pointer) {}
        ~UniquePtr()
        {
            delete Pointer;
        }

        // Deleted copy constructor and copy assignment operator to enforce uniqueness
        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        // Move Constructor
        UniquePtr(UniquePtr&& Other) noexcept : UniquePtr(Other.Pointer)
        {
            Other.Pointer = nullptr;
        }

        // Move Assignment
        UniquePtr& operator=(UniquePtr&& Other) noexcept
        {
            if(this != &Other)
            {
                delete Pointer;
                Pointer = Other.Pointer;
                Other.Pointer = nullptr;
            }

            return (*this);
        }

        T* Get() const noexcept { return Pointer; }
        T& operator*() const { return *Pointer; }
        T* operator->() const noexcept { return Pointer; }
        explicit operator bool() const noexcept { return Pointer != nullptr; }
        bool operator!() const noexcept { return Pointer == nullptr; }

        void Reset(T* NewPointer = nullptr) noexcept
        {
            T* OldPointer = Pointer;
            Pointer = NewPointer;
            delete OldPointer;
        }

        T* Release() noexcept
        {
            T* OldPointer = Pointer;
            Pointer = nullptr;
            return OldPointer;
        }

    private:
        T* Pointer;
    };

    template<typename T, typename... TArgs>
    UniquePtr<T> MakeUnique(TArgs&&... Args)
    {
        return UniquePtr<T>(new T(Forward<TArgs>(Args)...));
    }
}

