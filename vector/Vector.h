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
	try
	{
		for (size_t i = 0; i < other._size; ++i)
		{
			push_back(other._container[i]);
		}
	}
	catch (...)
	{
		cleanup();
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
		tmp.swap(*this); // ? if tmp's ctor throws ?
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
std::enable_if_t<std::is_copy_assignable_v<T>> resize_specialized(T* begin, T* end, T* mem)
{
	std::uninitialized_copy(begin, end, mem);
}

template<typename T>
std::enable_if_t<std::is_move_assignable_v<T>> resize_specialized(T* begin, T* end, T* mem)
{
	std::uninitialized_move(begin, end, mem);
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
			resize_specialized(begin(), end(), alloced_mem);

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
Vector<T>::operator[](size_t n)
{
	return *(begin() + n);
}

template<typename T>
typename Vector<T>::const_reference
Vector<T>::operator[](size_t n) const
{
	return *(begin() + n);
}

template<typename T>
typename Vector<T>::reference
Vector<T>::at(size_t n)
{
	if (n >= (static_cast<size_t>(end() - begin())))
	{
		throw std::out_of_range("Vector::operator[] -- out of range");
	}

	return _container[n];
}

template<typename T>
typename Vector<T>::const_reference
Vector<T>::at(size_t n) const
{
	if (n >= (static_cast<size_t>(end() - begin())))
	{
		throw std::out_of_range("Vector::operator[] -- out of range");
	}

	return _container[n];
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
	if (_container + _size <= _container)
	{
		throw std::range_error("vector::front -- empty vector");
	}

	return *_container;
}

template<typename T>
inline typename Vector<T>::const_reference
Vector<T>::front() const
{
	if (_container + _size <= _container)
	{
		throw std::range_error("vector::front -- empty vector");
	}

	return *_container;
}

template<typename T>
inline typename Vector<T>::reference
Vector<T>::back()
{
	if (_container + _size <= _container)
	{
		throw std::range_error("vector::front -- empty vector");
	}

	return *(_container + _size - 1);
}


template<typename T>
inline typename Vector<T>::const_reference
Vector<T>::back() const
{
	if (_container + _size <= _container)
	{
		throw std::range_error("vector::front -- empty vector");
	}

	return *(_container + _size - 1);
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