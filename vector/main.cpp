#include "Vector.h"
#include <iostream>
#include <vector>
#include <cstdio>
#include "gtest/gtest.h"
#include <variant>
struct S
{
	S() { puts("S()"); }
	S(const S&) { puts("S(const S&)"); }
	S(S&&) noexcept = delete; //{ puts("S(S&&)"); }
	~S() { puts("~S()"); }
	S& operator=(const S&) { puts("=(const S&)"); return *this; }
	S& operator=(S&&) noexcept { puts("=(S&&)"); return *this; }
};

template<unsigned...>
struct gcd;

template<unsigned M, unsigned N, unsigned... Rest>
struct  gcd<M, N, Rest...>
{
	static constexpr unsigned value = gcd<gcd<M, N>::value, Rest...>::value;
};

template<unsigned M, unsigned N>
struct gcd<M, N>
{
	static constexpr unsigned value = gcd<N, M % N>::value;
};

template<unsigned M>
struct gcd<M, 0>
{
	static_assert(M != 0);
	static constexpr unsigned value = M;
};

template<unsigned M>
struct gcd<M>
{
	static_assert(M != 0);
	static constexpr unsigned value = M;
};

int main()
{
	testing::InitGoogleTest();
	RUN_ALL_TESTS();
}
