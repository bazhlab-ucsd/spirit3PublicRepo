#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "network.h"

using namespace std;

void Network::initSpikeCountsvector(SpikeCountsClass & curSpikeCountsObj){
  for (int z = 0; z < layers; z++){
    curSpikeCountsObj.spikeCounts.push_back( new long long int* [layer_size[z]] );
    for (int i = 0; i < layer_size[z]; i++)
    {
      curSpikeCountsObj.spikeCounts[z][i] = new long long int [layer_size[z]];
    }
  }
  clearSpikecounts(curSpikeCountsObj);
}
    
void Network::copySpikeCountsToStageSpikeCounts(SpikeCountsClass & destination){
  for (int z = 0; z < layers; z++){
    for (int i = 0; i < layer_size[z]; i++){
      for (int j = 0; j < layer_size[z]; j++){
        destination.spikeCounts[z][i][j] = spikeCountsObj.spikeCounts[z][i][j];
      }
    }
  }
  destination.spikeCountsPeriod = spikeCountsObj.spikeCountsPeriod;
}

void Network::InitNetwork()
{
  aeon_spikes = 0;
  aeon_spikes_output = 0;
  numberOfNeuronsPerOutputDirection = 0;
  numberIterationsOfAdditiveHsp = 2;
  // set up spike average arrays
  initSpikeCountsvector(spikeCountsObj);
  initSpikeCountsvector(spikeCountsTrain1);
  initSpikeCountsvector(spikeCountsTrain2);
  initSpikeCountsvector(spikeCountsTrain1Perf);
  initSpikeCountsvector(spikeCountsTrain2Perf);
  initSpikeCountsvector(spikeCountsTest1);
  initSpikeCountsvector(spikeCountsTest2);
  //setup correct coordinates for each cell
  for (int z = 0, i = 0; z < layers; z++)
    for (int x = 0; x < layer_size[z]; x++)
      for (int y = 0; y < layer_size[z]; y++)
      {
        get_cell(x, y, z).location = Coord(x, y, z);
        cell[i++] = &get_cell(x, y, z);
      }
}

Network::Network(OutputLogService &logService) : LogService(logService)
{
  ApplyPlasticity = true;
  EnableLTD = true;
  EnableLTP = true;
  FreezeHiddenLayerWeights = false;
  EnableSpikeRateAdaptation = true;
  EnableInputBalencing = true;
  logSpiking = false; 
  logStdp = false;
  logAppliedStdp = false;
  EnableSpikeCounts = false;
  EnableInhibitionPlasticity = true;
  InputBalencingType = multiplicitive;
  EnableHighFrequencyWeightPrinting = false;
  InitNetwork();
}


void Network::clearSpikecounts(SpikeCountsClass & curSpikeCounts){
  curSpikeCounts.spikeCountsPeriod = 0;
  for (int z = 0; z < layers; z++)
    for (int x = 0; x < layer_size[z]; x++)
      for (int y = 0; y < layer_size[z]; y++)
        curSpikeCounts.spikeCounts[z][x][y] = 0;
}
void Network::printSpikeCounts(string title, SpikeCountsClass & curSpikeCounts){
  for (int z = 0; z < layers; z++)
    for (int x = 0; x < layer_size[z]; x++)
      for (int y = 0; y < layer_size[z]; y++)
      	LogService.GetLog(title) << z << " " << y << " " << x << " " << curSpikeCounts.spikeCounts[z][x][y] << endl;
 	 LogService.GetLog(title + "Period") << curSpikeCounts.spikeCountsPeriod << endl;
}

void Network::dump() { dump(0); }
void Network::dump(int level)
{
  for (int z = 0; z < layers; z++)
    for (int x = 0; x < layer_size[z]; x++)
      for (int y = 0; y < layer_size[z]; y++)
      {
        Cell &tmp = get_cell(x, y, z);

        vector<Connection *>::iterator it = tmp.in_connections.begin();
        vector<Connection *>::iterator end = tmp.in_connections.end();
        for (; it != end; it++)
          (*it)->dump(level);
      }
}

//gets the number output cells for the first direction based on how many
//have incomming connections
int Network::getNumberOutputCellsPerDirection(){
  int cellsPerDirection = output_size / 3;
  int connectedCellsPerDirection = 0;

  for( int x = 0; x < cellsPerDirection; x++){
    for( int y = 0; y < cellsPerDirection; y++){
      Cell & cur = get_cell(x, y, 2);

      if(cur.in_connections.size() > 0)
        connectedCellsPerDirection++;
    }
  }

  return connectedCellsPerDirection;
}


//functions for access to cell via its coordinates

void Network::write_network(const string f)
{
  Coord in, out;
  string junk;

  int x, y, z;

  cells_fired_cache.clear();
  for (z = 0; z < layers; z++)
    for (x = 0; x < layer_size[z]; x++)
      for (y = 0; y < layer_size[z]; y++)
      {
        Cell &c = get_cell(x, y, z);
        c.print_connections(f, LogService);
      }
  LogService.CloseLog(f);
} //write_network

//output matrix of output|_sum for debugging purposes
void Network::write_output_matrix()
{
  Coord in, out;
  string junk;
  int i, x, y, z = 1;
  double Sum = 0;

  for (i = x = 0; x < layer_size[1]; x++)
  {
    for (y = 0; y < layer_size[1]; y++)
    {
      Cell &cell = get_cell(x, y, z);
      //double sum=0; int i=0;
      //vector<Connection*>::iterator it=cell.out_connections.begin();
      //vector<Connection*>::iterator end=cell.out_connections.end();
      //for (;it!=end;it++,i++){
      //  if ((*it)->strength>0) i++,sum+=(*it)->output_sum;
      //}//cit
      LogService.GetLog(LogService.OutputSumLogKey) << cell.output_sum << " ";
      Sum += cell.output_sum;
      i++;
    }
    LogService.GetLog(LogService.OutputSumLogKey) << "\n";
  }
  LogService.GetLog(LogService.OutputSumLogKey) << "Aeon: " << LogService.Time.aeons() << " Sum: " << Sum / i << "\n";
} //write_output_matrix

