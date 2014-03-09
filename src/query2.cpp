#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "common.h"
#include "slidingmc.h"
#include "scont.h"
using namespace std;

enum Window
{
    WINDOW_1HR = 0,
    WINDOW_24HR,
    NUM_WINDOWS
};

unsigned getWindowSize(Window ws)
{
    switch(ws)
    {
        case WINDOW_1HR:
            return 60*60;
        case WINDOW_24HR:
            return 24*3600;
        default:
            cout<<"error occured!"<<endl;
            exit(-1);
    }
}

struct Node
{
    measurement mt;
    Node* next;

    Node(measurement m = measurement(), Node* n=NULL)
    : mt(m), next(n) {}
};
typedef struct Node Node;

unordered_map<unsigned, unordered_map<unsigned, SlidingMc> > mc[NUM_WINDOWS][NUM_HOUSE];
SlidingMc global_median[NUM_WINDOWS];
int num_percentage_more[NUM_WINDOWS][NUM_HOUSE] = {0};
SCont msc[NUM_WINDOWS][NUM_HOUSE];
Node* hr_begin_node[NUM_WINDOWS];

Node* current_node;

void solveQuery2(measurement *m)
{
    // struct timeval tbegin, tend;
    // gettimeofday(&tbegin, NULL);

    long unsigned latency[1000000];
    int c = 0;

    current_node->mt = *m;
    current_node->next = new Node();

    for(int i=0; i<NUM_WINDOWS; i++)
    {
        Window ws = (Window)i;
        while(true)
        {
            unsigned house_id = hr_begin_node[i]->mt.house_id;
            unsigned household_id =  hr_begin_node[i]->mt.household_id;
            unsigned plug_id =  hr_begin_node[i]->mt.plug_id;

            // initializing
            mc[i][house_id][household_id][plug_id];
            mc[i][m->house_id][m->household_id][m->plug_id];

            float old_median = global_median[i].getMedian();

            unsigned ts = hr_begin_node[i]->mt.timestamp;
            if(ts + getWindowSize(ws) <= current_node->mt.timestamp)
            {
                float old_plug_median = mc[i][house_id][household_id][plug_id].getMedian();

                global_median[i].del(hr_begin_node[i]->mt.value);
                mc[i][house_id][household_id][plug_id].del(hr_begin_node[i]->mt.value);

                float new_plug_median = mc[i][house_id][household_id][plug_id].getMedian();
                msc[i][house_id].insert(household_id, plug_id, new_plug_median, old_plug_median);

                Node* old_hr_begin_node = hr_begin_node[i];
                hr_begin_node[i] = hr_begin_node[i]->next;
                if(i == NUM_WINDOWS-1)
                    delete old_hr_begin_node;
            }

            if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
            {
                float old_plug_median = mc[i][m->house_id][m->household_id][m->plug_id].getMedian();

                global_median[i].insert(m->value);
                mc[i][m->house_id][m->household_id][m->plug_id].insert(m->value);

                float new_plug_median = mc[i][m->house_id][m->household_id][m->plug_id].getMedian();
                msc[i][m->house_id].insert(m->household_id, m->plug_id, new_plug_median, old_plug_median);
            }

            float new_median = global_median[i].getMedian();
            if(old_median == new_median)
            {
                int old_percentage = num_percentage_more[i][house_id];
                num_percentage_more[i][house_id] = msc[i][house_id].getNumOfLargeNum(new_median);

                if(old_percentage != num_percentage_more[i][house_id])
                {
                    if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
                    {
                        // gettimeofday(&tend, NULL);
                        // latency[c] = (tend.tv_sec-tbegin.tv_sec)*1000000 + tend.tv_usec-tbegin.tv_usec;
                        // c++;
                        long int times = (long int)current_node->mt.timestamp;
                        cout << (times+1-getWindowSize(ws)) << "," << times << "," << house_id << ","
                             << num_percentage_more[i][house_id]/(msc[i][house_id].getSize()/100.0) <<endl;
                    }
                    else
                    {
                        // gettimeofday(&tend, NULL);
                        // latency[c] = (tend.tv_sec-tbegin.tv_sec)*1000000 + tend.tv_usec-tbegin.tv_usec;
                        // c++;
                        cout << (ts+1) << "," << (ts + getWindowSize(ws)) << "," << house_id << ","
                             << num_percentage_more[i][house_id]/(msc[i][house_id].getSize()/100.0) <<endl;
                    }
                }

                old_percentage = num_percentage_more[i][m->house_id];
                num_percentage_more[i][m->house_id] = msc[i][m->house_id].getNumOfLargeNum(new_median);

                if(old_percentage != num_percentage_more[i][m->house_id])
                {
                    if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
                    {
                        // gettimeofday(&tend, NULL);
                        // latency[c] = (tend.tv_sec-tbegin.tv_sec)*1000000 + tend.tv_usec-tbegin.tv_usec;
                        // c++;
                        long int times = (long int)current_node->mt.timestamp;
                        cout << (times+1-getWindowSize(ws)) << "," << times << "," << m->house_id << ","
                             << num_percentage_more[i][m->house_id]/(msc[i][m->house_id].getSize()/100.0) <<endl;
                    }
                    else
                    {
                        // gettimeofday(&tend, NULL);
                        // latency[c] = (tend.tv_sec-tbegin.tv_sec)*1000000 + tend.tv_usec-tbegin.tv_usec;
                        // c++;
                        cout << (ts+1) << "," << (ts + getWindowSize(ws)) << "," << m->house_id << ","
                             << num_percentage_more[i][m->house_id]/(msc[i][m->house_id].getSize()/100.0) <<endl;
                    }
                }
            } else
            {
                for(int h=0; h<NUM_HOUSE; h++)
                {
                    int old_percentage = num_percentage_more[i][h];
                    num_percentage_more[i][h] = msc[i][h].getNumOfLargeNum(new_median);

                    if(old_percentage != num_percentage_more[i][h])
                    {
                        if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
                        {
                            // gettimeofday(&tend, NULL);
                            // latency[c] = (tend.tv_sec-tbegin.tv_sec)*1000000 + tend.tv_usec-tbegin.tv_usec;
                            // c++;
                            long int times = (long int)current_node->mt.timestamp;
                            cout << (times+1-getWindowSize(ws)) << "," << times << "," << h << ","
                                 << num_percentage_more[i][h]/(msc[i][h].getSize()/100.0) <<endl;
                        }
                        else
                        {
                            // gettimeofday(&tend, NULL);
                            // latency[c] = (tend.tv_sec-tbegin.tv_sec)*1000000 + tend.tv_usec-tbegin.tv_usec;
                            // c++;
                            cout << (ts+1) << "," << (ts + getWindowSize(ws)) << "," << h << ","
                                 << num_percentage_more[i][h]/(msc[i][h].getSize()/100.0) <<endl;
                        }
                    }
                }
            }

            if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
                break;
        }

        for(int i=0; i<c; i++)
            cerr<<latency[i]<<endl;
    }

    current_node = current_node->next;
}

