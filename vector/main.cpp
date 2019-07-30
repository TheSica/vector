#include "Vector.h"
#include <iostream>
#include <vector>

Vector<int> test()
{
	Vector<int>b(6);

	return b;
}

int main()
{
	std::vector<int> a;
	Vector<int> b(5);

	Vector<int>c(3);
	c = test();

}
