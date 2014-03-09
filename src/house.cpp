#include <iostream>
#include <vector>
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
#include <math.h>
#include "common_q1.h"
#include "mc.h"

//global
house_state state;
std::unordered_map<unsigned int, plug_state> household_aggregate;
plug_state house_aggregate;

std::unordered_map<unsigned int,
    std::unordered_map<unsigned int,
        std::unordered_map<unsigned int,
            std::unordered_map<unsigned int, Mc>
        >
    >
> median_container;

std::unordered_map<unsigned int,
    std::unordered_map<unsigned int, Mc>
>house_median_container;

//ML data containers
std::unordered_map<unsigned int,                    //lambda
    std::unordered_map<unsigned int,                //slice
        std::unordered_map<unsigned int,            //[0-2] historic index [no need for timeofday index]
            float
        >
    >
> house_weights, house_hist_medians, house_avg_values, house_forecasts;

std::unordered_map<unsigned int,                    //lambda
    std::unordered_map<unsigned int,                //slice
        std::unordered_map<unsigned int,            //[0-2] historic index [no need for timeofday index]
            unsigned int
        >
    >
> house_ts;


std::unordered_map<unsigned int,                    //lambda
    std::unordered_map<unsigned int,
        float
    >
> house_error;                                      //this is the error to see what value of lambda is the best

std::unordered_map<unsigned int,                    //lambda
    std::unordered_map<unsigned int,
        std::unordered_map<unsigned int,
            std::unordered_map<unsigned int,                //slice
                std::unordered_map<unsigned int,            //[0-2] historic index [no need for timeofday index]
                    float
                >
            >
        >
    >
> plug_weights, plug_hist_medians, plug_avg_values, plug_forecasts;

std::unordered_map<unsigned int,                    //lambda
    std::unordered_map<unsigned int,
        std::unordered_map<unsigned int,
            std::unordered_map<unsigned int,                //slice
                std::unordered_map<unsigned int,            //[0-2] historic index [no need for timeofday index]
                    unsigned int
                >
            >
        >
    >
> plug_ts;


std::unordered_map<unsigned int,                    //lambda
    std::unordered_map<unsigned int,                //slice
        std::unordered_map<unsigned int,
            std::unordered_map<unsigned int,
                float
            >
        >
    >
> plug_error;                                      //this is the error to see what value of lambda is the best


float lambda[] = {2};  //how many ever values you need.
size_t no_of_lambdas = sizeof(lambda)/sizeof(lambda[0]);

unsigned int house_id;

