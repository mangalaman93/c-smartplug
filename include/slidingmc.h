#ifndef SLIDINGMC_H
#define SLIDINGMC_H

#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

#define MAXNUM_BINS 1200

struct Bin
{
	float val;
	int freq;

	Bin(float v=-1, int f=0)
	: val(v), freq(f) {}
};

class SlidingMc
{
	int max_bins;   // maximum number of bins
	int size;  		// number of elments in the median container for 1 hr & 24hr sliding window
	int num_bins;	// number of bins <= NUM_BINS for 1 hr & 24hr sliding window
	Bin *bins;		// array of bins for 1 hr & 24hr sliding window

	int cum_sum;
	int cur_median_index;
	int maxnum_bins = MAXNUM_BINS;

	float findMedian();
	int binarySearch(float val);
	int binarySearch(int first, int last, float val);
	void addNewBin(int pos, Bin b);
	void insToMc(float val);

  public:
  	SlidingMc();
	~SlidingMc();
	SlidingMc(const SlidingMc &smc);
	void setMaxBins(int maxbins);
	float getMedian();
	void insert(float val);
	void del(float val);
};

#endif
