#include "mc.h"
using namespace std;

Mc::Mc()
{
	min_heap = Heap(CompareFloat(true));
	size = 0;
	odd = false;
}

float Mc::getMedian()
{
	if(size == 0)
	{
		return (-1);
	}

	if(odd)
		return extra_elem;
	else
		return (max_heap.top() + min_heap.top())/2;
}

void Mc::insert(float elem)
{
	size++;
	if(size == 1)
	{
		extra_elem = elem;
		odd = true;
		return;
	}

	if(odd)
	{
		if(extra_elem > elem)
		{
			min_heap.push(extra_elem);
			max_heap.push(elem);
		} else
		{
			min_heap.push(elem);
			max_heap.push(extra_elem);
		}
		odd = false;
	} else
	{
		if(elem >= max_heap.top())
		{
			if(elem < min_heap.top())
				extra_elem = elem;
			else
			{
				extra_elem = min_heap.top();
				min_heap.pop();
				min_heap.push(elem);
			}
		} else
		{
			extra_elem = max_heap.top();
			max_heap.pop();
			max_heap.push(elem);
		}
		odd = true;
	}
}

Mc::~Mc()
{
}
