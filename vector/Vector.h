#pragma once

#include <algorithm>
#include <cassert>

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

private:
	void cleanup();
private:
	size_t size;
	size_t capacity;
	T* container;
};

template<typename T>
inline Vector<T>::Vector()
	:
	size(0),
	capacity(1),
	container(static_cast<T*>(std::malloc(sizeof(T))))
{
}

template<typename T>
inline Vector<T>::Vector(size_t size)
	:
	size(size),
	capacity(size * 2),
	container(static_cast<T*>(std::malloc(sizeof(T)* capacity)))
{

	for (size_t i = 0; i < size; ++i)
	{
		new (container + i) T();
	}
}

template<typename T>
inline Vector<T>::Vector(const Vector& other)
	:
	size(other.size),
	capacity(other.capacity),
	container(static_cast<T*>(std::malloc(sizeof(T) * capacity)))
{
	std::copy(other.container, other.container + size, container);
}

template<typename T>
inline Vector<T>::Vector(Vector&& other) noexcept
	:
	size(0),
	capacity(0),
	container(nullptr)
{
	std::swap(size, other.size);
	std::swap(capacity, other.capacity);
	std::swap(container, other.container);
}

template<typename T>
inline Vector<T>::~Vector()
{
	cleanup();
}

template<typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
	size = other.size;
	capacity = other.capacity;

	std::copy(other.container, other.container + other.size, container);

	return *this;
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept
{
	cleanup();

	std::swap(size, other.size);
	std::swap(capacity, other.capacity);
	std::swap(container, other.container);

	return *this;
}

template<typename T>
inline void Vector<T>::cleanup()
{
	for (int i = size - 1; i >= 0; --i)
	{
		container[i].~T();
	}

	delete[] container;
	container = nullptr;

	size = 0;
	capacity = 0;
}