float forecastHouseLoad(unsigned int ts, unsigned int forcast_ts, float avg_value, unsigned int slice, unsigned int lambdaindex = 0) {
    if (ts%21600 == 0)
        cerr << "HOUSE_ERROR," << lambda[lambdaindex] << "," << ts << "," << slice << "," << house_error[lambdaindex][slice] << endl;

    float weight;
    if (house_weights[lambdaindex].find(slice) == house_weights[lambdaindex].end()) {
        house_weights[lambdaindex][slice][0] = 0.5;
        house_weights[lambdaindex][slice][1] = 0.5;
        house_weights[lambdaindex][slice][2] = 0.5;
    }
    weight = house_weights[lambdaindex][slice][0];
    float median;
    median = house_median_container[slice][forcast_ts % 86400].getMedian();
    float forecast = weight*median + (1 - weight)*avg_value;
    if (house_ts[lambdaindex].find(slice) == house_ts[lambdaindex].end())
    {
        house_ts[lambdaindex][slice][0] = 0;
        house_ts[lambdaindex][slice][1] = 0;
    }

    bool valid_history = ((ts - house_ts[lambdaindex][slice][1]) == 2*timeslice_lengths.at(slice));
    //cerr << "ts = " << ts << ", house_ts = " << house_ts[lambdaindex][slice][1] << ", timeslice_lengths = " << timeslice_lengths.at(slice) << endl;

    if (ts%timeslice_lengths.at(slice) != 0)
        return forecast;

    if (house_hist_medians[lambdaindex].find(slice) == house_hist_medians[lambdaindex].end()) {
        house_hist_medians[lambdaindex][slice][0] = 0;
        house_hist_medians[lambdaindex][slice][1] = 0;
    }

    if (house_forecasts[lambdaindex].find(slice) == house_forecasts[lambdaindex].end()) {
        house_forecasts[lambdaindex][slice][0] = 0;
        house_forecasts[lambdaindex][slice][1] = 0;
    }

    if (house_avg_values[lambdaindex].find(slice) == house_avg_values[lambdaindex].end()) {
        house_avg_values[lambdaindex][slice][0] = 0;
        house_avg_values[lambdaindex][slice][1] = 0;
    }

    if (house_error[lambdaindex].find(slice) == house_error[lambdaindex].end()) {
        house_error[lambdaindex][slice] = 0;
    }

    if (valid_history)
    {
        float error = (avg_value - house_forecasts[lambdaindex][slice][1]);
        house_error[lambdaindex][slice] += error * error;
    }

    float wnew = house_weights[lambdaindex][slice][0];

    if(valid_history)
    {
        wnew = (    (house_weights[lambdaindex][slice][2] * lambda[lambdaindex])
                    - ( (avg_value - house_avg_values[lambdaindex][slice][1])
                        *(  house_hist_medians[lambdaindex][slice][1]
                            - house_avg_values[lambdaindex][slice][1]
                        )
                    )
                )
        /
            (lambda[lambdaindex]
                    +
                    pow( (  house_hist_medians[lambdaindex][slice][1]
                        - house_avg_values[lambdaindex][slice][1]
                    ), 2.0));
        if(wnew < 0 || wnew > 1)
            wnew = house_weights[lambdaindex][slice][0];
    }

    house_forecasts[lambdaindex][slice][1] = house_forecasts[lambdaindex][slice][0];
    house_forecasts[lambdaindex][slice][0] = forecast;

    house_weights[lambdaindex][slice][2] = house_weights[lambdaindex][slice][1];
    house_weights[lambdaindex][slice][1] = house_weights[lambdaindex][slice][0];
    house_weights[lambdaindex][slice][0] = wnew;

    house_avg_values[lambdaindex][slice][1] = house_avg_values[lambdaindex][slice][0];
    house_avg_values[lambdaindex][slice][0] = avg_value;

    house_hist_medians[lambdaindex][slice][1] = house_hist_medians[lambdaindex][slice][0];
    house_hist_medians[lambdaindex][slice][0] = median;


    house_ts[lambdaindex][slice][1] = house_ts[lambdaindex][slice][0];
    house_ts[lambdaindex][slice][0] = ts;

    //cerr << "house_ts = " << house_ts[lambdaindex][slice][0] << endl;
    return forecast;
}

