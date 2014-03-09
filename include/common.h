#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>

#define NUM_HOUSE 40
#define NUM_PLUGS 2125

// input struct
struct measurement
{
	float value;
	unsigned char property;
	unsigned timestamp, plug_id, household_id, house_id;

	measurement(unsigned t=-1, float v=-1, unsigned char p=-1, unsigned i=-1, unsigned hh=-1, unsigned h=-1) :
		value(v), property(p), timestamp(t), plug_id(i), household_id(h), house_id(h) {}
};
typedef struct measurement measurement;

#endif
