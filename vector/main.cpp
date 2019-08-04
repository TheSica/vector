#include "Vector.h"
#include <iostream>
#include <vector>
#include <cstdio>
#include "gtest/gtest.h"

struct S
{
	S() { puts("S()"); }
	S(const S&) { puts("S(const S&)"); }
	S(S&&) noexcept { puts("S(S&&)"); }
	~S() { puts("~S()"); }
	S& operator=(const S&) { puts("=(const S&)"); return *this; }
	S& operator=(S&&) noexcept { puts("=(S&&)"); return *this; }
};



int main()
{
	testing::InitGoogleTest();
	RUN_ALL_TESTS();

	std::vector<S> base;
	std::vector<S> t = base;
}