//print coefficient of variation of strengths of connections from "layer"
double Network::get_connections_variability(int layer)
{
  double mean = 0;

  int x, y, i = 0;
  for (x = 0; x < layer_size[layer]; x++)
    for (y = 0; y < layer_size[layer]; y++)
    {
      Cell &c = get_cell(x, y, layer);
      vector<Connection *>::iterator it = c.out_connections.begin();
      vector<Connection *>::iterator end = c.out_connections.end();
      for (; it != end; it++)
        if ((*it)->strength > 0)
        {
          i++;
          mean += (*it)->strength;
        }
    }
  mean = mean / i;

  double sd = 0;
  for (x = 0; x < layer_size[layer]; x++)
    for (y = 0; y < layer_size[layer]; y++)
    {
      Cell &c = get_cell(x, y, layer);
      vector<Connection *>::iterator it = c.out_connections.begin();
      vector<Connection *>::iterator end = c.out_connections.end();
      for (; it != end; it++)
        if ((*it)->strength > 0)
          sd += sqr((*it)->strength - mean);
    }
  return sqrt(sd / i / mean);
} //connections variability

void Network::read_network(const string f, Coord * removeHiddenCoord, int numberHiddenNeuronsToRemove)
{
  int rx, ry, rz, ox, oy, oz; //pre & post coordinates
  Coord in, out;
  string junk;
  double stren;
  ifstream network_f(f.c_str());
  assert(network_f.good());
	
	//clear out all connections that were previously there
  list<Connection *> allCons;
	for(int i = 0; i < cells; i++){
		Cell & cur = *cell[i];

    for(vector<Connection *>::iterator it = cur.in_connections.begin();it != cur.in_connections.end(); it++){
      allCons.push_back(*it);
    }

    for(vector<Connection *>::iterator it = cur.out_connections.begin();it != cur.out_connections.end(); it++){
      allCons.push_back(*it);
    }

		cur.in_connections.clear();
		cur.out_connections.clear();
	}

  allCons.sort();
  allCons.unique();
  list<Connection *>::iterator itt = allCons.begin(); 
  list<Connection *>::iterator endd = allCons.end();
  for(;itt != endd; itt++){
    delete *itt;
  }

  while( network_f >> rz >> ry >> rx >> oz >> oy >> ox >> stren)
  {
    bool skip = false;

    //inefecient way to do this but needed to get this done.
    //tried to change parameters to reduce number of hidden layer neurons but ran into issues
    //this was the fastest way
    for(int i = 0; i < numberHiddenNeuronsToRemove && removeHiddenCoord != NULL; i++){
      // if the source or destination is a neuron that you should remove then dont add it
      if((removeHiddenCoord[i].x == rx  && removeHiddenCoord[i].y == ry  && removeHiddenCoord[i].z == rz) || (removeHiddenCoord[i].x == ox  && removeHiddenCoord[i].y == oy  && removeHiddenCoord[i].z == oz))
        skip = true;
    }

    if(skip)
      continue;

    Connection *tmp = new Connection(&get_cell(rx, ry, rz), &get_cell(ox, oy, oz), stren);
    //cout<<tmp->strength<<"\n";
    get_cell(rx, ry, rz).out_connections.push_back(tmp);
    get_cell(ox, oy, oz).in_connections.push_back(tmp);
  }
  numberOfNeuronsPerOutputDirection = getNumberOutputCellsPerDirection();
} //read_network

//calculate voltages of all cells for the next step
void Network::calc_next_state(int properDynamics)
{
  cells_fired_cache.clear();
  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];
    if (tmp.calc(LogService, properDynamics, logSpiking))
    { //calc and true if fired
      if(EnableSpikeCounts){
        spikeCountsObj.spikeCounts[tmp.location.z][tmp.location.x][tmp.location.y]++;
      }
      tmp.epoch_spikes++;
      if (tmp.location.z == 1)
        aeon_spikes++;
      if (tmp.location.z == 2)
        aeon_spikes_output++;
      //this line needs omp critical section if parallelized
      cells_fired_cache.push_back(tmp.location);
    } //if
  }   //for
  if(EnableSpikeCounts && Time.epoch_check())
    spikeCountsObj.spikeCountsPeriod++;
} //calc_next_state

