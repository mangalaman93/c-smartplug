#ifndef MC_H
#define MC_H

#include <iostream>
#include <cstdlib>
#include <queue>
using namespace std;

// true => min heap; false => max heap
class CompareFloat
{
	bool reverse;

  public:
	CompareFloat(const bool& rev=false){ reverse=rev;}
	bool operator()(const float& lhs, const float& rhs) const
	{
		if(reverse) return (lhs > rhs);
		else return (lhs < rhs);
	}
};

typedef priority_queue<float, vector<float>, CompareFloat> Heap;

// median container class
class Mc
{
	Heap max_heap;
	Heap min_heap;
	int size;
	float extra_elem;
	bool odd;

  public:
	Mc();
	~Mc();
	float getMedian();
	void insert(float elem);
};

#endif
