#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>
#include <ctime>

#include "../src/scont.cpp"
#include "../include/scont.h"

class SContTest : public CppUnit::TestFixture
{
  protected:
  	SCont *sct;

  public:
	void setUp()
	{
		sct = new SCont();
	}

	void tearDown()
	{
		delete sct;
	}

	void test1()
	{
		int i =0;
		for(int hh_id=0; hh_id<10; hh_id++)
			for(int pid=0; pid<5; pid++)
			{
				sct->insert(hh_id, pid, i, -1);
				i++;
			}

		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, sct->getNumOfLargeNum(50), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(24, sct->getNumOfLargeNum(25), 0.00001);
	}

	void test2()
	{
		int i =0;
		for(int hh_id=0; hh_id<10; hh_id++)
			for(int pid=0; pid<5; pid++)
			{
				float j = i;
				sct->insert(hh_id, pid, j, -1);
				i++;
			}

		sct->insert(0, 0, 0, 0);
		float j = 1;
		sct->insert(0, 1, 1001, j);

		CPPUNIT_ASSERT_DOUBLES_EQUAL(1, sct->getNumOfLargeNum(50), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(25, sct->getNumOfLargeNum(25), 0.00001);

		sct->insert(0, 1, 25, 1001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(24, sct->getNumOfLargeNum(25), 0.00001);
	}

	void test3()
	{
		int i =0;
		for(int hh_id=0; hh_id<10; hh_id++)
			for(int pid=0; pid<5; pid++)
			{
				float j = i;
				sct->insert(hh_id, pid, j, -1);
				i++;
			}

		i = 49;
		float j = 0;
		for(int hh_id=9; hh_id>=0; hh_id--)
			for(int pid=4; pid>=0; pid--)
			{
				sct->insert(hh_id, pid, j, i);
				i--; j = j+1;
			}

		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, sct->getNumOfLargeNum(50), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(24, sct->getNumOfLargeNum(25), 0.00001);
	}


	void test4()
	{
		int i = 49;
		for(int hh_id=0; hh_id<10; hh_id++)
			for(int pid=0; pid<5; pid++)
			{
				sct->insert(hh_id, pid, i, -1);
				i--;
			}

		CPPUNIT_ASSERT_DOUBLES_EQUAL(0, sct->getNumOfLargeNum(50), 0.00001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(24, sct->getNumOfLargeNum(25), 0.00001);
	}

	CPPUNIT_TEST_SUITE(SContTest);
	CPPUNIT_TEST(test1);
	CPPUNIT_TEST(test2);
	CPPUNIT_TEST(test3);
	CPPUNIT_TEST(test4);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SContTest);

int main()
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool wasSucessful = runner.run("", true);
	return wasSucessful;
}
