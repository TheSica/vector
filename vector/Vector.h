#pragma once

#include <algorithm>
#include <type_traits>

template<typename T>
class Vector
{
public:
    typedef                T* iterator;
    typedef const          T* const_iterator;

    typedef                T& reference;
    typedef const          T& const_reference;

    typedef                T* pointer;
    typedef const          T* const_pointer;

public:
    Vector();
    explicit Vector(const size_t size);
    Vector(const Vector<T>& other);
    Vector(Vector<T>&& other) noexcept (std::is_nothrow_move_constructible_v<T>);
    ~Vector();
    Vector<T>& operator=(const Vector<T>& other);
    Vector<T>& operator=(Vector<T>&& other) noexcept(std::is_nothrow_move_assignable_v<T>);

public:
    template<class... Args>
    reference emplace_back(Args&& ... args);

    void push_back(const T& element);
    void push_back(T&& element);

    iterator insert(iterator pos, const T& value);
    iterator insert(iterator pos, T&& value);

    iterator erase(iterator pos);
    const_iterator erase(const_iterator pos);
    iterator erase(iterator pos, iterator last);

    reference operator[](const size_t n) noexcept;
    const_reference operator[](const size_t n) const noexcept;

    reference at(const size_t n);
    const_reference at(const size_t n) const;

public:
    bool validate() const noexcept;
    bool empty() const noexcept;
    size_t size() const noexcept;
    size_t capacity() const noexcept;
    void reserve(const size_t newCapacity);

public:
    iterator                   begin() noexcept;
    const_iterator             begin() const noexcept;
    const_iterator             cbegin() const noexcept;

    iterator                   end() noexcept;
    const_iterator             end() const noexcept;
    const_iterator             cend() const noexcept;

    reference                  front();
    const_reference            front() const;

    reference                  back();
    const_reference            back() const;

    pointer                    data() noexcept;
    const_pointer              data() const noexcept;

private:
    void cleanup();
    void reallocate(const size_t desiredCapacity);
    void resize();
    void swap(Vector<T>& other) noexcept;
    void memcopy_trivially(T* src, T* dest, const size_t size);
    template<class... Args>
    void emplace_back_internal(Args&& ... element);
    template<class... U>
    void emplace_internal(iterator pos, U&& ... value);

private:
    size_t _size;
    size_t _capacity;
    T* _container;
};

template<typename T>
Vector<T>::Vector()
    :
    _size(0),
    _capacity(0),
    _container(nullptr)
{
}

template<typename T>
Vector<T>::Vector(const size_t size)
    :
    _size(0),
    _capacity(size),
    _container(static_cast<T*>(_aligned_malloc(sizeof(T)* size, alignof(T))))
{
    try
    {
        for (; _size < _capacity; _size += 1)
        {
            new (_container + _size) T();
        }
    }
    catch (...)
    {
        cleanup();
        throw;
    }
}

template<typename T>
Vector<T>::Vector(const Vector<T>& other)
    :
    _size(other._size),
    _capacity(other._size),
    _container(static_cast<T*>(_aligned_malloc(sizeof(T)* other._size, alignof(T))))
{
    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memcopy_trivially(_container, other._container, other._size);
    }
    else
    {
		std::uninitialized_copy(other.begin(), other.end(), _container);

       /* try
        {
			
            for (_size = 0; _size < other._size; _size += 1)
            {
                emplace_back_internal(std::forward<T>(other._container[_size]));
            }
        }
        catch (...)
        {
            cleanup();
            throw;
        }*/
    }
}

template<typename T>
Vector<T>::Vector(Vector<T>&& other) noexcept (std::is_nothrow_move_constructible_v<T>)
    :
    _size(other._size),
    _capacity(other._capacity),
    _container(other._container)
{
    other._size = 0;
    other._capacity = 0;
    other._container = nullptr;
}

template<typename T>
Vector<T>::~Vector()
{
    cleanup();
}

template<typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
    if (&other != this)
    {
        Vector<T> tmp(other);
        tmp.swap(*this);
    }
    return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
{
    if (&other != this)
    {
        other.swap(*this);
    }
    return *this;
}

template<typename T>
void Vector<T>::push_back(const T& element)
{
    if (_size == _capacity)
    {
        resize();
    }

    emplace_back_internal(element);
    _size += 1;
}

