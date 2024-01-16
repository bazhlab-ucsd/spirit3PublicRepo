#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <assert.h>
#include "cell.h"

using namespace std;
void Cell::print_connections(string f, OutputLogService &Logservice)
{
    vector<Connection *>::iterator it = out_connections.begin();
    vector<Connection *>::iterator end = out_connections.end();
    for (; it != end; it++)
    {
        Logservice.GetLog(f)
            << (*it)->pre_cell->location.z << " "
            << (*it)->pre_cell->location.y << " "
            << (*it)->pre_cell->location.x << " "
            << (*it)->post_cell->location.z << " "
            << (*it)->post_cell->location.y << " "
            << (*it)->post_cell->location.x << " "
            << (*it)->strength << endl;
    } //cit
}

void Cell::clean_spike_log(time_type ctime)
{ //remove spikes older than spike_mem
    list<time_type>::iterator it = spike_log.begin();
    while (!spike_log.empty() && ctime - (*it) > Connection::spike_mem)
        it = spike_log.erase(it);
}

void Cell::add_synaptic_input(double input_current)
{
    if(enableSynapticInput)
    {
        double rand_num = (double)rand() / (double)RAND_MAX;
        input += input_current / resistance * (1.0 - input_noise + rand_num * 2 * input_noise); //1+-input_noise
    }
}

Cell::Cell()
{
    fire_rate = 0.1;

    voltage = -0.939998046792114; //probably passed
    last_voltage = -0.939998046792114;
    last_last_voltage = -0.939998046792114;
    fired = 0;              //if this cell fired
    input = 0.0;            //value of incoming signal
    input_target = 0;       //actually this needs to be setup dynamically in the beginning
    output_target = 0;      //actually this needs to be setup dynamically in the beginning
    output_sum = 0;         //actually this needs to be setup dynamically in the beginning
    u = -2.821443297088526; //second state variable
    u_temp = -2.8214;       //second state variable

    //    spike_rate_target = excitability; //0.15; //10% food rate; we should drive if little bit over
    epoch_spikes = 0;
    spike_rate = 0; //spike rate during epoch
    resistance = 1;
    enableSynapticInput = true; 
    al = alOrig;
    sig = sigOrig;
}
