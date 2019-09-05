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

	std::vector<int> s;
	//std::copy(t.begin(), t.end(), std::ostream_iterator<int>(std::cout, " "));
/*	Vector<S> base;
	base.push_back({});
	base.push_back({});
	base.push_back({});
	base.push_back({});
	base.push_back({});
	std::cout << std::endl;
	base.erase(base.begin());*/

	/*Vector<int> base;
	base.push_back(1);
	base.push_back(2);
	base.push_back(3);
	base.push_back(4);
	base.push_back(5);
	std::cout << std::endl;*/
}
