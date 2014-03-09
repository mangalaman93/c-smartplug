#include <iostream>
#include <random>
using namespace std;

#include "mc.h"
#include "slidingmc.h"

int main()
{
    // number of experiments
    const int num_exp = 1000;
    const int nrolls = 50000;

    default_random_engine generator;
    normal_distribution<float> distribution(rand()%1000, rand()%1000);

    for(int i=0; i<num_exp; i++)
    {
        Mc m1;
        SlidingMc m2;

        for(int j=0; j<nrolls; j++)
        {
            float number = distribution(generator);
            m1.insert(number);
            m2.insert(number);
        }

        cout<<abs((m2.getMedian() - m1.getMedian())/m1.getMedian()*100)<<endl;
    }

    return 0;
}
