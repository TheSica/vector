#include "Vector.h"
#include <iostream>
#include <vector>

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
	/*{
		std::cout << "std::vector: \n";
		std::vector<S> a;
		a.push_back(S());
	}
		std::cout << std::endl;
	{
		std::cout << "Vector: \n";
		Vector<S> a;
		a.push_back(S());
	}*/

	Vector<int> a(10);
	a.push_back(1);
	a.print();
}
