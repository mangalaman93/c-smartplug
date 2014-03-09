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
#include <cstdio>
#include <ctime>
#include "common.h"
using namespace std;

// arg: port, datafile
int main(int argc, char *argv[])
{
    string data_file;
    int port;

    if(argc < 3)
    {
        cout<<"not enough arguments!"<<endl;
        exit(-1);
    } else
    {
        port = atoi(argv[1]);
        data_file = string(argv[2]);
    }

    int listenfd=0, connfd=0;
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
    listen(listenfd, 2);

    unsigned id;
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    FILE * ifile = fopen(data_file.c_str(), "r");
    measurement m;

    while(!feof(ifile))
    {
        if(fscanf(ifile, "%u,%u,%f,%c,%u,%u,%u", &id, &m.timestamp, &m.value, &m.property, &m.plug_id, &m.household_id, &m.house_id) < 7)
            continue;

        // send the message
        if(m.property == '0')
            write(connfd, &m, sizeof(m));
    }

    close(listenfd);
    close(connfd);
}
