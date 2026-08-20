#include <array>
#include <cstddef>
#include <ranges>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include "Allocators.h"

template<typename T, typename Fields, std::size_t Dimension, std::size_t Alignment>
class SoV
{
    static_assert(Dimension > 0);

public:
    using value_type = T;
    using field_type = Fields;
    using size_type = std::size_t;
    using vector_type = std::vector<T, Allocators::Aligned<T, Alignment>>;
    using iterator = typename std::array<vector_type, Dimension>::iterator;
    using const_iterator = typename std::array<vector_type, Dimension>::const_iterator;

    static constexpr size_type dimension = Dimension;
    static constexpr size_type alignment = Alignment;

public:
    SoV() = default;

    explicit SoV(size_type size)
    {
        resize(size);
    }

    SoV(size_type size, const T& value)
    {
        resize(size);
        fill(value);
    }

    [[nodiscard]]
    size_type size() const noexcept
    {
        return Data[0].size();
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        return Data[0].empty();
    }

    [[nodiscard]]
    size_type capacity() const noexcept
    {
        return Data[0].capacity();
    }

    void resize(size_type size)
    {
        for (auto& vec : Data)
            vec.resize(size);
    }

    void resize(size_type size, const T& value)
    {
        for (auto& vec : Data)
            vec.resize(size, value);
    }

    void reserve(size_type capacity)
    {
        for (auto& vec : Data)
            vec.reserve(capacity);
    }

    void shrink_to_fit()
    {
        for (auto& vec : Data)
            vec.shrink_to_fit();
    }

    void clear() noexcept
    {
        for (auto& vec : Data)
            vec.clear();
    }

    void fill(const T& value)
    {
        for (auto& vec : Data)
            std::ranges::fill(vec, value);
    }

    vector_type& operator[](field_type i) noexcept
    {
        return Data[static_cast<size_type>(i)];
    }

    const vector_type& operator[](field_type i) const noexcept
    {
        return Data[static_cast<size_type>(i)];
    }

    vector_type& at(field_type i)
    {
        return Data.at(static_cast<size_type>(i));
    }

    const vector_type& at(field_type i) const
    {
        return Data.at(static_cast<size_type>(i));
    }

    iterator begin() noexcept
    {
        return Data.begin();
    }

    iterator end() noexcept
    {
        return Data.end();
    }

    const_iterator begin() const noexcept
    {
        return Data.begin();
    }

    const_iterator end() const noexcept
    {
        return Data.end();
    }

    const_iterator cbegin() const noexcept
    {
        return Data.cbegin();
    }

    const_iterator cend() const noexcept
    {
        return Data.cend();
    }

    void swap(SoV& other) noexcept
    {
        Data.swap(other.Data);
    }

private:
    std::array<vector_type, Dimension> Data;
};