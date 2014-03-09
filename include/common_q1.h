#ifndef COMMON_Q1_H
#define COMMON_Q1_H

#include <unordered_map>
#include <map>
using namespace std;

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

// current data for each time slice (query 1)
typedef struct {
	unsigned int	last_timestamp;
	float			accumulated_load;
	unsigned int	count_of_values;
} timeslice_state;

typedef unordered_map<unsigned int, timeslice_state> 	plug_state;
typedef unordered_map<unsigned int, plug_state > 		household_state;
typedef unordered_map<unsigned int, household_state > 	house_state;

typedef enum {TIMESLICE_30S = 0, TIMESLICE_1M, TIMESLICE_5M, TIMESLICE_15M, TIMESLICE_60M, TIMESLICE_120M} timeslice;

const map<unsigned int, unsigned int> timeslice_lengths = {
	{TIMESLICE_30S, 30},
	{TIMESLICE_1M, 60},
	{TIMESLICE_5M, 300},
	{TIMESLICE_15M, 900},
	{TIMESLICE_60M, 3600},
	{TIMESLICE_120M, 7200}
};

timeslice_state initial_timeslice_state = {0,0,0};
plug_state 	initial_plug_state = {
	{TIMESLICE_30S, initial_timeslice_state},
	{TIMESLICE_1M, initial_timeslice_state},
	{TIMESLICE_5M, initial_timeslice_state},
	{TIMESLICE_15M, initial_timeslice_state},
	{TIMESLICE_60M, initial_timeslice_state},
	{TIMESLICE_120M, initial_timeslice_state}
};
household_state initial_household_state;

#endif