void Network::compute_STDP(Cell &cell)
{
  time_type t = Time.ctime; //current spike time

  //pre-post firing pair
  vector<Connection *>::iterator it = cell.in_connections.begin();
  vector<Connection *>::iterator end = cell.in_connections.end();
  for (; it != end; it++)
  {
    if ((*it)->strength < 0 || (*it)->freeze == true)
      continue; //we do not add traces when strngth is negative; biologically controversial what to do here
    list<time_type>::iterator sit = (*it)->pre_cell->spike_log.begin();
    list<time_type>::iterator send = (*it)->pre_cell->spike_log.end();
    for (; sit != send; sit++){
      unsigned int prevSize = (*it)->trace.size();
      double stdpTraceValue = (*it)->add_pre_post_stdp_trace(t, *sit, EnableLTP);
      if(logStdp && prevSize < (*it)->trace.size()){ // if logStdp and the trace value was actually added to the list of traces
        LogService.GetLog("StdpEvents") << t  << " " 
          << (*it)->pre_cell->location.x << " " 
          << (*it)->pre_cell->location.y << " " 
          << (*it)->pre_cell->location.z << " " 
          << (*it)->post_cell->location.x << " " 
          << (*it)->post_cell->location.y << " " 
          << (*it)->post_cell->location.z << " " 
          << stdpTraceValue << endl;
      }
    }
  } //cit

  it = cell.out_connections.begin();
  end = cell.out_connections.end();
  for (; it != end; it++)
  {
    (*it)->clean_trace(Time.ctime);
    if ((*it)->strength < 0 || (*it)->freeze == true) 
      continue;
    list<time_type>::iterator sit = (*it)->post_cell->spike_log.begin();
    list<time_type>::iterator send = (*it)->post_cell->spike_log.end();
    for (; sit != send; sit++){
      unsigned int prevSize = (*it)->trace.size();
      double stdpTraceValue = (*it)->add_post_pre_stdp_trace(t, *sit, EnableLTD);
      if(logStdp && prevSize < (*it)->trace.size()){ // if logStdp and the trace value was actually added to the list of traces
        LogService.GetLog("StdpEvents") << t  << " " 
          << (*it)->pre_cell->location.x << " " 
          << (*it)->pre_cell->location.y << " " 
          << (*it)->pre_cell->location.z << " " 
          << (*it)->post_cell->location.x << " " 
          << (*it)->post_cell->location.y << " " 
          << (*it)->post_cell->location.z << " " 
          << stdpTraceValue << endl;
      }
    }
  } //cit

  cell.clean_spike_log(Time.ctime);
  cell.spike_log.push_back(Time.ctime);
} //compute_STDP

//compute direction of next move of critter, winner is cell which has most activity && fired first in case of tie
//when no firing, repeat movement from last step
void Network::compute_critter_direction(Critter &critter, Cell &cell)
{
  if (cell.location.z == layers - 1)
  {
    //need to split output layer into thirds so dirction can be calculated
    int splitSize = output_size / 3;
    //output layer is 9 x 9 so split into thirds to get coresponding direction.
    int x = cell.location.x / splitSize;
    int y = cell.location.y / splitSize;
    critter.dirtotal[x][y]++; //after the loop to get cell which fired first

    //cout<<"in 3";
    int dmax = 0; //what is the maximum number of firing of output cell since last move
    vector<Coord> winners;

    //this seems to be unnecessary to do for each cell,
    //it could matter previously because we cared about the order of firing
    //now it could be moved out of the function
    for (int i = 0; i < output_size; i++)
      for (int j = 0; j < output_size; j++){
        int mapi = i / splitSize;
        int mapj = j / splitSize;
        if (mapi != 1 || mapj != 1)
        { //ignore middle output cell
          if (critter.dirtotal[mapi][mapj] == dmax)
            winners.push_back(Coord(mapi, mapj));
          if (critter.dirtotal[mapi][mapj] > dmax)
          {
            dmax = critter.dirtotal[mapi][mapj];
            winners.clear();
            winners.push_back(Coord(mapi, mapj));
          }
        }
      }

    const Coord tmp = winners[rand() % winners.size()];

    if (x != 1 || y != 1)
    {                                  //direction can't be 0,0, very low probability we will move on later
      critter.direction.x = (tmp.x - 1) % 2; //-1 -> shifting [1,1] of output cells to [0,0] coordinates
      critter.direction.y = (tmp.y - 1) % 2;
      //cout << "criter direction " << critter.direction.x << " " << critter.direction.y << endl;
    }
  } //if output_layer
} //compute_critter_direction

//apply firing results to connections and critter
void Network::apply_firings(Critter &critter)
{
  list<Coord>::iterator it = cells_fired_cache.begin();
  list<Coord>::iterator end = cells_fired_cache.end();

  for (; it != end; it++)
  {
    Cell &tmp = get_cell(*it);

    //summing of all input for cells in next layer
    for (unsigned int c = 0; c < tmp.out_connections.size(); c++)
      tmp.out_connections[c]->post_cell->add_synaptic_input(tmp.out_connections[c]->strength);

    compute_STDP(tmp);
    compute_critter_direction(critter, tmp);

  } //for cell
} //apply_firings

