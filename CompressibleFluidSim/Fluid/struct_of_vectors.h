#include <array>
#include <cstddef>
#include <ranges>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include "std_allocators.h"

template<typename T, typename Fields, std::size_t dimension, std::size_t alignment>
class sov
{
    static_assert(dimension > 0);

public:
    using value_type = T;
    using field_type = Fields;
    using size_type = std::size_t;
    using vector_type = std::vector<T, allocators::aligned<T, alignment>>;
    using iterator = typename std::array<vector_type, dimension>::iterator;
    using const_iterator = typename std::array<vector_type, dimension>::const_iterator;

    static constexpr size_type dimensions = dimension;
    static constexpr size_type alignment_value = alignment;

public:
    sov() = default;

    explicit sov(size_type size)
    {
        resize(size);
    }

    sov(size_type size, const T& value)
    {
        resize(size);
        fill(value);
    }

    [[nodiscard]]
    size_type size() const noexcept
    {
        return data[0].size();
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        return data[0].empty();
    }

    [[nodiscard]]
    size_type capacity() const noexcept
    {
        return data[0].capacity();
    }

    void resize(size_type size)
    {
        for (auto& vec : data)
            vec.resize(size);
    }

    void resize(size_type size, const T& value)
    {
        for (auto& vec : data)
            vec.resize(size, value);
    }

    void reserve(size_type capacity)
    {
        for (auto& vec : data)
            vec.reserve(capacity);
    }

    void shrink_to_fit()
    {
        for (auto& vec : data)
            vec.shrink_to_fit();
    }

    void clear() noexcept
    {
        for (auto& vec : data)
            vec.clear();
    }

    void fill(const T& value)
    {
        for (auto& vec : data)
            std::ranges::fill(vec, value);
    }

    vector_type& operator[](field_type i) noexcept
    {
        return data[static_cast<size_type>(i)];
    }

    const vector_type& operator[](field_type i) const noexcept
    {
        return data[static_cast<size_type>(i)];
    }

    vector_type& at(field_type i)
    {
        return data.at(static_cast<size_type>(i));
    }

    const vector_type& at(field_type i) const
    {
        return data.at(static_cast<size_type>(i));
    }

    iterator begin() noexcept
    {
        return data.begin();
    }

    iterator end() noexcept
    {
        return data.end();
    }

    const_iterator begin() const noexcept
    {
        return data.begin();
    }

    const_iterator end() const noexcept
    {
        return data.end();
    }

    const_iterator cbegin() const noexcept
    {
        return data.cbegin();
    }

    const_iterator cend() const noexcept
    {
        return data.cend();
    }

    void swap(sov& other) noexcept
    {
        data.swap(other.data);
    }

private:
    std::array<vector_type, dimension> data;
};