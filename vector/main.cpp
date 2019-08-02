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


GTEST_API_ int main(int argc, char** argv) 
{
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);


	return RUN_ALL_TESTS();
}