//happens only we are in reward part of critter movement; we want to make active connection stronger and inactive weaker
void Network::apply_STDP_traces(double dopamine)
{
  if(!ApplyPlasticity)
    return;

  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];

    if(ShouldSkpHiddenCellPlasticity(tmp))
      continue;

    vector<Connection *>::iterator cit = tmp.in_connections.begin();
    vector<Connection *>::iterator cend = tmp.in_connections.end();
    for (; cit != cend; cit++)
    {
      (*cit)->clean_trace(Time.ctime); 

      double &strength = ((*cit)->strength);

      if(isnan(strength))
        {
          cout << "NaN: " << tmp.location.z << " " << tmp.location.y << " " << tmp.location.x << "\n";
          exit(1);
        }
      if (strength < 0 || (*cit)->freeze == true)
        continue; //we don't update negative ones, see similar comments in compute_STDP

      //each trace affects strength of connection (involve pre_cell output balancing)
      list<Trace>::iterator tit = (*cit)->trace.begin();
      list<Trace>::iterator tend = (*cit)->trace.end();
      double sumtr = 0;
      for (; tit != tend; tit++)
      {
        sumtr += tit->mag * 1.0 / (Time.ctime - tit->time + Time.epoch);
      }
      (*cit)->trace_average = (*cit)->trace_average * (1 - Connection::trace_average_d) + Connection::trace_average_d * sumtr;
      if ((*cit)->trace_average >= 0 && (*cit)->trace_average < 1e-10)
        (*cit)->trace_average = 1e-10;
      if ((*cit)->trace_average < 0 && (*cit)->trace_average > -1e-10)
        (*cit)->trace_average = -1e-10;

      tit = (*cit)->trace.begin();
      for (; tit != tend; tit++)
      {
        double stdp_dt = tit->mag * 1.0 / (Time.ctime - tit->time + Time.epoch); //we add epoch in order to decrease sensitivity to the very recent spikes

        //for many strong outputs is should have hard time to build more; we don't want a single cell dominate the whole network
        //slight optimization - we take output_sum from last iteration step instead of the current one computed later, it shouldn't change much
        double delta = stdp_dt * dopamine * sumtr / (*cit)->trace_average;
        if (tmp.location.z == 1)
          delta = stdp_dt; //we want reward only for output layer cells

        delta = (delta > 0.01) ? 0.01 : delta;
        delta = (delta < -0.01) ? -0.01 : delta;

        double balancing = (*cit)->pre_cell->output_target / (*cit)->pre_cell->output_sum;
        if(logAppliedStdp)
          LogService.GetLog("appliedStdpEvents") << Time.ctime  << " " 
            << (*cit)->pre_cell->location.x << " " 
            << (*cit)->pre_cell->location.y << " " 
            << (*cit)->pre_cell->location.z << " " 
            << (*cit)->post_cell->location.x << " " 
            << (*cit)->post_cell->location.y << " " 
            << (*cit)->post_cell->location.z << " " 
            << balancing << " " 
            << delta << endl;
        strength *= 1 + balancing * delta;
      }

      if (tmp.location.z == 1)
      {
        if (strength > strength_middle_bound)
          strength = strength_middle_bound * tmp.resistance;
      }
      else if (strength > strength_bound)
        strength = strength_bound;

      if (strength <= 0.00001)
        strength = 0.00001; //set to minimum positive value in case the strength is zero; this could be tuned
    }                       //cit

  } //for cell
  if(enableInputBalencingOverride == 1 && EnableInputBalencing)
    input_balancing();
  output_balancing(); 
} //apply_STDP_traces

void Network::init_balancing()
{
  init_excitability();

  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];
    double sum;
    vector<Connection *>::iterator cit = tmp.in_connections.begin();
    vector<Connection *>::iterator cend = tmp.in_connections.end();
    for (sum = 0; cit != cend; cit++)
      if ((*cit)->strength > 0) //inhibition should equal excitation
        sum += (*cit)->strength;
    tmp.input_target = sum;

    // not every output layer cell is used any more
    // if (tmp.location.z > 0)
    //   assert(sum != 0.0);

    cit = tmp.in_connections.begin();
    for (sum = 0; cit != cend; cit++)
      if ((*cit)->strength < 0 && (*cit)->pre_cell->location.z < 2) //inhibition should equal excitation
        (*cit)->relative_inhib = (*cit)->strength / tmp.input_target;

    cit = tmp.out_connections.begin();
    cend = tmp.out_connections.end();
    for (sum = 0; cit != cend; cit++)
      if ((*cit)->strength > 0)
        sum += (*cit)->strength;
    tmp.output_target = sum; //tmp.output_target=tmp.output_sum; 

    // redid network, it is possible for hidden layer cells to not be connected to out put
    // with 784 hidden and 81 output, every output cell recieves 87 input connections
    // only one hidden cell has no connections to output layer
    // if (tmp.location.z < layers - 1)
    //   assert(sum != 0.0);

  }                   //cells
  output_balancing(); //needs to be done before entering apply_STDP_traces -> div by zero otherwise
} //init_balancing

