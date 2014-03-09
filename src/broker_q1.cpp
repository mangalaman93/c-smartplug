#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "common.h"
#include <cstdio>
using namespace std;

// arg: #houses datafile port
int main(int argc, char *argv[])
{
    int subscribers_expected, port;
    string data_file;

    if(argc < 4)
    {
        cout<<"not enough arguments!"<<endl;
        exit(-1);
    } else
    {
        subscribers_expected  = atoi(argv[1]);
        data_file = string(argv[2]);
        port = atoi(argv[3]);
    }

    int listenfd=0, connfd=0, n=0;
    struct sockaddr_in serv_addr;
    char send_buffer[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(send_buffer, '0', sizeof(send_buffer));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // listening on the socket
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, subscribers_expected);

    int *con_map;
    con_map = new int[subscribers_expected];

    unsigned id;
    int house_id;

    // get synchronization from subscribers
    int subscribers = 0;
    while(subscribers < subscribers_expected)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if((n = read(connfd, &house_id, sizeof(int))) > 0)
        {
            if(house_id < subscribers_expected && house_id >= 0)
            {
                con_map[house_id] = connfd;
            } else
            {
                cout<<"error occurred: wrong house id!"<<endl;
                delete[] con_map;
                close(listenfd);
                exit(-1);
            }
        }

        subscribers++;
    }

    FILE * ifile = fopen(data_file.c_str(), "r");
    measurement m;

    unsigned long ptime = time(NULL), ctime, count=0, stat=1000000;

    while(!feof(ifile))
    {
        if(fscanf(ifile, "%u,%u,%f,%c,%u,%u,%u", &id, &m.timestamp, &m.value, &m.property, &m.plug_id, &m.household_id, &house_id) < 7)
            continue;

        // send the message
        if ( house_id < subscribers )
        {
            write(con_map[house_id], &m, sizeof(m));
            count++;
        }
        if(count == stat)
        {
            ctime = time(NULL);
            cerr<<"Throughput = "<<count/(ctime-ptime+1)<<endl;
            ptime = ctime;
            count = 0;
        }
    }


    for(int i=0; i<subscribers_expected; i++)
        close(con_map[i]);
    delete[] con_map;
    close(listenfd);
}
