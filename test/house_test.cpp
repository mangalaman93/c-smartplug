#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>

#include "../src/house.cpp"

measurement event(unsigned int ts, float value, unsigned char prop, unsigned int p_id, unsigned int hh_id)
{
	measurement m = { ts, value, prop, p_id, hh_id};
	return m;
}

class HouseProcessTest : public CppUnit::TestFixture
{
  public:
	void setUp()
	{
	}

	void tearDown()
	{
		state.clear();
		household_aggregate.clear();
		house_aggregate.clear();
		median_container.clear();
		house_median_container.clear();
	}

	void testInitial()
	{
		CPPUNIT_ASSERT(state.size() == 0);
		CPPUNIT_ASSERT(household_aggregate.size() == 0);
		CPPUNIT_ASSERT(house_aggregate.size() == 0);
		CPPUNIT_ASSERT(median_container.size() == 0);
		CPPUNIT_ASSERT(house_median_container.size() == 0);
	}

	//for testing state changes of one non-boundary event for one household
	void testNonBoundaryFirstEvent()
	{
		measurement m = event(1, 1.5, 1, 0, 0);
		doProcessing(&m);
		CPPUNIT_ASSERT(state.size() == 1);
		CPPUNIT_ASSERT(state[m.household_id].size() == 1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT(household_aggregate.size() == 0);
		CPPUNIT_ASSERT(house_aggregate.size() == 0);
		CPPUNIT_ASSERT(median_container.size() == 0);
		CPPUNIT_ASSERT(house_median_container.size() == 0);
	}

	//for testing state changes of one boundary event for one household
	void test30SBoundaryFirstEvent()
	{
		measurement m = event(30, 1.5, 1, 0, 12);
		doProcessing(&m);
		CPPUNIT_ASSERT(state.size() == 1);
		CPPUNIT_ASSERT(state[m.household_id].size() == 1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT(household_aggregate.size() == 1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(household_aggregate[m.household_id][TIMESLICE_1M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(household_aggregate[m.household_id][TIMESLICE_5M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(household_aggregate[m.household_id][TIMESLICE_15M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(household_aggregate[m.household_id][TIMESLICE_60M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(household_aggregate[m.household_id][TIMESLICE_120M].accumulated_load, m.value, 0.00001);
		CPPUNIT_ASSERT(house_aggregate.size() == 0);
		CPPUNIT_ASSERT(median_container.size() == 1); // since forcast_plug accesses it
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, median_container[m.household_id][m.plug_id][TIMESLICE_30S][30].getMedian(), 0.00001);
		CPPUNIT_ASSERT(house_median_container.size() == 0);
	}

	//for testing state changes of one non-boundary event for 29 households
	void testNonBoundaryFirstEvents()
	{
		measurement m;
		for (int i=1; i<30; i++)
		{
			m = event(i, i, 1, i, i);
			doProcessing(&m);
		}
		CPPUNIT_ASSERT(state.size() == 29);
		CPPUNIT_ASSERT(state[m.household_id].size() == 1);
		CPPUNIT_ASSERT(household_aggregate.size() == 0);
		CPPUNIT_ASSERT(house_aggregate.size() == 0);
		CPPUNIT_ASSERT(median_container.size() == 0);
		CPPUNIT_ASSERT(house_median_container.size() == 0);
	}

	//for testing state changes of 29 non-boundary event for one household same plug
	void testNonBoundaryEvents()
	{
		measurement m;
		float sum = 0;
		for (int i=1; i<30; i++)
		{
			m = event(i, i, 1, 0, 0);
			sum += i;
			doProcessing(&m);
		}
		CPPUNIT_ASSERT(state.size() == 1);
		CPPUNIT_ASSERT(state[m.household_id].size() == 1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, sum, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0, 0.00001);
		CPPUNIT_ASSERT(household_aggregate.size() == 0);
		CPPUNIT_ASSERT(house_aggregate.size() == 0);
		CPPUNIT_ASSERT(median_container.size() == 0);
		CPPUNIT_ASSERT(house_median_container.size() == 0);
	}

	//for testing state changes of one boundary event for one household
	void test30SPrePostBoundaryEvents()
	{
		measurement m = event(29, 1.5, 1, 0, 12);
		measurement m1 = event(31, 2.5, 1, 0, 13);

		doProcessing(&m);
		doProcessing(&m1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT(household_aggregate.size() == 1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT(median_container.size() == 1); // since forcast_plug accesses it
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, median_container[m.household_id][m.plug_id][TIMESLICE_1M][30].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, house_median_container[TIMESLICE_1M][30].getMedian(), 0.00001);
	}

	//for testing state changes of one boundary event for one household
	void test60SPrePostBoundaryEvents()
	{
		measurement m = event(59, 1.5, 1, 0, 12);
		measurement m1 = event(61, 2.5, 1, 0, 13);

		doProcessing(&m);
		doProcessing(&m1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT(household_aggregate.size() == 1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, household_aggregate[m.household_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, house_aggregate[TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT(median_container.size() == 1); // since forcast_plug accesses it
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_1M][0].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_1M][0].getMedian(), 0.00001);
	}

	//for testing state changes of one boundary event for one household
	void test300SPrePostBoundaryEvents()
	{
		measurement m = event(299, 1.5, 1, 0, 12);
		measurement m1 = event(301, 2.5, 1, 0, 13);

		doProcessing(&m);
		doProcessing(&m1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_1M][240].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_1M][240].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_5M][0].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_5M][0].getMedian(), 0.00001);
	}

	//for testing state changes of one boundary event for one household
	void test900SPrePostBoundaryEvents()
	{
		measurement m = event(899, 1.5, 1, 0, 12);
		measurement m1 = event(901, 2.5, 1, 0, 13);

		doProcessing(&m);
		doProcessing(&m1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_1M][840].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_1M][840].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_5M][600].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_5M][600].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_15M][0].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_15M][0].getMedian(), 0.00001);
	}

	//for testing state changes of one boundary event for one household
	void test3600SPrePostBoundaryEvents()
	{
		measurement m = event(3599, 1.5, 1, 0, 12);
		measurement m1 = event(3601, 2.5, 1, 0, 13);

		doProcessing(&m);
		doProcessing(&m1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_1M][3540].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_1M][3540].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_5M][3300].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_5M][3300].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_15M][2700].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_15M][2700].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_60M][0].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_60M][0].getMedian(), 0.00001);
	}

	//for testing state changes of one boundary event for one household
	void test7200SPrePostBoundaryEvents()
	{
		measurement m = event(7199, 1.5, 1, 0, 12);
		measurement m1 = event(7201, 2.5, 1, 0, 13);

		doProcessing(&m);
		doProcessing(&m1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_30S].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_1M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_5M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_15M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_60M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, state[m.household_id][m.plug_id][TIMESLICE_120M].accumulated_load, 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_1M][7140].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_1M][7140].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_5M][6900].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_5M][6900].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_15M][6300].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_15M][6300].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_60M][3600].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_60M][3600].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, median_container[m.household_id][m.plug_id][TIMESLICE_120M][0].getMedian(), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(m.value, house_median_container[TIMESLICE_120M][0].getMedian(), 0.00001);
	}

	//for testing correctness of forcast using median container
	void testForcast()
	{
		measurement m;
		int offsets[5] {7200, 3600, 900, 300, 60};
		for (int i=1; i<30; i++)
		{
			cout <<"new event\n";
			for (int j = 7200; j < 7202; j+=30) {
				m = event((i-1)*86400+j-1, i, 1, 0, 0);
				doProcessing(&m);
				m = event((i-1)*86400+j+1, 0, 1, 0, 0);
				doProcessing(&m);
			}
/*	        for (auto& slice:house_median_container)
	        for (auto& ts:slice.second)
       			cout <<"House MC : Slice = "<<slice.first
       				<<" : ts = "<<ts.first
       				<<" : Median = "<< ts.second.getMedian() <<endl;
	        for (auto& hh:median_container)
	        for (auto& plug:hh.second)
	        for (auto& slice:plug.second)
	        for (auto& ts:slice.second)
	        	cout <<"Plug MC : HH = "<<hh.first
	        		<<" : plig = "<<plug.first
	        		<<" : Slice = "<<slice.first
	        		<<" : ts = "<<ts.first
	        		<<" : Median = "<< ts.second.getMedian() <<endl;
*/
			for (int j = 0; j < 1; j++) {
				m = event(i*86400+7200-2*offsets[j], 0.5+i/2.0, 1, 0, 0);
				doProcessing(&m);
				m = event(i*86400+7200-2*offsets[j]+1, 0.5+i/2.0, 1, 0, 0);
				doProcessing(&m);
			}
			CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5+i/2.0, median_container[0][0][5][0].getMedian(), 0.00001);
			CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5+i/2.0, house_median_container[5][0].getMedian(), 0.00001);

		}
	}

	CPPUNIT_TEST_SUITE( HouseProcessTest );
	CPPUNIT_TEST( testInitial );
	CPPUNIT_TEST( testNonBoundaryFirstEvent );
	CPPUNIT_TEST( testNonBoundaryFirstEvents );
	CPPUNIT_TEST( testNonBoundaryEvents );
	CPPUNIT_TEST( test30SBoundaryFirstEvent );
	CPPUNIT_TEST( test30SPrePostBoundaryEvents );
	CPPUNIT_TEST( test60SPrePostBoundaryEvents );
	CPPUNIT_TEST( test300SPrePostBoundaryEvents ); //TODO recheck
	CPPUNIT_TEST( test900SPrePostBoundaryEvents ); //TODO recheck
	CPPUNIT_TEST( test3600SPrePostBoundaryEvents ); //TODO recheck
	CPPUNIT_TEST( test7200SPrePostBoundaryEvents ); //TODO recheck
	CPPUNIT_TEST( testForcast );
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( HouseProcessTest );

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  bool wasSucessful = runner.run("", true);
  return wasSucessful;
}