//hetero synaptic plasticity
void Network::input_balancing()
{
  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];
    
    if(ShouldSkpHiddenCellPlasticity(tmp))
      continue;
      
    vector<Connection *>::iterator cit = tmp.in_connections.begin();
    vector<Connection *>::iterator cend = tmp.in_connections.end();

    if(InputBalencingType == multiplicitive){
      double input_sum = 0;
      double freeze_sum = 0;
      // get total input sum
      for (; cit != cend; cit++){
        if ((*cit)->strength > 0 && (*cit)->freeze == false){ //inhibition should equal excitation
          input_sum += (*cit)->strength;
        }
        else if ((*cit)->strength > 0 && (*cit)->freeze == true)
          freeze_sum += (*cit)->strength;
      }

      for (cit = tmp.in_connections.begin(); cit != cend; cit++)
      {
        if ((*cit)->strength > 0 && (*cit)->freeze == false)
        {
          (*cit)->strength *= (tmp.input_target - freeze_sum) / input_sum;
          if ((*cit)->strength < 0.00001)
            (*cit)->strength = 0.00001; //unlikely, but to be sure...
        }
        // output balencing overwrites this change later in the code
        if ((*cit)->strength < 0 && (*cit)->pre_cell->location.z < 2)
          (*cit)->strength = (*cit)->relative_inhib * tmp.input_target;
      }
    }

    else if(InputBalencingType == additive){
      for ( int k = 0; k < numberIterationsOfAdditiveHsp; k++){ //do additive hsp multiple times depending on how accurate we want to be
        double input_sum = 0;
        double freeze_sum = 0;
        double numberSyns = 0;
        // get total input sum
        for (; cit != cend; cit++){
          if ((*cit)->strength > 0 && (*cit)->freeze == false){ //inhibition should equal excitation
            input_sum += (*cit)->strength;
            if ((*cit)->strength > 0.00001) // only count synapses that are above their min value
              numberSyns = numberSyns + 1.0;
          }
          else if ((*cit)->strength > 0 && (*cit)->freeze == true)
            freeze_sum += (*cit)->strength;
        }

        for (cit = tmp.in_connections.begin(); cit != cend; cit++)
        {
          if ((*cit)->strength > 0 && (*cit)->freeze == false)
          {
            (*cit)->strength += (((tmp.input_target - freeze_sum) -  input_sum) / numberSyns);
            if ((*cit)->strength < 0.00001)
              (*cit)->strength = 0.00001; //unlikely, but to be sure...
          }
          // output balencing overwrites this change later in the code
          if ((*cit)->strength < 0 && (*cit)->pre_cell->location.z < 2)
            (*cit)->strength = (*cit)->relative_inhib * tmp.input_target;
        }
      }   
    }
  } //cells
} //input balancing
//running out of inner cell resources, currently disabled
void Network::output_balancing()
{
  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];

    double sum = 0;
    vector<Connection *>::iterator cit = tmp.out_connections.begin();
    vector<Connection *>::iterator cend = tmp.out_connections.end();
    int n = 0;
    for (; cit != cend; cit++)
      if ((*cit)->strength > 0 && (*cit)->freeze == false)
      {
        sum += (*cit)->strength;
        n++;
      }

    tmp.output_sum = sum;

    double x = 0.01; //rate of firm balancing change
    //To enable output balancing uncomment line below
    //cit=tmp.out_connections.begin();
    for (; cit != cend; cit++)
      if ((*cit)->strength > 0 && (*cit)->freeze == false)
      {
        (*cit)->strength *= (1 - x) + (x * tmp.output_target / sum);
        if ((*cit)->strength <= 0.00001)
          (*cit)->strength = 0.00001;
        if ((*cit)->strength >= strength_bound)
          (*cit)->strength = strength_bound;
      }

    double mean = sum / n;

    if(EnableInhibitionPlasticity){
      cit = tmp.out_connections.begin();
      for (; cit != cend; cit++)
        if ((*cit)->strength < 0 && (*cit)->pre_cell->location.z < 2)
          (*cit)->strength = -mean / numberOfNeuronsPerOutputDirection;
    }

  } //cells
} //output balancing
void Network::spike_rate_adaptation()
{
  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];

    if (tmp.in_connections.size() <= 1)
      continue; //prevent changes in strength of relay connections

    tmp.spike_rate = tmp.spike_rate * (1 - tmp.spike_rate_dt) + tmp.epoch_spikes * tmp.spike_rate_dt;

    if (tmp.location.z == 2)
    {
      if (tmp.spike_rate > tmp.spike_rate_target)
        tmp.resistance *= (1 + tmp.input_target_dt);
      if (tmp.spike_rate < tmp.spike_rate_target)
        tmp.resistance *= (1 - tmp.input_target_dt);
    }
    if (tmp.location.z == 1 && !FreezeHiddenLayerWeights)
    {
      if (tmp.spike_rate > tmp.spike_rate_target)
        tmp.input_target *= (1 - tmp.input_target_dt);
      if (tmp.spike_rate < tmp.spike_rate_target)
        tmp.input_target *= (1 + tmp.input_target_dt);
    }
    assert(tmp.input_target != 0);

    tmp.epoch_spikes = 0; //counting anew for the next epoch
  }                       //for
} //spike_rate_adaptation
void Network::init_excitability()
{
  int x, y;
  for (int layer = 1; layer <= 2; layer++)
    for (x = 0; x < layer_size[layer]; x++)
      for (y = 0; y < layer_size[layer]; y++)
      {
        Cell &c = get_cell(x, y, layer);
        c.spike_rate_target = layer == 1 ? middle_excitability : output_excitability;
      }
}

void Network::disableCell(int x, int y, int z)
{
  //cell we want to disconnect from the network
  Cell &disMe = get_cell(x, y, z);
  disMe.enableSynapticInput = false;
}

void Network::StrengthenStrongWeights()
{
	string outputfile = "StrengthenStronWeights";
  int layer = 2;
  for (int x = 0; x < layer_size[layer]; x++)
    for (int y = 0; y < layer_size[layer]; y++)
    {
      Cell &c = get_cell(x, y, layer);
      vector<Connection *>::iterator cit = c.in_connections.begin();
      vector<Connection *>::iterator cend = c.in_connections.end();
      for (; cit != cend; cit++)
      {
        if ((*cit)->strength > thresholdWeightIncrease)
        {
      		LogService.GetLog(outputfile) << (*cit)->pre_cell->location.z << " " 
                                        << (*cit)->pre_cell->location.y << " " 
                                        << (*cit)->pre_cell->location.x <<  " " 
                                        << (*cit)->post_cell->location.z << " " 
                                        << (*cit)->post_cell->location.y << " " 
                                        << (*cit)->post_cell->location.x << " "
                                        << (*cit)->strength <<  " | ";
          (*cit)->strength = (*cit)->strength * (1 + percentWeightIncrease);
          LogService.GetLog(outputfile) << (*cit)->pre_cell->location.z << " " 
                                        << (*cit)->pre_cell->location.y << " " 
                                        << (*cit)->pre_cell->location.x <<  " " 
                                        << (*cit)->post_cell->location.z << " " 
                                        << (*cit)->post_cell->location.y << " " 
                                        << (*cit)->post_cell->location.x <<  " "
                                        << (*cit)->strength << endl;
        }
      }
    }
}

