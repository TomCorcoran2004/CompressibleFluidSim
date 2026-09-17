#pragma once
#pragma once
#include <array>
#include <cstddef>
#include <ranges>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include "CompressibleFluidSim/fluid/utils/gpu_buffer.h"

template<typename T, typename fields, std::size_t dimension>
class gpu_struct_of_arrays
{
public:
    gpu_struct_of_arrays() {};

    gpu_struct_of_arrays(std::size_t size)
    {
        resize(size);
    }

    std::size_t size() const
    {
        return _data[0].size();
    }

    void resize(std::size_t size)
    {
        for (auto& gpu_buffer : _data)
            gpu_buffer.resize(size);
    }

    gpu_buffer<T>& operator[](fields i)
    {
        return _data[static_cast<std::size_t>(i)];
    }

    const gpu_buffer<T>& operator[](fields i) const
    {
        return _data[static_cast<std::size_t>(i)];
    }

    auto begin()
    {
        return _data.begin();
    }

    auto end()
    {
        return _data.end();
    }

    const auto begin() const
    {
        return _data.begin();
    }

    const auto end() const
    {
        return _data.end();
    }

    const auto cbegin() const
    {
        return _data.cbegin();
    }

    const auto cend() const
    {
        return _data.cend();
    }
private:
    std::array<gpu_buffer<T>, dimension> _data;
};