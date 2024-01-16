#ifndef CELL_H
#define CELL_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <cstdio>
#include <assert.h>
#include "coord.h"
#include "connection.h"
#include "outputLogService.h"
#include "toStr.h"

using namespace std;

class Cell
{
  public:
    //needed for MAP equations
    double voltage;           //current voltage in soma
    double last_voltage;      //in previous time step
    double last_last_voltage; //in previous previous time step, need for map equation
    double u;                 //second state variable
    double u_temp;            //second state variable
    int fired;                //if this cell fired

    //needed for balancing
    double input;         //current total of inputs, decays with time, internal for MAP
    double input_target;  //keeping "constant" total input strength ("balancing"), can be changed only by adaptation
    double output_target; //output balancing of strengths used for competition
    double output_sum;    //total strengths of all outputs at the current time step

    //needed for spike rate adaptation
    //adaptation should help with reaching happiness asymptote faster and keep us there
    int epoch_spikes;         //spikes during last epoch
    double spike_rate;        //spike rate during epoch
    double spike_rate_target; //reference value how the neuron should be approximatelly firing during epoch
    static const_expr double spike_rate_dt = 0.0001;
    static const_expr double input_target_dt = 0.00001;
    double resistance; //high values causing the cell less likely to fire
        //Glutamate inhibition creates more sensitivity to the input, scale of hours, thiagarajan 05

    static const_expr double mu = 0.0005;
    static const_expr double beta_e = 0.133;

    static const_expr double alOrig = 3.65;
    static const_expr double sigOrig = 0.06;

    double al;
    double sig;

    vector<Connection *> in_connections, out_connections;

    void print_connections(string f, OutputLogService &Logservice);

    Coord location;                        //{x,y,z}              //layer 0..2
    int flash;                             //the cell is flashed and waits for input to be added
    list<time_type> spike_log;             //memory for spikes (stdp)
    void clean_spike_log(time_type ctime); //remove spikes older than spike_mem

    double incoming_total;  //total synaptic weight of all inputs, for input balancing
    double fire_rate;       //0.1
    double outgoing_target; //cell's total strenth for output connections
    bool enableSynapticInput;

    static const_expr double input_noise = 0.12;
    void add_synaptic_input(double input_current);

    Cell();

    //MAP, returns 1 if cell fires
    //inline functions must go in .h file
    inline int calc(OutputLogService &LogService, int properDynamics, bool logSpiking)
    {

        //never used
        /*if(input_bound) {
        if(input > 0.05)  input = 0.05;
        if(input < -0.05) input = -0.05;
    }*/
        if(properDynamics == 1){ // proper neuron dynamics
          u_temp = u + (beta_e * input);
          u = u - mu * (voltage + 1) + (mu * sig) + mu * input;
        }
        else if(properDynamics == 0){ // flipped neuron dynamics
          u = u - mu * (voltage + 1) + (mu * sig) + mu * input;
          u_temp = u + (beta_e * input);
        }
        else // improper properDynamics value
          assert(0); 

        if (voltage <= 0)
        {
            last_last_voltage = last_voltage;
            last_voltage = voltage;
            voltage = al / (1 - voltage) + u_temp;
            fired = 0;
        }
        else if (voltage > 0 && voltage < al + u_temp && last_last_voltage <= 0)
        {
            //printf("starting last_voltage:%lf \n",last_voltage);
            last_last_voltage = last_voltage;
            last_voltage = voltage;
            voltage = al + u_temp;
            //printf("voltage:%lf last_voltage:%lf u_temp:%lf \n", voltage, last_voltage, u_temp);
            fired = 0;
        }
        else if ((voltage >= (al + u_temp)) || last_last_voltage > 0)
        {
            last_last_voltage = last_voltage;
            last_voltage = voltage;
            voltage = -1;
            fired = 1;
            //print spike times
            if(logSpiking){
              LogService.GetLog(LogService.SpikeTimesLogKey)
                << location.z << " "
                << location.y << " "
                << location.x << " "
                << LogService.Time.ctime << endl;
            }

            /*num_fired = num_fired + 1;*/ 
        }
        else
        {
            printf("voltage:%lf last_voltage:%lf last_last_voltage:%lf u_temp:%lf input:%lf u:%lf \n", voltage, last_voltage, last_last_voltage, u_temp, input, u);
            assert(0);
        }


        input = input * 0.985; //0.99;//decay

        //Writing to the log here really slows down the code
        //this shoulb be commented out unless this data is needed

        // LogService.GetLog(LogService.CellDataKey)
        //  << to_str(location.z) << " "
        //   << to_str(location.y) << " "
        //   << to_str(location.x) << " "
        //   << to_str(LogService.Time.ctime) << " "
        //   << to_str(input) << " "
        //   << to_str(voltage) << " "
        //   << to_str(fired ) << endl;

        return fired;
    } //calc

  private:
    Cell(const Cell &that); //forbid faulty Cell tmp=getcell(); instead of Cell &tmp=getcell();
    //Cell(const Cell& that) = delete; //we can move this to public part once -std=c++11 is gcc default

}; //Cell

#endif // CELL_H
