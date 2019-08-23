#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <malloc.h>
#include <type_traits>

template<typename T>
class Vector
{
public:
	typedef				T* iterator;
	typedef const		T* const_iterator;

	typedef				T& reference;
	typedef const		T& const_reference;

	typedef				T* pointer;
	typedef const		T* const_pointer;

public:
	Vector();
	explicit Vector(size_t size);
	Vector(const Vector<T>& other);
	Vector(Vector<T>&& other) noexcept;
	~Vector();
	Vector<T>& operator=(const Vector<T>& other);
	Vector<T>& operator=(Vector<T>&& other) noexcept;

public:
	void push_back(const T& element);
	void push_back(T&& element);

	iterator erase(iterator pos);
	const_iterator erase(const_iterator pos);
	iterator erase(iterator pos, iterator last);

	template<class... Args>
	reference emplace_back(Args&& ... args);

	reference operator[](size_t n);
	const_reference operator[](size_t n) const;
	reference at(size_t n);
	const_reference at(size_t n) const;

public:
	bool validate() const noexcept;
	bool empty() const noexcept;
	size_t size() const noexcept;

public:
	iterator				begin() noexcept;
	const_iterator			begin() const noexcept;

	iterator				end() noexcept;
	const_iterator			end() const noexcept;

	reference				front();
	const_reference			front() const;

	reference				back();
	const_reference			back() const;

	pointer					data() noexcept;
	const_pointer			data() const noexcept;

private:
	void cleanup();
	void resize();
	void swap(Vector<T>& other) noexcept;
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
Vector<T>::Vector(size_t size)
	:
	_size(size),
	_capacity(size),
	_container(static_cast<T*>(_aligned_malloc(sizeof(T)* size, alignof(T))))
{
	try
	{
		for (size_t i = 0; i < size; ++i)
		{
			new (_container + i) T();
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
	_size(0),
	_capacity(other._size),
	_container(static_cast<T*>(_aligned_malloc(sizeof(T)* other._size, alignof(T))))
{
	if constexpr (std::is_trivially_constructible_v<T>)
	{
		try
		{
			for (_size = 0; _size < other._size; ++_size)
			{
				push_back(other._container[_size]);
			}
		}
		catch (...)
		{
			cleanup();
			throw;
		}
	}
	else
	{
		std::memcpy(_container, other._container, other._size);
		_size = other._size;
	}
}

template<typename T>
Vector<T>::Vector(Vector<T>&& other) noexcept
	:
	_size(other._size),
	_capacity(other._capacity),
	_container(other._container)
{
	other._size = 0;
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
Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept
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

	new(_container + _size) T(element);
	_size += 1;
}

template<typename T>
void Vector<T>::push_back(T&& element)
{
	if (_size == _capacity)
	{
		resize();
	}

	new(_container + _size) T(std::move(element));
	_size += 1;
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

	new(_container + _size) T(std::forward<Args>(args)...);
	_size += 1;

	return back();
}

template<typename T>
void Vector<T>::cleanup()
{
	std::destroy(begin(), end());

	_aligned_free(_container);
}

template<typename T>
std::enable_if_t<std::is_nothrow_move_constructible_v<T>> resize_specialized(T* first, T* last, T* dest)
{
	std::uninitialized_move(first, last, dest);
}

template<typename T>
std::enable_if_t<std::is_copy_constructible_v<T> && !std::is_nothrow_move_constructible_v<T>> resize_specialized(T* first, T* last, T* dest)
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
void Vector<T>::resize()
{
	_capacity = std::max(static_cast<size_t>(2), _capacity * 2);

	if (void* try_alloc_mem = _aligned_malloc(sizeof(T) * _capacity, alignof(T)))
	{
		try
		{
			auto alloced_mem = static_cast<T*>(try_alloc_mem);

			resize_specialized<T>(begin(), end(), alloced_mem);

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
inline void Vector<T>::swap(Vector<T>& other) noexcept
{
	std::swap(_size, other._size);
	std::swap(_capacity, other._capacity);
	std::swap(_container, other._container);
}

template<typename T>
inline bool operator==(const Vector<T>& a, const Vector<T>& b)
{
	return ((a.size() == b.size()) && std::equal(a.begin(), a.end(), b.begin()));
}

template<typename T>
typename Vector<T>::reference
Vector<T>::operator[](size_t index)
{
	return *(begin() + index);
}

template<typename T>
typename Vector<T>::const_reference
Vector<T>::operator[](size_t index) const
{
	return *(begin() + index);
}

template<typename T>
typename Vector<T>::reference
Vector<T>::at(size_t index)
{
	if (index >= (static_cast<size_t>(end() - begin())))
	{
		throw std::out_of_range("Vector::at -- out of range");
	}

	return _container[index];
}

template<typename T>
typename Vector<T>::const_reference
Vector<T>::at(size_t index) const
{
	if (index >= (static_cast<size_t>(end() - begin())))
	{
		throw std::out_of_range("Vector::at -- out of range");
	}

	return _container[index];
}

template<typename T>
inline bool Vector<T>::validate() const noexcept
{
	//return(begin() < end() && _capacity >= _size);
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