// arg: broker_ip port
int main(int argc, char *argv[])
{
    int port;

    if(argc < 3)
    {
        cout<<"not enough arguments!"<<endl;
        exit(-1);
    } else
        port = atoi(argv[2]);

    int sockfd = 0;
    unsigned n = 0;
    char recv_buff[1024];
    struct sockaddr_in serv_addr;

    memset(recv_buff, '0',sizeof(recv_buff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }

    // init
    current_node = new Node();
    for(int i=0; i<NUM_WINDOWS; i++)
        hr_begin_node[i] = current_node;

    measurement *m = new measurement;
    while((n = read(sockfd, m, sizeof(measurement))) > 0)
    {
        if(n == sizeof(measurement))
        {
            solveQuery2(m);
        } else if(n < sizeof(measurement))
        {
            char b[sizeof(measurement)];
            memcpy(b, &m, n);

            unsigned nrest = read(sockfd, m, sizeof(measurement) - n);
            if(nrest + n == sizeof(measurement))
            {
                memcpy(b+n, &m, nrest);
                solveQuery2(m);
            } else
            {
                cout<<"Error: should not reah here lineno:"<<__LINE__<<"!"<<endl;
                delete m;
                close(sockfd);
                exit(-1);
            }
        } else
        {
            cout<<"Error: should not reah here lineno:"<<__LINE__<<"!"<<endl;
            delete m;
            close(sockfd);
            exit(-1);
        }
    }

    delete m;
    close(sockfd);
    return 0;
}
