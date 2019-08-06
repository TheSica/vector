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

/*	Vector<S> base;
	base.push_back({});
	base.push_back({});
	base.push_back({});
	base.push_back({});
	base.push_back({});
	std::cout << std::endl;
	base.erase(base.begin());*/

	Vector<int> base;
	base.push_back(1);
	base.push_back(2);
	base.push_back(3);
	base.push_back(4);
	base.push_back(5);
	std::cout << std::endl;

	base.erase(base.begin()+2, base.begin()+4);

	for (auto i : base)
	{
		std::cout << i << " ";
	}
}
