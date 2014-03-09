#include <iostream>
#include <cassert>
using namespace std;

float data[] = {1, 2, 3, 4, 6, 8, 80};

int binarySearch(int first, int last, float val)
{
	int mid = (last + first)/2;
	if(mid == first)
		return first;
	else
	{
		if(val == data[mid])
			return mid;
		else if(val > data[mid])
			return binarySearch(mid, last, val);
		else
			return binarySearch(first, mid, val);
	}
}

int main()
{
	assert(2 == binarySearch(0, 7, 3));
	assert(0 == binarySearch(0, 7, -1));
	assert(6 == binarySearch(0, 7, 80.0));
	assert(3 == binarySearch(0, 7, 5));
	assert(6 == binarySearch(0, 7, 80.1));
}
