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
#include <pthread.h>
#include <cmath>
#include <semaphore.h>
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

enum TypeEvent
{
    INSERT = 0,
    GLOBAL_CHANGED,
    DELETE,
    BOTH,
    EXIT,
    NONE
};

struct QueueNode
{
    measurement mt;
    QueueNode* next;
    float global_median;
    TypeEvent type;
    Window ws;
    long int times;

    QueueNode(measurement m=measurement(), QueueNode* n=NULL, float gm=-1, TypeEvent te=NONE, Window w=WINDOW_1HR, long int t=-1)
    : mt(m), next(n), global_median(gm), type(te), ws(w), times(t) {}
};
typedef struct QueueNode QueueNode;

// shared variables
pthread_mutex_t mutex[NUM_HOUSE];
sem_t empty[NUM_HOUSE];

struct ThreadData
{
    QueueNode *queue;
    int house_id;

    ThreadData(QueueNode *q, int h)
    : queue(q), house_id(h) {}
};

void* solveHouse(void *threadarg)
{
    struct ThreadData *my_data = (struct ThreadData*) threadarg;
    unordered_map<unsigned,
        unordered_map<unsigned,
            SlidingMc> >
    mc[NUM_WINDOWS];
    int num_percentage_more[NUM_WINDOWS] = {0};
    SCont msc[NUM_WINDOWS];

    QueueNode* ch_node = my_data->queue;
    int house_id = my_data->house_id;

    while(true)
    {
        TypeEvent te;
        pthread_mutex_lock(&mutex[house_id]);
        te = ch_node->type;
        pthread_mutex_unlock(&mutex[house_id]);
        if(te == GLOBAL_CHANGED)
        {
            int i = (int) ch_node->ws;

            int old_percentage = num_percentage_more[i];
            num_percentage_more[i] = msc[i].getNumOfLargeNum(ch_node->global_median);

            if(old_percentage != num_percentage_more[i])
            {
                cout << (ch_node->times+1-getWindowSize(ch_node->ws)) << "," << ch_node->times << "," << house_id << ","
                             << num_percentage_more[i]/(msc[i].getSize()/100.0) <<endl;
            }
            QueueNode *node = ch_node;
            ch_node = ch_node->next;
            delete node;
        } else if(te == DELETE)
        {
            int i = (int) ch_node->ws;

            unsigned household_id =  ch_node->mt.household_id;
            unsigned plug_id =  ch_node->mt.plug_id;

            // initializing
            mc[i][household_id][plug_id];

            float old_plug_median = mc[i][household_id][plug_id].getMedian();
            mc[i][household_id][plug_id].insert(ch_node->mt.value);
            float new_plug_median = mc[i][household_id][plug_id].getMedian();
            msc[i].insert(household_id, plug_id, new_plug_median, old_plug_median);

            int old_percentage = num_percentage_more[i];
            num_percentage_more[i] = msc[i].getNumOfLargeNum(ch_node->global_median);

            if(old_percentage != num_percentage_more[i])
            {
                cout << (ch_node->times+1-getWindowSize(ch_node->ws)) << "," << ch_node->times << "," << house_id << ","
                             << num_percentage_more[i]/(msc[i].getSize()/100.0) <<endl;
            }

            QueueNode *node = ch_node;
            ch_node = ch_node->next;
            delete node;
        } else if(te == BOTH)
        {
            int i = (int) ch_node->ws;

            unsigned household_id =  ch_node->mt.household_id;
            unsigned plug_id =  ch_node->mt.plug_id;

            // initializing
            mc[i][household_id][plug_id];

            float old_plug_median = mc[i][household_id][plug_id].getMedian();
            mc[i][household_id][plug_id].del(ch_node->mt.value);
            float new_plug_median = mc[i][household_id][plug_id].getMedian();
            msc[i].insert(household_id, plug_id, new_plug_median, old_plug_median);

            QueueNode *node = ch_node;
            ch_node = ch_node->next;
            delete node;
        } else if(te == INSERT)
        {
            int i = (int) ch_node->ws;
            unsigned household_id =  ch_node->mt.household_id;
            unsigned plug_id =  ch_node->mt.plug_id;

            // initializing
            mc[i][household_id][plug_id];

            float old_plug_median = mc[i][household_id][plug_id].getMedian();
            mc[i][household_id][plug_id].del(ch_node->mt.value);
            float new_plug_median = mc[i][household_id][plug_id].getMedian();
            msc[i].insert(household_id, plug_id, new_plug_median, old_plug_median);

            int old_percentage = num_percentage_more[i];
            num_percentage_more[i] = msc[i].getNumOfLargeNum(ch_node->global_median);

            if(old_percentage != num_percentage_more[i])
            {
                int i = (int) ch_node->ws;
                cout << (ch_node->times+1-getWindowSize(ch_node->ws)) << "," << ch_node->times << "," << house_id << ","
                             << num_percentage_more[i]/(msc[i].getSize()/100.0) <<endl;
            }

            QueueNode *node = ch_node;
            ch_node = ch_node->next;
            delete node;
        } else if(te == NONE)
        {
            struct timespec ts;
            ts.tv_sec += 2;
            clock_gettime(CLOCK_REALTIME, &ts);
            sem_wait(&empty[house_id]);
        } else if(te == EXIT)
        {
            delete ch_node;
            break;
        }
    }

    pthread_exit(NULL);
}

void sendEvent(int house_id, TypeEvent event, QueueNode **current_house_node, measurement m,
                Window ws, long int tstamp, float global_median){
    // passing event to house threads
    current_house_node[house_id]->mt = m;
    current_house_node[house_id]->ws = ws;
    current_house_node[house_id]->times = tstamp;
    current_house_node[house_id]->global_median = global_median;

    QueueNode *n = new QueueNode();
    current_house_node[house_id]->next = n;
    pthread_mutex_lock(&mutex[house_id]);
    current_house_node[house_id]->type = event;
    pthread_mutex_unlock(&mutex[house_id]);

    sem_post(&empty[house_id]);
    current_house_node[house_id] = n;
}

