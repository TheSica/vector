#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>

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
private:
	void cleanup();
	void resize();
private:
	size_t _size;
	size_t _capacity;
	T* _container;
};

template<typename T>
Vector<T>::Vector()
	:
	_size(0),
	_capacity(1),
	_container(static_cast<T*>(std::malloc(sizeof(T))))
{
}

template<typename T>
Vector<T>::Vector(size_t size)
	:
	_size(size),
	_capacity(size * 2),
	_container(static_cast<T*>(std::malloc(sizeof(T)* _capacity)))
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
	_capacity(other._capacity),
	_container(static_cast<T*>(std::malloc(sizeof(T)* _capacity)))
{
	for (size_t i = 0; i < other._size; ++i)
	{
		push_back(other._container[i]);
	}
}

template<typename T>
Vector<T>::Vector(Vector<T>&& other) noexcept
	:
	_size(0),
	_capacity(0),
	_container(nullptr)
{
	std::swap(_size, other._size);
	std::swap(_capacity, other._capacity);
	std::swap(_container, other._container);
}

template<typename T>
Vector<T>::~Vector()
{
	cleanup();
}

template<typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
	_size = other._size;
	_capacity = other._capacity;

	std::copy(other._container, other._container + other._size, _container);

	return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept
{
	cleanup();

	std::swap(_size, other._size);
	std::swap(_capacity, other._capacity);
	std::swap(_container, other._container);

	return *this;
}

template<typename T>
void Vector<T>::push_back(const T& element)
{
	if (_size + 1 > _capacity)
	{
		resize();
	}

	new(_container + _size) T(element);
	_size += 1;
}

template<typename T>
void Vector<T>::push_back(T&& element)
{
	if (_size + 1 > _capacity)
	{
		resize();
	}

	new(_container + _size) T(std::move(element));
	_size += 1;
}

template<typename T>
inline void Vector<T>::print()
{
	for (int i = 0; i < _size; ++i)
	{
		std::cout << _container[i] << " ";
	}

	std::cout<<std::endl;
}

template<typename T>
void Vector<T>::cleanup()
{
	for (int i = 0; i < _size; ++i)
	{
		_container[_size - 1 - i].~T();
	}

	free(_container);
	_container = nullptr;
}

template<typename T>
void Vector<T>::resize()
{
	_capacity *= 2;

	if (void* mem = std::realloc(_container, _capacity))
	{
		_container = static_cast<T*>(mem);
	}
	else
	{
		throw std::bad_alloc();
	}
}
