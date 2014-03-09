#include <iostream>
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
	cout<<binarySearch(0, 7, 3)<<endl;
	cout<<binarySearch(0, 7, -1)<<endl;
	cout<<binarySearch(0, 7, 80.0)<<endl;
	cout<<binarySearch(0, 7, 5)<<endl;
	cout<<binarySearch(0, 7, 80.1)<<endl;
}