// main thread gloabls
Node* current_node;
Node* hr_begin_node[NUM_WINDOWS];
SlidingMc global_median[NUM_WINDOWS];

void solveQuery2(measurement *m, QueueNode** current_house_node)
{
    current_node->mt = *m;
    current_node->next = new Node();

    for(int i=0; i<NUM_WINDOWS; i++)
    {
        Window ws = (Window)i;
        while(true)
        {
            float old_median = global_median[i].getMedian();
usleep(10);
            unsigned ts = hr_begin_node[i]->mt.timestamp;
            if(ts + getWindowSize(ws) <= current_node->mt.timestamp)
                global_median[i].del(hr_begin_node[i]->mt.value);

            if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
                global_median[i].insert(m->value);

            float new_median = global_median[i].getMedian();

            if(ts + getWindowSize(ws) < current_node->mt.timestamp)
                //only delete happened
                sendEvent(hr_begin_node[i]->mt.house_id, DELETE, current_house_node, hr_begin_node[i]->mt, ws, (long int)ts, new_median);
            else if(ts + getWindowSize(ws) > current_node->mt.timestamp)
                //only insert happened
                sendEvent(m->house_id, INSERT, current_house_node, *m, ws, (long int)current_node->mt.timestamp, new_median);
            else if(ts + getWindowSize(ws) == current_node->mt.timestamp)
            {
                //both insert and happened
                if(m->house_id == hr_begin_node[i]->mt.house_id)
                {
                    //happened to same house
                    sendEvent(hr_begin_node[i]->mt.house_id, BOTH, current_house_node, hr_begin_node[i]->mt, ws, (long int)ts, new_median);
                    sendEvent(m->house_id, INSERT, current_house_node, *m, ws, (long int)current_node->mt.timestamp, new_median);
                } else
                {
                    //happened to different houses
                    sendEvent(hr_begin_node[i]->mt.house_id, DELETE, current_house_node, hr_begin_node[i]->mt, ws, (long int)ts, new_median);
                    sendEvent(m->house_id, INSERT, current_house_node, *m, ws, (long int)current_node->mt.timestamp, new_median);
                }
            }

            if(fabs(new_median - old_median) > 0.001)
            {
                long int pass_ts = 0;
                if(ts + getWindowSize(ws) <= current_node->mt.timestamp)
                    pass_ts = (long int)ts;
                else
                    pass_ts = (long int)current_node->mt.timestamp;

                //global median changed
                for(unsigned h=0; h<NUM_HOUSE; h++)
                    //pass event to everybody except to m->house_id and  hr_begin_node[i]->mt.house_id
                    if(h != m->house_id && h != hr_begin_node[i]->mt.house_id)
                        sendEvent(h, GLOBAL_CHANGED, current_house_node, measurement(), ws, pass_ts, new_median);
            }

            if(ts + getWindowSize(ws) <= current_node->mt.timestamp)
            {
                Node* old_hr_begin_node = hr_begin_node[i];
                hr_begin_node[i] = hr_begin_node[i]->next;
                if(i == NUM_WINDOWS-1)
                    delete old_hr_begin_node;
            }

            if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
                break;
        }
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

    // house queues
    QueueNode **current_house_node;
    current_house_node = new QueueNode*[NUM_HOUSE];
    for(int i=0; i<NUM_HOUSE; i++)
    {
        current_house_node[i] = new QueueNode();
        pthread_mutex_init(&mutex[i], NULL);
        if(sem_init(&empty[i], 0, 0) == -1)
        {
            cout<<"unable to initialise semaphore!"<<endl;
            exit(-2);
        }
    }

    // creating threads
    pthread_t threads[NUM_HOUSE];
    struct ThreadData* td[NUM_HOUSE];
    pthread_attr_t attr;
    void *status;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(int h=0; h<NUM_HOUSE; h++)
    {
        td[h] = new struct ThreadData(current_house_node[h], h);
        int rc = pthread_create(&threads[h], NULL, solveHouse, (void *)td[h]);
        if(rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
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
            solveQuery2(m, current_house_node);
        } else if(n < sizeof(measurement))
        {
            char b[sizeof(measurement)];
            memcpy(b, &m, n);

            unsigned nrest = read(sockfd, m, sizeof(measurement) - n);
            if(nrest + n == sizeof(measurement))
            {
                memcpy(b+n, &m, nrest);
                solveQuery2(m, current_house_node);
            } else
            {
                cout<<"Error: should not reach here lineno:"<<__LINE__<<"!"<<endl;
                delete m;
                close(sockfd);
                exit(-1);
            }
        } else
        {
            cout<<"Error: should not reach here lineno:"<<__LINE__<<"!"<<endl;
            delete m;
            close(sockfd);
            exit(-1);
        }
    }

    // send exit signal to threads
    for(int h=0; h<NUM_HOUSE; h++)
    {
        pthread_mutex_lock(&mutex[h]);
        current_house_node[h]->type = EXIT;
        pthread_mutex_unlock(&mutex[h]);
        sem_post(&empty[h]);
    }

    pthread_attr_destroy(&attr);
    for(int h=0; h<NUM_HOUSE; h++)
    {
        int rc = pthread_join(threads[h], &status);
        if(rc)
        {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
        delete td[h];
        pthread_mutex_destroy(&mutex[h]);
    }

    delete[] current_house_node;
    delete m;
    close(sockfd);
    return 0;
}