template<typename T>
void Vector<T>::push_back(T&& element)
{
    if (_size == _capacity)
    {
        resize();
    }

    emplace_back_internal(std::move(element));
    _size += 1;
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::insert(iterator pos, const T& value)
{
    emplace_internal(pos, value);

    _size += 1;

    return pos;
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::insert(iterator pos, T&& value)
{
    emplace_internal(pos, std::move(value));

    _size += 1;

    return pos;
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::erase(iterator position)
{
    if (position < begin() || position >= end())
    {
        throw std::out_of_range("Vector::erase -- out of range");
    }

    std::move(position + 1, end(), position);

    back().~T();
    _size -= 1;

    return position;
}

template<typename T>
typename Vector<T>::const_iterator
Vector<T>::erase(const_iterator position)
{
    if (position < begin() || position >= end())
    {
        throw std::out_of_range("Vector::erase -- out of range");
    }

    auto destPositon = const_cast<iterator>(position);

    return erase(destPositon);
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::erase(iterator first, iterator last)
{
    if (first > last || first < begin() || first > end() || last < begin() || last > end())
    {
        throw std::out_of_range("Vector::erase(first, last) -- out of range");
    }

    if (first == last)
    {
        return begin();
    }

    size_t elementsToRemoveCnt = std::distance(first, last);

    auto position = std::move(last, end(), first);

    std::destroy(position, end());

    _size -= elementsToRemoveCnt;

    return first;
}

template<typename T>
template<class... Args>
inline typename Vector<T>::reference
Vector<T>::emplace_back(Args&& ... args)
{
    if (_size == _capacity)
    {
        resize();
    }

    emplace_back_internal(std::move(args)...);
    _size += 1;

    return back();
}

template<typename T>
void Vector<T>::cleanup()
{
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        std::destroy(begin(), end());
    }

    _aligned_free(_container);
}

template<typename T>
std::enable_if_t<std::is_nothrow_move_constructible_v<T>> uninitialized_move_or_copy(T* first, T* last, T* dest)
{
    std::uninitialized_move(first, last, dest);
}

template<typename T>
std::enable_if_t<std::is_copy_constructible_v<T> && !std::is_nothrow_move_constructible_v<T>> uninitialized_move_or_copy(T* first, T* last, T* dest)
{
    try
    {
        std::uninitialized_copy(first, last, dest);
    }
    catch (...)
    {
        _aligned_free(dest);
        throw;
    }
}

template<typename T>
inline void Vector<T>::reallocate(const size_t desiredCapacity)
{
    _capacity = desiredCapacity;

    if (void* try_alloc_mem = _aligned_malloc(sizeof(T) * _capacity, alignof(T)))
    {
        try
        {
            auto alloced_mem = static_cast<T*>(try_alloc_mem);

            if constexpr (std::is_trivially_copyable_v<T>)
            {
                memcopy_trivially(alloced_mem, _container, _size);
            }
            else
            {
                uninitialized_move_or_copy<T>(begin(), end(), alloced_mem);
            }

            cleanup();

            _container = alloced_mem;
        }
        catch (...)
        {
            _aligned_free(try_alloc_mem);
            throw;
        }
    }
    else
    {
        throw std::bad_alloc();
    }
}

template<typename T>
void Vector<T>::resize()
{
    reallocate(std::max(static_cast<size_t>(2), _capacity * 2));
}

template<typename T>
inline void Vector<T>::swap(Vector<T>& other) noexcept
{
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_container, other._container);
}

template<typename T>
void Vector<T>::memcopy_trivially(T* dest, T* src, const size_t size)
{
    std::memcpy(dest, src, size * sizeof(T));
    _size = size;
}

template<typename T>
template<class... U>
void Vector<T>::emplace_internal(iterator pos, U&& ... value)
{
    if (pos < begin() || pos > end())
    {
        throw std::out_of_range("Vector::insert -- out of range");
    }

    if (pos == end())
    {
        if (_size == _capacity)
        {
            resize();
        }

        emplace_back_internal(value...);

        return;
    }

    const size_t positionIndex = std::distance(begin(), pos);

    if (_size == _capacity)
    {
        resize();
    }

    emplace_back_internal(std::move(back()));

    if constexpr (std::is_nothrow_move_assignable_v<T>)
    {
        std::move_backward(begin() + positionIndex, end() - 1, end());
    }
    else
    {
        Vector<T> tmp(*this);
        try
        {
            std::copy_backward(begin() + positionIndex, end() - 1, end()); // does mempcy for trivial objects
        }
        catch (...)
        {
            cleanup();
            swap(tmp);
            throw;
        }
    }

    new(begin() + positionIndex) T(std::forward<U>(value)...);
}

template<typename T>
template<class... Args>
inline void Vector<T>::emplace_back_internal(Args&& ... element)
{
    new(_container + _size) T(std::forward<Args>(element)...);
}

template<typename T>
inline bool operator==(const Vector<T>& a, const Vector<T>& b)
{
    return ((a.size() == b.size()) && std::equal(a.begin(), a.end(), b.begin()));
}

template<typename T>
typename Vector<T>::reference
Vector<T>::operator[](const size_t index) noexcept
{
    return *(begin() + index);
}

template<typename T>
typename Vector<T>::const_reference
Vector<T>::operator[](const size_t index) const noexcept
{
    return *(begin() + index);
}

template<typename T>
typename Vector<T>::reference
Vector<T>::at(const size_t index)
{
    if (index >= size())
    {
        throw std::out_of_range("Vector::at -- out of range");
    }

    return _container[index];
}

template<typename T>
typename Vector<T>::const_reference
Vector<T>::at(const size_t index) const
{
    if (index >= size())
    {
        throw std::out_of_range("Vector::at -- out of range");
    }

    return _container[index];
}

template<typename T>
inline bool Vector<T>::validate() const noexcept
{
    return (_capacity >= _size);
}

template<typename T>
inline bool Vector<T>::empty() const noexcept
{
    return _size == 0;
}

template<typename T>
inline size_t Vector<T>::size() const noexcept
{
    return _size;
}

template<typename T>
inline size_t Vector<T>::capacity() const noexcept
{
    return _capacity;
}

template<typename T>
inline void Vector<T>::reserve(const size_t newCapacity)
{
    if (newCapacity <= _capacity)
    {
        return;
    }

    if (!empty())
    {
        reallocate(newCapacity);
    }
    else if (empty() && _capacity > 0)
    {
        _aligned_free(_container);

        _container = static_cast<T*>(_aligned_malloc(sizeof(T) * newCapacity, alignof(T)));
    }
    else if (empty() && _capacity == 0)
    {
        _container = static_cast<T*>(_aligned_malloc(sizeof(T) * newCapacity, alignof(T)));
    }
    else
    {
        // ?
        throw;
    }

    _capacity = newCapacity;
}

template<typename T>
inline typename Vector<T>::iterator
Vector<T>::begin() noexcept
{
    return _container;
}

template<typename T>
inline typename Vector<T>::const_iterator
Vector<T>::begin() const noexcept
{
    return _container;
}

template<typename T>
typename Vector<T>::const_iterator
Vector<T>::cbegin() const noexcept
{
    return _container;
}

template<typename T>
inline typename Vector<T>::iterator
Vector<T>::end() noexcept
{
    return _container + _size;
}

template<typename T>
inline typename Vector<T>::const_iterator
Vector<T>::end() const noexcept
{
    return _container + _size;
}

template<typename T>
typename Vector<T>::const_iterator
Vector<T>::cend() const noexcept
{
    return _container + _size;
}

template<typename T>
inline typename Vector<T>::reference
Vector<T>::front()
{
    return const_cast<reference>(std::as_const(*this).front());
}
    
template<typename T>
inline typename Vector<T>::const_reference
Vector<T>::front() const
{
    if (empty())
    {
        throw std::range_error("vector::front -- empty vector");
    }

    return *begin();
}

template<typename T>
inline typename Vector<T>::reference
Vector<T>::back()
{
    return const_cast<reference>(std::as_const(*this).back());
}

template<typename T>
inline typename Vector<T>::const_reference
Vector<T>::back() const
{
    if (empty())
    {
        throw std::range_error("vector::back -- empty vector");
    }

    return *std::prev(end());
}

template<typename T>
inline typename Vector<T>::const_pointer
Vector<T>::data() const noexcept
{
    return _container;
}

template<typename T>
inline typename Vector<T>::pointer
Vector<T>::data() noexcept
{
    return _container;
}
