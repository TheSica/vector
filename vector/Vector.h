#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <malloc.h>

template<typename T>
class Vector
{
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
	void print();
public:
	bool validate() const noexcept;
	bool empty() const noexcept;
	size_t size() const noexcept;
private:
	void cleanup();
	void resize(size_t new_capacity);
	void swap(Vector<T>& other);
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
	for (size_t i = 0; i < size; ++i)
	{
		new (_container + i) T();
	}
}

template<typename T>
Vector<T>::Vector(const Vector<T>& other)
	:
	_size(0),
	_capacity(other._size),
	_container(static_cast<T*>(_aligned_malloc(sizeof(T)* _size, alignof(T))))
{
	for (size_t i = 0; i < other._size; ++i)
	{
		push_back(other._container[i]);
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
		swap(tmp);
		/*if (_capacity >= other._capacity)
		{
			for (int i = 0; i < _size; ++i)
			{
				_container[_size - 1 - i].~T();
			}

			_size = other._size;
		}
		else
		{
			cleanup();

			_capacity = other._capacity;
			_size = other._size;

			_container = static_cast<T*>(_aligned_malloc(sizeof(T) * _capacity));
		}

		std::copy(other._container, other._container + other._size, _container);*/
	}
	return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept
{
	if (&other != this)
	{
		swap(other);
	}
	return *this;
}


template<typename T>
void Vector<T>::push_back(const T& element)
{
	if (_size == _capacity)
	{
		resize(_capacity * 2);
	}

	new(_container + _size) T(element);
	_size += 1;
}

template<typename T>
void Vector<T>::push_back(T&& element)
{
	if (_size ==_capacity)
	{
		resize(_capacity * 2);
	}

	new(_container + _size) T(std::move(element));
	_size += 1;
}

template<typename T>
inline void Vector<T>::print()
{
	for (int i = 0; i < _size; ++i)
	{
		//std::cout << _container[i] << " ";
	}

	std::cout << std::endl;
}

template<typename T>
inline bool Vector<T>::validate() const noexcept
{
	return true;
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
void Vector<T>::cleanup()
{
	for (int i = 0; i < _size; ++i)
	{
		_container[_size - 1 - i].~T();
	}

	_aligned_free(_container);
}

template<typename T>
void Vector<T>::resize(size_t new_capacity)
{
	std::cout<<"called"; 
	_capacity = new_capacity;

	if (void* mem = std::realloc(_container, _capacity))
	{
		_container = static_cast<T*>(mem);
	}
	else
	{
		throw std::bad_alloc();
	}
}

template<typename T>
inline void Vector<T>::swap(Vector<T>& other)
{
	_size = other._size;
	_capacity = other._capacity;
	_container = other._container;
}


template<typename T>
inline bool operator==(const Vector<T>& a, const Vector<T>& b)
{
	//return ((a.size() == b.size()) && equal(a.begin(), a.end(), b.begin()));
	return a.size() == b.size();
}