void Network::SerializeCellStates(string filename)
{
  LogService.GetLog(filename).precision(100);
  LogService.GetLog(filename) << layer_size[0] << " " << layer_size[1] << " " << layer_size[2] << endl;

  for (int i = 0; i < cells; i++)
  {
    Cell &cur = *cell[i];

    LogService.GetLog(filename) << cur.location.z << " "
                                << cur.location.y << " "
                                << cur.location.x << " "

                                << cur.input << " "
                                << cur.voltage << " "
                                << cur.last_voltage << " "
                                << cur.last_last_voltage << " "
                                << cur.u << " "
                                << cur.u_temp << " "
                                << cur.fired << " "
                                << cur.input_target << " "
                                << cur.output_target << " "
                                << cur.output_sum << " "
                                << cur.epoch_spikes << " "
                                << cur.spike_rate << " "
                                << cur.spike_rate_target << " "
                                << cur.resistance << " "
                                << cur.flash << " "
                                << cur.incoming_total << " "
                                << cur.fire_rate << " "
                                << cur.outgoing_target << " "
                                << cur.enableSynapticInput << endl;

    //    list<time_type> spike_log;             //memory for spikes (stdp)
  }
}

void Network::DeserializeCellStates(string filename)
{
  ifstream fin;
  fin.open(filename, std::ifstream::in);
  Cell temp;

  if (!fin.good())
    assert(false);

  int layerSize0, layerSize1, layerSize2;
  fin >> layerSize0 >> layerSize1 >> layerSize2;

  if (layer_size[0] != layerSize0 || layer_size[1] != layerSize1 || layer_size[2] != layerSize2)
  {
    cout << "Serialized network does not match dimensions of current network" << endl;
    assert(false);
  }

  while (
      fin >> temp.location.z >> temp.location.y >> temp.location.x

      >> temp.input 
      >> temp.voltage 
      >> temp.last_voltage 
      >> temp.last_last_voltage 
      >> temp.u 
      >> temp.u_temp 
      >> temp.fired 
      >> temp.input_target 
      >> temp.output_target 
      >> temp.output_sum 
      >> temp.epoch_spikes 
      >> temp.spike_rate 
      >> temp.spike_rate_target 
      >> temp.resistance 
      >> temp.flash 
      >> temp.incoming_total 
      >> temp.fire_rate 
      >> temp.outgoing_target 
      >> temp.enableSynapticInput)
  {

    Cell &cur = get_cell(temp.location.x, temp.location.y, temp.location.z);

    cur.input = temp.input;
    cur.voltage = temp.voltage;
    cur.last_voltage = temp.last_voltage;
    cur.last_last_voltage = temp.last_last_voltage;
    cur.u = temp.u;
    cur.u_temp = temp.u_temp;
    cur.fired = temp.fired;
    cur.input_target = temp.input_target;
    cur.output_target = temp.output_target;
    cur.output_sum = temp.output_sum;
    cur.epoch_spikes = temp.epoch_spikes;
    cur.spike_rate = temp.spike_rate;
    cur.spike_rate_target = temp.spike_rate_target;
    cur.resistance = temp.resistance;
    cur.flash = temp.flash;
    cur.incoming_total = temp.incoming_total;
    cur.fire_rate = temp.fire_rate;
    cur.outgoing_target = temp.outgoing_target;
    cur.enableSynapticInput = temp.enableSynapticInput;
  }
}

void Network::SerializeNetwork(string stateFileName, string weightFileName)
{
  SerializeCellStates(stateFileName);
  write_network(weightFileName);
}

void Network::DeserializeNetwork(string stateFileName, string weightFileName)
{
  DeserializeCellStates(stateFileName);
  read_network(weightFileName);
}




//only replaces connections between layers, does not do anything for recurrent connections
void Network::read_network_parts(const string fileName, bool inputToHiddenWeights, bool hiddenToOutputWeights)
{
  int rx, ry, rz, ox, oy, oz; //pre & post coordinates
  Coord in, out;
  string junk;
  double stren;
  ifstream network_f(fileName.c_str());
  assert(network_f.good());
  bool inputConnections[layers]; // clear incommming connections from these layers
  bool outputConnections[layers]; // clear outgoing connections from these layers
  
  for(int i = 0; i < layers; i++){
    outputConnections[i] = inputConnections[i] = false;
  }

  //must change both outgoing connections from input layer and incoming connections to hidden layer
  if(inputToHiddenWeights)
    outputConnections[0] = inputConnections[1] = true;

  //must change both outgoing connections from hidden layer and incoming connections to hidden layer
  if(hiddenToOutputWeights)
    outputConnections[1] = inputConnections[2] = true;
	
  //clear out nonrecurrent connections for all valid layers
  list<Connection *> allCons;
	for(int i = 0; i < cells; i++){
		Cell & cur = *cell[i];

    //clear out all nonrecurrent incomming connections
    if(inputConnections[cur.location.z]){
      for(vector<Connection *>::iterator it = cur.in_connections.begin();it != cur.in_connections.end(); ){
        //if connection coresponds to connection in previous layer then over write it. want to save recurrent connections
        Connection & c = *(*it);
        if( c.pre_cell->location.z == cur.location.z - 1){
          allCons.push_back(*it);
          it = cur.in_connections.erase(it);
        }
        else
          it++;
      }
    }

    //clear out all nonrecurrent outgoing connections
    if(outputConnections[cur.location.z]){
      for(vector<Connection *>::iterator it = cur.out_connections.begin();it != cur.out_connections.end();){
        Connection & c = *(*it);
        //if connection coresponds to connection in following layer then over write it. want to save recurrent connections
        if( c.post_cell->location.z == cur.location.z + 1){
          allCons.push_back(*it);
          it = cur.out_connections.erase(it);
        }
        else
          it++;
      } 
    }
	}

  allCons.sort();
  allCons.unique();
  for(list<Connection *>::iterator itt = allCons.begin();itt != allCons.end(); itt++){
    delete *itt;
  }

  while( network_f >> rz >> ry >> rx >> oz >> oy >> ox >> stren)
  {
    //cout<<tmp->strength<<"\n";
    if(outputConnections[rz] && inputConnections[oz]){
      Connection *tmp = new Connection(&get_cell(rx, ry, rz), &get_cell(ox, oy, oz), stren);
      get_cell(rx, ry, rz).out_connections.push_back(tmp);
      get_cell(ox, oy, oz).in_connections.push_back(tmp);
    }
  }
  numberOfNeuronsPerOutputDirection = getNumberOutputCellsPerDirection();
} //read_network_parts



