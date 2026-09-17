#pragma once
#include <cstddef>
#include <new>

namespace allocators
{
    template<typename T, std::size_t alignment>
    struct aligned
    {
        using value_type = T;

        template<typename U>
        struct rebind
        {
            using other = aligned<U, alignment>;
        };

        aligned() noexcept = default;

        template<typename U>
        aligned(const aligned<U, alignment>&) noexcept {}

        T* allocate(std::size_t n)
        {
            return static_cast<T*>(::operator new(n * sizeof(T), std::align_val_t(alignment)));
        }

        void deallocate(T* ptr, std::size_t n)
        {
            ::operator delete(ptr, std::align_val_t(alignment));
        }
    };
}
