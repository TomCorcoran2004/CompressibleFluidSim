#pragma once
#include <cstddef>
#include <new>

namespace Allocators
{
    template<typename T, std::size_t Alignment>
    struct Aligned
    {
        using value_type = T;

        template<typename U>
        struct rebind
        {
            using other = Aligned<U, Alignment>;
        };

        T* allocate(std::size_t n)
        {
            return static_cast<T*>(::operator new(n * sizeof(T), std::align_val_t(Alignment)));
        }

        void deallocate(T* ptr, std::size_t n)
        {
            ::operator delete(ptr, std::align_val_t(Alignment));
        }
    };

}