void Network::RestoreHiddenLayerWeightsAndAllNeuronStatesFromFile(string stateFileName, string weightFileName){
  read_network_parts(weightFileName, true, false);
  DeserializeCellStates(stateFileName);
}
 
void Network::FreezeTopPercentagePerNeuron(double percentage){
  for(int x = 0; x < layer_size[2]; x++){ 
    for(int y = 0; y < layer_size[2]; y++){ 
      Cell & cell = get_cell(x, y, 2);
      Connection::freezeTopPercentageOfSynapses(cell.in_connections, percentage); 
    }
  }
}

void Network::FreezeTopPercentagePerDirection(double percentage){
  for (int directioni = 0; directioni < 3; directioni++)
  {
    for (int directionj = 0; directionj < 3; directionj++)
    {
      //Skip middle directions since no connections
      if (directioni == 1 && directionj == 1)
        continue;
      vector<Connection *> directionConnections;
      //for each output layer cell in the direction
      for (int reli = 0; reli < (layer_size[2] / 3); reli++)
      {
        for (int relj = 0; relj < (layer_size[2] / 3); relj++)
        {
          //real output layer coords
          int i = reli + (directioni * (layer_size[2] / 3));
          int j = relj + (directionj * (layer_size[2] / 3));
          Cell & cur = get_cell(i, j, 2);
          directionConnections.insert(directionConnections.begin(), cur.in_connections.begin(), cur.in_connections.end());
        }
      }
    Connection::freezeTopPercentageOfSynapses(directionConnections, percentage); 
    }
  }
}

void Network::FreezeTopPercentagePerLayer(double percentage){
  vector<Connection *> allConnections;
  for(int x = 0; x < layer_size[2]; x++){ 
    for(int y = 0; y < layer_size[2]; y++){ 
      Cell & cur = get_cell(x, y, 2);
      allConnections.insert(allConnections.begin(), cur.in_connections.begin(), cur.in_connections.end());
    }
  }
  Connection::freezeTopPercentageOfSynapses(allConnections, percentage); 
}

void Network::SpikeNeuronsInLayerByPoisson(int layer, double threshold){
  if(layer < 0 || layer > 2){
    cout << "Invalid layer index in SpikeNeuronsInLayerByPoisson " << layer << endl;
    assert(0);
  }
  if(threshold < 0 || threshold > 1){
    cout << "Invalid threshold value in SpikeNeuronsInLayerByPoisson " << layer << endl;
    assert(0);
  }
  int numberOfSpikes = 0;
  for(int x = 0; x < layer_size[layer]; x++){
    for(int y = 0; y < layer_size[layer]; y++){
      double randValue = ((double)rand()) / ((double)RAND_MAX);
      // if for the given layer this neuron should spike then do so
      if(randValue <= threshold){
        Cell & cell = get_cell(x,y,layer);
        cell.input += 0.025; // this value will need to be change if any layer besieds input layer is used. Homeostatic mechanisms will change the amout on input required to cause spiking
        numberOfSpikes++;
      }
    }
  }
}

void Network::SpikeNeuronsInLayerByPubPrivPoisson(int layer, double thresholdPub, double thresholdPriv){
  if(layer < 0 || layer > 2){
    cout << "Invalid layer index in SpikeNeuronsInLayerByPubPrivPoisson " << layer << endl;
    assert(0);
  }
  if(thresholdPub < 0 || thresholdPub > 1){
    cout << "Invalid thresholdPub value in SpikeNeuronsInLayerByPubPrivPoisson " << thresholdPub << endl;
    assert(0);
  }
  if(thresholdPriv < 0 || thresholdPriv > 1){
    cout << "Invalid thresholdPriv value in SpikeNeuronsInLayerByPubPrivPoisson " << thresholdPriv << endl;
    assert(0);
  }

  double pubInput = 0;
  double pubRandValue = ((double)rand()) / ((double)RAND_MAX);
  // if for the given layer this neuron should spike then do so
  if(pubRandValue <= thresholdPub)
    pubInput = 0.025;

  for(int x = 0; x < layer_size[layer]; x++){
    for(int y = 0; y < layer_size[layer]; y++){
      Cell & cell = get_cell(x,y,layer);
      cell.input += pubInput; //add public noise to every cell

      // implement PrivateNoise
      double privRandValue = ((double)rand()) / ((double)RAND_MAX);
      if(privRandValue <= thresholdPriv){
        cell.input += 0.025; // this value will need to be change if any layer besieds input layer is used. Homeostatic mechanisms will change the amout on input required to cause spiking
      }
    }
  }
}
// this one will not work with the new implementation of spike counts
void Network::SpikeNeuronsByAverageActivity(int layer){
  if(layer < 0 || layer > 2){
    cout << "Invalid layer index in SpikeNeuronsInLayerByPoisson " << layer << endl;
    assert(0);
  }
  int numberOfSpikes = 0;
  for(int x = 0; x < layer_size[layer]; x++){
    for(int y = 0; y < layer_size[layer]; y++){
      double randValue = ((double)rand()) / ((double)RAND_MAX);
      double threshold = (double)spikeCountsObj.spikeCounts[layer][x][y] / (double)spikeCountsObj.spikeCountsPeriod;
      // if for the given layer this neuron should spike then do so
      if(randValue <= threshold){
        Cell & cell = get_cell(x,y,layer);
        cell.input += 0.025; // this value will need to be change if any layer besieds input layer is used. Homeostatic mechanisms will change the amout on input required to cause spiking
        numberOfSpikes++;
      }
    }
  }
}
   
