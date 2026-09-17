#pragma once
#include <cstddef>
#include <algorithm>
#include <utility>
#include <cuda_runtime.h>

template<typename T>
class gpu_buffer
{
public:
    gpu_buffer();
    gpu_buffer(std::size_t size);
    gpu_buffer(const T* buffer, std::size_t size);

    ~gpu_buffer();

    gpu_buffer(const gpu_buffer& other);
    gpu_buffer& operator=(const gpu_buffer& other);

    gpu_buffer(gpu_buffer&& other);
    gpu_buffer& operator=(gpu_buffer&& other);

    void resize(std::size_t n);

    T* data();
    const T* data() const;
    std::size_t size() const;

    void upload(const T* src, std::size_t count);
    void download(T* dst) const;

    void copy_from(const gpu_buffer<T>& other);

private:
    T* _data = nullptr;
    std::size_t _size = 0;
};

template<typename T>
gpu_buffer<T>::gpu_buffer(){}

template<typename T>
gpu_buffer<T>::gpu_buffer(std::size_t size) : _size(size)
{
    if (_size == 0) return;

    cudaMalloc(reinterpret_cast<void**>(&_data), _size * sizeof(T));
}

template<typename T>
gpu_buffer<T>::gpu_buffer(const T* buffer, std::size_t size) : _size(size)
{
    if (_size == 0) return;

    cudaMalloc(reinterpret_cast<void**>(&_data), _size * sizeof(T));
    upload(buffer, _size);
}

template<typename T>
gpu_buffer<T>::~gpu_buffer()
{
    if (_data) cudaFree(_data);
}

template<typename T>
gpu_buffer<T>::gpu_buffer(const gpu_buffer& other) : _size(other._size)
{
    if (_size == 0) return;

    cudaMalloc(reinterpret_cast<void**>(&_data), _size * sizeof(T));
    cudaMemcpy(_data, other._data, _size * sizeof(T), cudaMemcpyDeviceToDevice);
}

template<typename T>
gpu_buffer<T>& gpu_buffer<T>::operator=(const gpu_buffer& other)
{
    if (this == &other) return *this;

    gpu_buffer temp(other);

    std::swap(_data, temp._data);
    std::swap(_size, temp._size);

    return *this;
}

template<typename T>
gpu_buffer<T>::gpu_buffer(gpu_buffer&& other) : _data(other._data), _size(other._size)
{
    other._data = nullptr;
    other._size = 0;
}

template<typename T>
gpu_buffer<T>& gpu_buffer<T>::operator=(gpu_buffer&& other)
{
    if (this == &other) return *this;

    if (_data) cudaFree(_data);


    _data = other._data;
    _size = other._size;

    other._data = nullptr;
    other._size = 0;

    return *this;
}

template<typename T>
void gpu_buffer<T>::resize(std::size_t n)
{
    if (n == _size) return;

    if (n == 0)
    {
        if (_data) cudaFree(_data);

        _data = nullptr;
        _size = 0;

        return;
    }

    T* new_data = nullptr;
    cudaMalloc(reinterpret_cast<void**>(&new_data), n * sizeof(T));

    if (_data)
    {
        const std::size_t copy_count = std::min(_size, n);

        cudaMemcpy(new_data, _data, copy_count * sizeof(T), cudaMemcpyDeviceToDevice);

        cudaFree(_data);
    }

    _data = new_data;
    _size = n;
}

template<typename T>
T* gpu_buffer<T>::data()
{
    return _data;
}

template<typename T>
const T* gpu_buffer<T>::data() const
{
    return _data;
}

template<typename T>
std::size_t gpu_buffer<T>::size() const
{
    return _size;
}

template<typename T>
void gpu_buffer<T>::upload(const T* src, std::size_t count)
{
    if (count != _size)
        resize(count);

    cudaMemcpy(_data, src, count * sizeof(T), cudaMemcpyHostToDevice);
}

template<typename T>
void gpu_buffer<T>::download(T* dst) const
{
    cudaMemcpy(dst, _data, _size * sizeof(T), cudaMemcpyDeviceToHost);
}

template<typename T>
void gpu_buffer<T>::copy_from(const gpu_buffer<T>& other)
{
    if (this == &other)
        return;

    if (_size != other._size)
    {
        resize(other._size);
    }

    if (_size == 0)
        return;

    cudaMemcpy(_data, other._data, _size * sizeof(T), cudaMemcpyDeviceToDevice);
}