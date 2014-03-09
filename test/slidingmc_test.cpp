#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>
#include <ctime>

#include "../src/slidingmc.cpp"
#include "../include/slidingmc.h"

class SlidingMcTest : public CppUnit::TestFixture
{
  protected:
  	SlidingMc *sm;

  public:
	void setUp()
	{
		sm = new SlidingMc();
	}

	void tearDown()
	{
		delete sm;
	}

	void test1()
	{
		for(float i=1; i<4000; i++)
			sm->insert(i);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(1999, sm->getMedian(), 0.00001);
	}

	void test2()
	{
		for(float i=1; i<80000; i++)
			sm->insert(i);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(39978, sm->getMedian(), 0.00001);
	}

	void test3()
	{
		for(float i=1; i<90000; i++)
			sm->insert(i);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(44989, sm->getMedian(), 0.00001);
	}

	void test4()
	{
		for(float i=1; i<9; i++)
			sm->insert(i);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(4, sm->getMedian(), 0.00001);
	}


	void test6()
	{
		for(float i=1; i<2; i++)
			sm->insert(i);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(1, sm->getMedian(), 0.00001);
	}

	void test_overlap1()
	{
		for(float i=1; i<24*3600+1; i++)
			sm->insert(i);
		sm->insert(100);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(43154.5, sm->getMedian(), 0.00001);
	}

	void test_overlap2()
	{
		for(float i=1; i<24*3600+1; i++)
			sm->insert(24*3600-i);
		sm->insert(100);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(43232, sm->getMedian(), 0.00001);
	}

	void test_random1()
	{
		for(float i=1; i<65584; i++)
			sm->insert(rand()%100+50);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(100.5, sm->getMedian(), 0.00001);
	}

	void test_random2()
	{
		for(float i=1; i<65584; i++)
			sm->insert((float)(rand()%10000)/100);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(49.985, sm->getMedian(), 0.00001);
	}

	void test_random3()
	{
		for(float i=1; i<65584; i++)
			sm->insert((float)(rand()%50000)/100);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(249.675, sm->getMedian(), 0.00001);
	}

	void test_del1()
	{
		for(float i=1; i<4000; i++)
			sm->insert(i);
		for(float i=1; i<4000; i++)
			sm->del(i);

		CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, sm->getMedian(), 0.00001);
	}

	void test_del2()
	{
		for(float i=1; i<80000; i++)
			sm->insert(i);
		for(float i=1; i<80000; i++)
			sm->del(i);

		CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, sm->getMedian(), 0.00001);
	}

	void test_del3()
	{
		for(float i=1; i<80000; i++)
			sm->insert(i);
		for(float i=1; i<40000; i++)
			sm->del(i);

		CPPUNIT_ASSERT_DOUBLES_EQUAL(60009.5, sm->getMedian(), 0.00001);
	}

	void test_del4()
	{
		for(float i=1; i<80000; i++)
			sm->insert(i);
		for(float i=40001; i<80000; i++)
			sm->del(i);

		CPPUNIT_ASSERT_DOUBLES_EQUAL(19993, sm->getMedian(), 0.00001);
	}

	void test_highfreq()
	{
		for(float i=1; i<80000; i++)
			sm->insert(i);

		for(int i=0; i<40000; i++)
			sm->insert(40000);

		CPPUNIT_ASSERT_DOUBLES_EQUAL(39931, sm->getMedian(), 0.00001);
	}

	CPPUNIT_TEST_SUITE(SlidingMcTest);
	CPPUNIT_TEST(test1);
	CPPUNIT_TEST(test2);
	CPPUNIT_TEST(test3);
	CPPUNIT_TEST(test4);
	CPPUNIT_TEST(test6);
	CPPUNIT_TEST(test_overlap1);
	CPPUNIT_TEST(test_overlap2);
	CPPUNIT_TEST(test_random1);
	CPPUNIT_TEST(test_random2);
	CPPUNIT_TEST(test_random3);
	CPPUNIT_TEST(test_del1);
	CPPUNIT_TEST(test_del2);
	CPPUNIT_TEST(test_del3);
	CPPUNIT_TEST(test_del4);
	CPPUNIT_TEST(test_highfreq);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SlidingMcTest);

int main()
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool wasSucessful = runner.run("", true);
	return wasSucessful;
}
