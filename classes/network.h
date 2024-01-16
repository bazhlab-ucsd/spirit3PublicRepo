#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <stdlib.h>
#include <fstream>

#include "cell.h"
#include "critter.h"
#include "outputLogService.h"
#include "timeStructure.h"

extern TimeStructure Time;
extern double strength_bound;
extern double strength_middle_bound;
extern double middle_excitability;
extern double output_excitability;
extern double punishment;
extern double percentWeightIncrease;
extern double thresholdWeightIncrease;

extern int enableInputBalencingOverride;

static const int layers = 3;                      //number of layers and z(+1!) of output layer at the same time
//  static const int layer_size[4]={input_size,middle_ex_size,middle_in_size,output_size}; //later move it to the class
static const int layer_size[layers] = {7, 28, 9}; //later move it to the class


enum InputBalencingTypeEnum{
  multiplicitive,
  additive
};

class SpikeCountsClass{
  public:
  vector<long long int **>  spikeCounts;
  long long int spikeCountsPeriod;
};

class Network
{

private:
  OutputLogService &LogService;
  void InitNetwork();
  inline bool ShouldSkpHiddenCellPlasticity(Cell & cell){
    return FreezeHiddenLayerWeights && cell.location.z == 1;
  }


public:
  static const int input_size = 7;
  static const int middle_size = 28;
  static const int output_size = 9;
  static const int cells = input_size * input_size + middle_size * middle_size + output_size * output_size;
  bool ApplyPlasticity;
  bool FreezeHiddenLayerWeights;
  bool EnableSpikeRateAdaptation;
  bool EnableInputBalencing;
  bool logSpiking;
  bool logStdp;
  bool logAppliedStdp;
  bool EnableLTD;
  bool EnableLTP;
  bool EnableInhibitionPlasticity;
  InputBalencingTypeEnum InputBalencingType;
  bool EnableHighFrequencyWeightPrinting;

  bool EnableSpikeCounts;
  SpikeCountsClass spikeCountsObj;
  SpikeCountsClass spikeCountsTrain1Perf;
  SpikeCountsClass spikeCountsTrain2Perf;
  SpikeCountsClass spikeCountsTest1;
  SpikeCountsClass spikeCountsTest2;
  SpikeCountsClass spikeCountsTrain1;
  SpikeCountsClass spikeCountsTrain2;

  void copySpikeCountsToStageSpikeCounts(SpikeCountsClass & destination);

  Cell input_layer[input_size][input_size];
  Cell middle_layer[middle_size][middle_size];
  Cell output_layer[output_size][output_size];
  Cell *cell[cells]; //one dimensional list of all cells above, for openMP iteration

  list<Coord> cells_fired_cache; //list of cells which fired in the last iteration of calc()
  int aeon_spikes;
  int aeon_spikes_output;
  int numberOfNeuronsPerOutputDirection;
  int numberIterationsOfAdditiveHsp;

  void dump();
  void dump(int level);
  Network(OutputLogService &logService);

  //functions for access to cell via its coordinates
  inline Cell &get_cell(int x, int y, int z)
  {
		if(z >= layers || x >= layer_size[z] || y >= layer_size[z]){
			assert(false);
		}
    switch (z)
    {
    case 0:
      return input_layer[x][y];
    case 1:
      return middle_layer[x][y];
    case 2:
      return output_layer[x][y];
    default:
      assert(false);
    }
  } //cell
  inline Cell &get_cell(const Coord &c) { return get_cell(c.x, c.y, c.z); }
  inline Cell &operator[](const Coord &c) { return get_cell(c); }

  int getNumberOutputCellsPerDirection();

  void write_network(const string f);

  //output matrix of output|_sum for debugging purposes
  void write_output_matrix();

#define sqr(x) ((x) * (x))
  //print coefficient of variation of strengths of connections from "layer"
  double get_connections_variability(int layer);

  void read_network(const string f, Coord * removeHiddenCoord = NULL, int numberHiddenNeuronsToRemove = 0);
  
  //only replaces connections between layers, does not do anything for recurrent connections
  void read_network_parts(const string fileName, bool inputToHiddenWeights, bool hiddenToOutputWeights);

  //calculate voltages of all cells for the next step
  void calc_next_state(int properDynamics);

  void compute_STDP(Cell &cell);

  //compute direction of next move of critter, winner is cell which has most activity && fired first in case of tie
  //when no firing, repeat movement from last step
  void compute_critter_direction(Critter &critter, Cell &cell);

  //apply firing results to connections and critter
  void apply_firings(Critter &critter);

  //happens only we are in reward part of critter movement; we want to make active connection stronger and inactive weaker
  void apply_STDP_traces(double dopamine);

  void init_balancing();

  //hetero synaptic plasticity
  void input_balancing();
  void output_balancing();
  void spike_rate_adaptation();
  void init_excitability();

  void disableCell(int x, int y, int z);
	void StrengthenStrongWeights();

	void SerializeCellStates(string filename);
	void DeserializeCellStates(string filename);
	
	void SerializeNetwork(string stateFileName, string weightFileName);
	void DeserializeNetwork(string stateFileName, string weightFileName);
  void RestoreHiddenLayerWeightsAndAllNeuronStatesFromFile(string stateFileName, string weightFileName);

  void FreezeTopPercentagePerNeuron(double percentage);
  void FreezeTopPercentagePerDirection(double percentage);
  void FreezeTopPercentagePerLayer(double percentage);

  //Every neuron has a chance to spike in the layer accoring to the Poisson process described by the parameters
  //threshold - threshold used to compare with random number. If threshold >= random number then the coresponding neuron will spike. random number values between 0 and 1
  void SpikeNeuronsInLayerByPoisson(int layer, double threshold);
  void SpikeNeuronsInLayerByPubPrivPoisson(int layer, double thresholdPub, double thresholdPriv);
  void SpikeNeuronsByAverageActivity(int layer);
  void SpikeNeuronsByAverageBetweenToSpikeTimesArrays(SpikeCountsClass & spikeCounts1, SpikeCountsClass & spikeCounts2, int layer);
  void SpikeNeuronsByUniformAverageBetweenToSpikeTimesArrays(SpikeCountsClass & spikeCounts1, SpikeCountsClass & spikeCounts2, int layer);
  void SpikeNeuronsByMaxBetweenToSpikeTimesArrays(SpikeCountsClass & spikeCounts1, SpikeCountsClass & spikeCounts2, int layer);
  void clearSpikecounts(SpikeCountsClass & curSpikeCounts);
  void initSpikeCountsvector(SpikeCountsClass & curSpikeCountsObj);
  void printSpikeCounts(string title, SpikeCountsClass & curSpikeCounts);
  void resetCellParameters();
  void setCellParameters(int layer, double sig, double al);
  void scaleWeights(int layer, double scale);
}; //Network

#endif // NETWORK_H
