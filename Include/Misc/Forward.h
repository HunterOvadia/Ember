#pragma once

namespace Ember
{
    template<typename T> struct RemoveReference { typedef T Type; };
    template<typename T> struct RemoveReference<T&> { typedef T Type; };
    template<typename T> struct RemoveReference<T&&> { typedef T Type; };

    template<typename T>
    typename RemoveReference<T>::Type&& Forward(typename RemoveReference<T>::Type& Arg) noexcept
    {
        return static_cast<typename RemoveReference<T>::Type&&>(Arg);
    }

    template<typename T>
    typename RemoveReference<T>::Type&& Forward(typename RemoveReference<T>::Type&& Arg) noexcept
    {
        return static_cast<typename RemoveReference<T>::Type&&>(Arg);
    }
}
