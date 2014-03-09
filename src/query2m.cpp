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
    DELETE,
    BOTH,
    EXIT,
    NONE
}

struct QueueNode
{
    measurement mt;
    QueueNode* next;
    float global_median;
    TypeEvent type;

    QueueNode(measurement m=measurement(), QueueNode* n=NULL, float gm=-1, TypeEvent e=NONE)
    : mt(m), next(n), global_median(gm), type(t) {}
};
typedef struct QueueNode QueueNode;

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
    sleep(2);

    QueueNode* ch_node = my_data->queue;
    int house_id = my_data->house_id;

    while(true)
    {
        if(ch_node->next != NULL)
        {
            QueueNode *node = ch_node;
            ch_node = ch_node->next;
            delete node;
        } else if(ch_node->type == EXIT)
        {
            delete ch_node;
            break;
        }
    }

    pthread_exit(NULL);
}

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

            unsigned ts = hr_begin_node[i]->mt.timestamp;
            if(ts + getWindowSize(ws) <= current_node->mt.timestamp)
            {
                global_median[i].del(hr_begin_node[i]->mt.value);

                Node* old_hr_begin_node = hr_begin_node[i];
                hr_begin_node[i] = hr_begin_node[i]->next;
                if(i == NUM_WINDOWS-1)
                    delete old_hr_begin_node;
            }

            if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
            {
                global_median[i].insert(m->value);
            }

            float new_median = global_median[i].getMedian();

            if(fabs(new_median - old_median) > 0.0001)
            {
                for(int h=0; h<NUM_HOUSE; h++)
                {

                }
            } else
            {

            }

            if(ts + getWindowSize(ws) >= current_node->mt.timestamp)
                break;
        }
    }

    current_node = current_node->next;

    // passing event to house threads
    current_house_node[m->house_id]->mt = *m;
    QueueNode *n = new QueueNode();
    current_house_node[m->house_id]->next = n;
    current_house_node[m->house_id] = n;
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
        current_house_node[i] = new QueueNode();

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
        current_house_node[h]->should_exit = true;
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
    }

    delete[] current_house_node;
    delete m;
    close(sockfd);
    return 0;
}