void Network::SpikeNeuronsByAverageBetweenToSpikeTimesArrays(SpikeCountsClass & spikeCounts1, SpikeCountsClass & spikeCounts2, int layer){
  if(layer < 0 || layer > 2){
    cout << "Invalid layer index in SpikeNeuronsInLayerByPoisson " << layer << endl;
    assert(0);
  }
  int numberOfSpikes = 0;
  for(int x = 0; x < layer_size[layer]; x++){
    for(int y = 0; y < layer_size[layer]; y++){
      double randValue = ((double)rand()) / ((double)RAND_MAX);
      double threshold1 = (double)spikeCounts1.spikeCounts[layer][x][y] / (double)spikeCounts1.spikeCountsPeriod;
      double threshold2 = (double)spikeCounts2.spikeCounts[layer][x][y] / (double)spikeCounts2.spikeCountsPeriod;
      double threshold = (threshold1 + threshold2 ) / 2;
      // if for the given layer this neuron should spike then do so
      if(randValue <= threshold){
        Cell & cell = get_cell(x,y,layer);
        cell.input += 0.025; // this value will need to be change if any layer besieds input layer is used. Homeostatic mechanisms will change the amout on input required to cause spiking
        numberOfSpikes++;
      }
    }
  }
}

void Network::SpikeNeuronsByUniformAverageBetweenToSpikeTimesArrays(SpikeCountsClass & spikeCounts1, SpikeCountsClass & spikeCounts2, int layer){
  if(layer < 0 || layer > 2){
    cout << "Invalid layer index in SpikeNeuronsInLayerByPoisson " << layer << endl;
    assert(0);
  }
  int numberOfSpikes = 0;
  double totalThreshhold = 0;
  for(int x = 0; x < layer_size[layer]; x++){
    for(int y = 0; y < layer_size[layer]; y++){
      double threshold1 = (double)spikeCounts1.spikeCounts[layer][x][y] / (double)spikeCounts1.spikeCountsPeriod;
      double threshold2 = (double)spikeCounts2.spikeCounts[layer][x][y] / (double)spikeCounts2.spikeCountsPeriod;
      totalThreshhold += threshold1;
      totalThreshhold += threshold2;
    }
  }
  // the 2 is becasue there are 2 spike time arrays
  double uniformThresh = totalThreshhold / (2.0 * double(layer_size[layer]) * double(layer_size[layer]));
  for(int x = 0; x < layer_size[layer]; x++){
    for(int y = 0; y < layer_size[layer]; y++){
      double randValue = ((double)rand()) / ((double)RAND_MAX);
      // if for the given layer this neuron should spike then do so
      if(randValue <= uniformThresh){
        Cell & cell = get_cell(x,y,layer);
        cell.input += 0.025; // this value will need to be change if any layer besieds input layer is used. Homeostatic mechanisms will change the amout on input required to cause spiking
        numberOfSpikes++;
      }
    }
  }
}

void Network::SpikeNeuronsByMaxBetweenToSpikeTimesArrays(SpikeCountsClass & spikeCounts1, SpikeCountsClass & spikeCounts2, int layer){
  if(layer < 0 || layer > 2){
    cout << "Invalid layer index in SpikeNeuronsInLayerByPoisson " << layer << endl;
    assert(0);
  }
  int numberOfSpikes = 0;
  for(int x = 0; x < layer_size[layer]; x++){
    for(int y = 0; y < layer_size[layer]; y++){
      double randValue = ((double)rand()) / ((double)RAND_MAX);
      double threshold1 = (double)spikeCounts1.spikeCounts[layer][x][y] / (double)spikeCounts1.spikeCountsPeriod;
      double threshold2 = (double)spikeCounts2.spikeCounts[layer][x][y] / (double)spikeCounts2.spikeCountsPeriod;
      double threshold = (threshold1 > threshold2 ) ? threshold1 : threshold2;
      // if for the given layer this neuron should spike then do so
      if(randValue <= threshold){
        Cell & cell = get_cell(x,y,layer);
        cell.input += 0.025; // this value will need to be change if any layer besieds input layer is used. Homeostatic mechanisms will change the amout on input required to cause spiking
        numberOfSpikes++;
      }
    }
  }
}


void Network::resetCellParameters(){
  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];
    tmp.sig = tmp.sigOrig;
    tmp.al = tmp.alOrig;
  }
}

void Network::setCellParameters(int layer, double sig, double al){
  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];
    if(tmp.location.z == layer){
      tmp.sig = sig;
      tmp.al = al;
    }
  }
}

void Network::scaleWeights(int layer, double scale)
{
  for (int i = 0; i < cells; i++)
  {
    Cell &tmp = *cell[i];
    //if not in correct layer then skip
    if(tmp.location.z != layer)
      continue;

    vector<Connection *>::iterator cit = tmp.in_connections.begin();
    vector<Connection *>::iterator cend = tmp.in_connections.end();
    for (; cit != cend; cit++)
      (*cit)->strength = (*cit)->strength * scale;
  }
} 