float forecastPlugLoad(unsigned int ts, unsigned int forcast_ts, unsigned int hh_id, unsigned int plug_id, float avg_value, unsigned int slice, unsigned int lambdaindex = 0)
{
    //lambda, timestamp, hh_id, plug_id, slice, error
    if (ts%21600 == 0)
        cerr << "PLUG_ERROR," << lambda[lambdaindex] << "," << ts << "," << hh_id << "," << plug_id << "," << slice << "," << plug_error[lambdaindex][hh_id][plug_id][slice] << endl;
    float weight;
    if (plug_weights[lambdaindex][hh_id][plug_id].find(slice) == plug_weights[lambdaindex][hh_id][plug_id].end()) {
        plug_weights[lambdaindex][hh_id][plug_id][slice][0] = 0.5;
        plug_weights[lambdaindex][hh_id][plug_id][slice][1] = 0.5;
        plug_weights[lambdaindex][hh_id][plug_id][slice][2] = 0.5;
    }
    weight = plug_weights[lambdaindex][hh_id][plug_id][slice][0];

    float median, forecast;
    median = median_container[hh_id][plug_id][slice][forcast_ts % 86400].getMedian();
    if (median < 0)
        forecast = avg_value;
    else
        forecast = weight*median + (1 - weight)*avg_value;

    if (plug_ts[lambdaindex][hh_id][plug_id].find(slice) == plug_ts[lambdaindex][hh_id][plug_id].end())
    {
        plug_ts[lambdaindex][hh_id][plug_id][slice][0] = 0;
        plug_ts[lambdaindex][hh_id][plug_id][slice][1] = 0;
    }

    bool valid_history = ((ts - plug_ts[lambdaindex][hh_id][plug_id][slice][1]) == 2*timeslice_lengths.at(slice));

    if (ts%timeslice_lengths.at(slice) != 0)
        return forecast;

    if (plug_hist_medians[lambdaindex][hh_id][plug_id].find(slice) == plug_hist_medians[lambdaindex][hh_id][plug_id].end()) {
        plug_hist_medians[lambdaindex][hh_id][plug_id][slice][0] = 0;
        plug_hist_medians[lambdaindex][hh_id][plug_id][slice][1] = 0;
    }

    if (plug_forecasts[lambdaindex][hh_id][plug_id].find(slice) == plug_forecasts[lambdaindex][hh_id][plug_id].end()) {
        plug_forecasts[lambdaindex][hh_id][plug_id][slice][0] = 0;
        plug_forecasts[lambdaindex][hh_id][plug_id][slice][1] = 0;
    }

    if (plug_avg_values[lambdaindex][hh_id][plug_id].find(slice) == plug_avg_values[lambdaindex][hh_id][plug_id].end()) {
        plug_avg_values[lambdaindex][hh_id][plug_id][slice][0] = 0;
        plug_avg_values[lambdaindex][hh_id][plug_id][slice][1] = 0;
    }

    if (plug_error[lambdaindex][hh_id][plug_id].find(slice) == plug_error[lambdaindex][hh_id][plug_id].end()) {
        plug_error[lambdaindex][hh_id][plug_id][slice] = 0;
    }

    if (valid_history)
    {
        float error = (avg_value - plug_forecasts[lambdaindex][hh_id][plug_id][slice][1]);
        plug_error[lambdaindex][hh_id][plug_id][slice] = error * error;
    }

    plug_forecasts[lambdaindex][hh_id][plug_id][slice][1] = plug_forecasts[lambdaindex][hh_id][plug_id][slice][0];
    plug_forecasts[lambdaindex][hh_id][plug_id][slice][0] = forecast;

    float wnew = plug_weights[lambdaindex][hh_id][plug_id][slice][0];
    if (valid_history)
    {
        wnew = (  (plug_weights[lambdaindex][hh_id][plug_id][slice][2] * lambda[lambdaindex])
                    - ( (avg_value - plug_avg_values[lambdaindex][hh_id][plug_id][slice][1])
                        *(  plug_hist_medians[lambdaindex][hh_id][plug_id][slice][1]
                            - plug_avg_values[lambdaindex][hh_id][plug_id][slice][1]
                        )
                    )
                )
        /
            (lambda[lambdaindex]
                    +
                    pow( (  plug_hist_medians[lambdaindex][hh_id][plug_id][slice][1]
                        - plug_avg_values[lambdaindex][hh_id][plug_id][slice][1]
                    ), 2.0));
        if(wnew < 0 || wnew > 1)
            wnew = plug_weights[lambdaindex][hh_id][plug_id][slice][0];
    }

    plug_weights[lambdaindex][hh_id][plug_id][slice][2] = plug_weights[lambdaindex][hh_id][plug_id][slice][1];
    plug_weights[lambdaindex][hh_id][plug_id][slice][1] = plug_weights[lambdaindex][hh_id][plug_id][slice][0];
    plug_weights[lambdaindex][hh_id][plug_id][slice][0] = wnew;

    plug_avg_values[lambdaindex][hh_id][plug_id][slice][1] = plug_avg_values[lambdaindex][hh_id][plug_id][slice][0];
    plug_avg_values[lambdaindex][hh_id][plug_id][slice][0] = avg_value;

    plug_hist_medians[lambdaindex][hh_id][plug_id][slice][1] = plug_hist_medians[lambdaindex][hh_id][plug_id][slice][0];
    plug_hist_medians[lambdaindex][hh_id][plug_id][slice][0] = median;


    plug_ts[lambdaindex][hh_id][plug_id][slice][1] = plug_ts[lambdaindex][hh_id][plug_id][slice][0];
    plug_ts[lambdaindex][hh_id][plug_id][slice][0] = ts;

    return forecast;
}

void forcastPlugLoad(unsigned int ts, unsigned int hh_id, unsigned int plug_id, float average_load, unsigned int slice) {
    unsigned int forcast_ts = ts - ts % timeslice_lengths.at(slice) +
            ((ts % timeslice_lengths.at(slice))?2:1) * timeslice_lengths.at(slice);
    float median = median_container[hh_id][plug_id][slice][forcast_ts % 86400].getMedian(), forcast;
    if (median < 0){
        forcast = average_load;
    }
    else{
        forcast = (average_load + median)/2;
        for(unsigned int i = 0; i < no_of_lambdas; i++)
            forcast = forecastPlugLoad(ts, forcast_ts, hh_id, plug_id, average_load, slice, i);
    }
    printf("PLUG_FORECAST_%u_S %u %u,%u,%u,%u,%f\n", timeslice_lengths.at(slice), ts, forcast_ts, house_id, hh_id, plug_id, forcast);
}

void forcastHouseLoad(unsigned int ts, float average_load, unsigned int slice) {
    unsigned int forcast_ts = ts - ts % timeslice_lengths.at(slice) +
            ((ts % timeslice_lengths.at(slice))?2:1) * timeslice_lengths.at(slice);
    float median = house_median_container[slice][forcast_ts % 86400].getMedian(), forcast;
    if (median < 0)
    {
        forcast = average_load;
    }
    else
    {
        forcast = (average_load + median)/2;
        for(unsigned int i = 0; i < no_of_lambdas; i++)
            forcast = forecastHouseLoad(ts, forcast_ts, average_load, slice, i);
    }

    printf("HOUSE_FORECAST_%u_S %u %u,%u,%f\n", timeslice_lengths.at(slice), ts, forcast_ts, house_id, forcast);
}

