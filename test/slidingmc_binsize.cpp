#include <iostream>
#include <random>
using namespace std;

#include "mc.h"
#include "slidingmc.h"

int main()
{
    // number of experiments
    const int num_exp = 2000;
    const int nrolls = 40000;

    for(int k=0; k<10; k++)
    {
        default_random_engine generator;
        normal_distribution<float> distribution(rand()%1000, rand()%1000);
        distribution.reset();

        float number[nrolls];
        for(int j=0; j<nrolls; j++)
        {
            number[j] = distribution(generator);
        }
        for(int i=100; i<=num_exp; i++)
        {
            Mc m1;
            SlidingMc m2(i);

            for(int j=0; j<nrolls; j++)
            {
                m1.insert(number[j]);
                m2.insert(number[j]);
            }

            cout<<abs((m2.getMedian() - m1.getMedian())/m1.getMedian()*100)<<endl;
        }
    }

    return 0;
}
