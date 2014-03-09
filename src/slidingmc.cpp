#include "slidingmc.h"

SlidingMc::SlidingMc()
{
	size = 0;
	num_bins = 0;
	bins = new Bin[MAXNUM_BINS+2];
	bins = &(bins[1]);

	cur_median_index = -1;
	cum_sum = 0;
}

SlidingMc::SlidingMc(const SlidingMc &smc)
{
	size = smc.size;
	num_bins = smc.num_bins;

	bins = new Bin[MAXNUM_BINS+2];
	bins = &(bins[1]);

	cur_median_index = smc.cur_median_index;
	cum_sum = smc.cum_sum;

	for(int i=0; i<num_bins; i++)
	{
		bins[i].val = smc.bins[i].val;
		bins[i].freq = smc.bins[i].freq;
	}
}

float SlidingMc::findMedian()
{
	int mindex = (size+1)/2;

	if(cum_sum < mindex)
	{
		while(cum_sum < mindex)
		{
			cur_median_index++;
			cum_sum += bins[cur_median_index].freq;
		}
	} else if(cum_sum > mindex)
	{
		int temp = cum_sum - bins[cur_median_index].freq;
		while(temp > mindex)
		{
			cur_median_index--;
			cum_sum = temp;
			temp -= bins[cur_median_index].freq;
		}
	}

	if(cur_median_index == num_bins-1)
		return bins[cur_median_index].val;
	else if(cur_median_index == 0)
		return (bins[cur_median_index].val + bins[cur_median_index+1].val)/2;
	else if(size == cum_sum*2)
		return (bins[cur_median_index-1].val/4
				+ bins[cur_median_index].val/2
				+ bins[cur_median_index+1].val/4);
	else
		return (bins[cur_median_index].val + bins[cur_median_index+1].val)/2;
}

int SlidingMc::binarySearch(float val)
{
	if(num_bins == 0)
		return -1;

	if(val < bins[0].val)
		return -1;

	if(val > bins[num_bins-1].val)
		return num_bins;

	return binarySearch(0, num_bins, val);
}

int SlidingMc::binarySearch(int first, int last, float val)
{
	int mid = (last + first)/2;
	if(mid == first)
		return first;
	else
	{
		if(val == bins[mid].val)
			return mid;
		else if(val > bins[mid].val)
			return binarySearch(mid, last, val);
		else
			return binarySearch(first, mid, val);
	}
}

void SlidingMc::addNewBin(int pos, Bin b)
{
	if(pos+1 >= num_bins)
	{
		cout<<"should not reach at "<<__LINE__<<"in file: "<<__FILE__<<endl;
		exit(-1);
	}

	if(cur_median_index > pos)
	{
		cur_median_index++;
		cum_sum += b.freq;
	}

	memmove(&(bins[pos+2]), &(bins[pos+1]), sizeof(Bin)*(num_bins-pos-1));
	bins[pos+1] = b;
	num_bins++;
}

void SlidingMc::insert(float val)
{
	int pos = binarySearch(val);
	size++;
	bool flag = false;

	if(pos == -1)
	{
		bins = bins - 1;
		bins[0] = Bin(val, 1);
		num_bins++;
		flag = true;

		cum_sum++;
		cur_median_index++;
	} else if(pos > num_bins)
	{
		cout<<"error occured: binarySearch is wrong!"<<endl;
		exit(-1);
	} else if(pos == num_bins)
	{
		bins[pos] = Bin(val, 1);
		num_bins++;
	} else
	{
		if(bins[pos].val == val || MAXNUM_BINS <= num_bins)
		{
			if(pos <= cur_median_index)
				cum_sum++;
			bins[pos].freq++;

			if(num_bins >=2 && bins[pos].freq > 10*size/num_bins)
			{
				if(pos == 0)
				{
					bins = bins-1;
					flag = true;
					cur_median_index++;

					bins[0].val = bins[1].val;
					bins[1].val = (bins[0].val + bins[2].val)/2;
					bins[0].freq = bins[1].freq/2;
					bins[1].freq -= bins[0].freq;
				} else if(pos == num_bins-1)
				{
					bins[num_bins].val = bins[num_bins-1].val;
					bins[num_bins-1].val = (bins[num_bins-2].val + bins[num_bins].val)/2;
					bins[num_bins].freq = bins[num_bins-1].freq/2;
					bins[num_bins-1].freq -= bins[num_bins].freq;

					if(cur_median_index == num_bins-1)
						cur_median_index++;
				} else if(pos*2 < num_bins)
				{
					memmove(bins-1, bins, sizeof(Bin)*pos);
					bins = bins-1;
					flag = true;

					if(cur_median_index >= pos)
						cur_median_index++;

					bins[pos].val = (bins[pos-1].val + bins[pos+1].val)/2;
					bins[pos].freq = bins[pos+1].freq/2;
					bins[pos+1].freq -= bins[pos].freq;
				} else
				{
					memmove(&(bins[pos+2]), &(bins[pos+1]), sizeof(Bin)*(num_bins-pos-1));

					bins[pos+1].val = bins[pos].val;
					bins[pos].val = (bins[pos-1].val + bins[pos+1].val)/2;
					bins[pos+1].freq = bins[pos].freq/2;
					bins[pos].freq -= bins[pos+1].freq;

					if(cur_median_index >= pos)
						cur_median_index++;
				}

				num_bins++;
			}
		}
		else
			addNewBin(pos, Bin(val, 1));
	}

	// merge bins
	if(num_bins == MAXNUM_BINS+1)
	{
		int min = bins[0].freq + bins[1].freq, sum, index=0;
		for(int i=1; i<MAXNUM_BINS; i++)
		{
			sum = bins[i].freq + bins[i+1].freq;
			if(min > sum)
			{
				min = sum;
				index = i;
			}
		}

		if(cur_median_index > index)
			cur_median_index--;
		else if(cur_median_index == index)
			cum_sum += bins[cur_median_index+1].freq;

		if(flag)
		{
			memmove(&(bins[1]), &(bins[0]), sizeof(Bin)*(index+1));
			bins[index+1].freq = min;
			bins = bins + 1;
		} else
		{
			bins[index].freq = min;
			memmove(&(bins[index+1]), &(bins[index+2]), sizeof(Bin)*(num_bins-index-2));
		}
		num_bins--;
		flag = false;
	}

	if(flag)
	{
		memmove(&(bins[1]), &(bins[0]), sizeof(Bin)*(num_bins));
		bins = bins + 1;
	}
}

void SlidingMc::del(float val)
{
	int pos = binarySearch(val);
	if(pos < 0)
		pos = 0;

	if(pos == num_bins)
		pos--;
	bins[pos].freq--;
	size--;

	if(cur_median_index >= pos)
		cum_sum--;

	if(bins[pos].freq == 0)
	{
		memmove(&(bins[pos]), &(bins[pos+1]), sizeof(Bin)*(num_bins-pos-1));
		num_bins--;

		if(cur_median_index >= pos)
			cur_median_index--;
	}
}

float SlidingMc::getMedian()
{

	if(size == 0)
		return -1;
	else
		return findMedian();
}

SlidingMc::~SlidingMc()
{
	bins =  bins - 1;
	delete[] bins;
}