void processHouse(unsigned int boundary_ts, unsigned int x, float load, bool flush = false) {

    static unsigned int last_timestamp = boundary_ts;
    static bool just_flushed = false;
    if (flush) {
        if (just_flushed) return;
        for (auto& slice:timeslice_lengths) {
            if((slice.first) == (TIMESLICE_30S)) continue;
            forcastHouseLoad(last_timestamp, house_aggregate[slice.first].accumulated_load, slice.first);
            if (last_timestamp % slice.second == 0)
                house_median_container[slice.first][(last_timestamp-slice.second)%86400].insert(house_aggregate[slice.first].accumulated_load);
            house_aggregate[slice.first].accumulated_load = 0;
        }
        just_flushed = true;
    } else {
        last_timestamp = boundary_ts;
        house_aggregate[x].accumulated_load += load;
        just_flushed = false;
    }
}

void processHouseHold(unsigned int boundary_ts, unsigned int x, float load, unsigned int household_id, bool flush = false) {

    static unsigned int last_timestamp = boundary_ts;
    static bool just_flushed = false;
    if (flush || last_timestamp < boundary_ts) {
        if (flush && just_flushed) return;
        for (auto& household:household_aggregate) {
            for (auto& slice:timeslice_lengths) {
                if (slice.first == TIMESLICE_30S) continue;
                if (flush)
                    processHouse(last_timestamp, slice.first, household.second[slice.first].accumulated_load);
                household.second[slice.first].accumulated_load = 0;
            }
        }
        if (flush) {
            processHouse(0,0,0,true);
            just_flushed = true;
        }
    }
    if (!flush) {
        last_timestamp = boundary_ts;

        if (household_aggregate.find(household_id) == household_aggregate.end())
            household_aggregate[household_id] = initial_plug_state;
        household_aggregate[household_id][x].accumulated_load += load;
        just_flushed = false;
    }
    //for testing
/*  cout<<boundary_ts<< " slice " << x<<
            " load " << load<<
            " hhid " << household_id <<
            " agg " <<household_aggregate[household_id][x].accumulated_load <<endl;
*/
}

//for each measurement read do
//float doProcessing(measurement *input) {  //for testing
void doProcessing(measurement *input) {

    if (input->property == '0') //ignore if the measurement is a work value
        return ;
//      return 0; //for testing

    timeval ctime;
    gettimeofday(&ctime, NULL);
    static timeval ptime = ctime;

    unsigned int modulo = input->timestamp % timeslice_lengths.at(TIMESLICE_30S);
    static unsigned int last_timestamp = input->timestamp -((modulo == 0)? timeslice_lengths.at(TIMESLICE_30S) : modulo);

    if ( state.find(input->household_id) == state.end() )
        state[input->household_id] = initial_household_state;
    if ( state[input->household_id].find(input->plug_id) ==
            state[input->household_id].end() ) {
        state[input->household_id][input->plug_id] = initial_plug_state;
        for (auto& x:timeslice_lengths) {
            state[input->household_id][input->plug_id][x.first].last_timestamp =
                    input->timestamp -
                    ((input->timestamp % x.second == 0)? x.second : input->timestamp % x.second);
        }
    }

    unsigned int gap = input->timestamp - last_timestamp;
    if (gap > timeslice_lengths.at(TIMESLICE_30S)) {

        gap = last_timestamp;
        last_timestamp = input->timestamp -
                ((modulo == 0)? timeslice_lengths.at(TIMESLICE_30S) : modulo);
        gap = last_timestamp - gap;

        for (auto& household:state)
        for (auto& plug:household.second) {
            if (plug.second[TIMESLICE_30S].last_timestamp == last_timestamp)
                continue;
            for (auto& x:plug.second) {
                if (x.first == TIMESLICE_30S)
                    continue;
                if (last_timestamp - x.second.last_timestamp > timeslice_lengths.at(x.first)) {
                    x.second.accumulated_load = 0;
                    x.second.count_of_values = 0;
                    x.second.last_timestamp = last_timestamp - last_timestamp % timeslice_lengths.at(x.first);
                    continue;
                }
                x.second.accumulated_load += plug.second[TIMESLICE_30S].accumulated_load;
                x.second.count_of_values += plug.second[TIMESLICE_30S].count_of_values;
                float average_load = 0;
                if (x.second.count_of_values) {
                    average_load = x.second.accumulated_load /  x.second.count_of_values;
                    forcastPlugLoad(last_timestamp, household.first, plug.first, average_load, x.first);
                }

                if (last_timestamp - x.second.last_timestamp == timeslice_lengths.at(x.first)) {
                    median_container[household.first][plug.first][x.first][x.second.last_timestamp % 86400].insert(average_load);
                    x.second.accumulated_load = 0;
                    x.second.count_of_values = 0;
                    x.second.last_timestamp = last_timestamp;
                }

                processHouseHold(last_timestamp, x.first, average_load, household.first);
            }
            plug.second[TIMESLICE_30S].accumulated_load = 0;
            plug.second[TIMESLICE_30S].count_of_values = 0;
            plug.second[TIMESLICE_30S].last_timestamp = last_timestamp;

        }
        processHouseHold(0,0,0,0,true);
        gettimeofday(&ctime, NULL);
        //std::cerr << "Latency = " << (ctime.tv_sec - ptime.tv_sec)*1000000 + ctime.tv_usec - ptime.tv_usec<< std::endl;
    }

    plug_state &p_state = state[input->household_id][input->plug_id];

    if (input->timestamp - p_state[TIMESLICE_30S].last_timestamp <= timeslice_lengths.at(TIMESLICE_30S)) {
        p_state[TIMESLICE_30S].accumulated_load += input->value;
        p_state[TIMESLICE_30S].count_of_values++;
    }

    if (input->timestamp - p_state[TIMESLICE_30S].last_timestamp == timeslice_lengths.at(TIMESLICE_30S)) {
        for (auto& x:timeslice_lengths) {
            if (x.first == TIMESLICE_30S)
                continue;
            p_state[x.first].accumulated_load += p_state[TIMESLICE_30S].accumulated_load;
            p_state[x.first].count_of_values += p_state[TIMESLICE_30S].count_of_values;
            float average_load = 0;
            if (p_state[x.first].count_of_values) {
                average_load = p_state[x.first].accumulated_load /  p_state[x.first].count_of_values;
                forcastPlugLoad(input->timestamp, input->household_id, input->plug_id, average_load, x.first);
            }

            if (input->timestamp - p_state[x.first].last_timestamp == timeslice_lengths.at(x.first)) {
                median_container[input->household_id][input->plug_id][x.first][p_state[x.first].last_timestamp % 86400].insert(average_load);
                p_state[x.first].accumulated_load = 0;
                p_state[x.first].count_of_values = 0;
                p_state[x.first].last_timestamp = input->timestamp;
            }
            processHouseHold(input->timestamp, x.first, average_load, input->household_id);
        }
        p_state[TIMESLICE_30S].accumulated_load = 0;
        p_state[TIMESLICE_30S].count_of_values = 0;
        p_state[TIMESLICE_30S].last_timestamp = input->timestamp;
    }

    if (input->timestamp - p_state[TIMESLICE_30S].last_timestamp > timeslice_lengths.at(TIMESLICE_30S)) {
        p_state[TIMESLICE_30S].accumulated_load = input->value;
        p_state[TIMESLICE_30S].count_of_values = 1;
        p_state[TIMESLICE_30S].last_timestamp = input->timestamp -
                ((modulo == 0)? timeslice_lengths.at(TIMESLICE_30S) : modulo);
    }
    ptime = ctime;
}

// arg: house_id broker_ip port
int main(int argc, char *argv[])
{
    string house_id_str;
    int port;

    if(argc < 4)
    {
        cout <<"not enough arguments!"<<endl;
        exit(-1);
    } else
    {
        house_id_str  = string(argv[1]);
        house_id = atoi(house_id_str.c_str());
        port = atoi(argv[3]);
    }
    cout<<"house running with id :"<<house_id_str<<endl;

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

    if(inet_pton(AF_INET, argv[2], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }

    // informing about the house id to the broker process
    if (write(sockfd, &house_id, sizeof(house_id)) < 0) {
        printf("\n Error : Socket Write failed \n");
        return 1;
    }

    measurement *m = new measurement;
    while((n = read(sockfd, m, sizeof(measurement))) > 0)
    {
        if(n == sizeof(measurement))
        {
            doProcessing(m);
        } else if(n < sizeof(measurement))
        {
            char b[sizeof(measurement)];
            memcpy(b, &m, n);

            unsigned nrest = read(sockfd, m, sizeof(measurement) - n);
            if(nrest == sizeof(measurement) - n)
            {
                memcpy(b+n, &m, nrest);
                doProcessing(m);
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
