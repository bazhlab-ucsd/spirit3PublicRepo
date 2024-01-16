#include <iostream>
#include <fstream>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm> 
#include <ctime>
#include <chrono>
#include "./classes/coord.h"
#include "./classes/critter.h"
#include "./classes/timeStructure.h"
#include "./classes/trace.h"
#include "./classes/connection.h"
#include "./classes/cell.h"
#include "./classes/network.h"
#include "./classes/environment.h"
#include "./classes/outputLogService.h"
#include "./classes/toStr.h"
#include "./classes/stageManager.h"

void Connection::dump(int level)
{
	if (pre_cell == NULL || post_cell == NULL)
	{
		cout << "NULL!\n";
		return;
	}
	cout << pre_cell->location.z << pre_cell->location.y << pre_cell->location.x << "->"
		 << post_cell->location.z << post_cell->location.y << post_cell->location.x << ": " << strength << "\n";
}

using namespace std;
using namespace std::chrono;

#if __GNUC__ > 5
#define const_expr constexpr
#else
#define const_expr const
#endif

#define HiddenLayerDim 28

int mode_init = 0;
int h_init = 1;
int v_init = 0;
int dpos_init = -1;
int dneg_init = 0;
int hidden_neuron_dropoff_seed = -1;
int initialNumberHiddenLayerNeurons = 784;

int train_time = 0;
int switch_time1 = 0;
int switch_time2 = 0;
int sleepy_time = 0;
int wake_time = 0;

int mode_swt = 0;
int h_swt = 0;
int v_swt = 1;
int dpos_swt = 0;
int dneg_swt = -1;
int properDynamics = 1;
int applyPunishment = 0;
int epochsPerAeon = 100;

int enableSpikeRateAdaptationOverride = 1;
int enableInputBalencingOverride = 1;

int trainingSpikeRateAdaptation = 1;
int trainingInputBalencing = 1;

string output_location;
string DeserializeCellStateFile;
string DeserializeWeightFile;

int initAmt;

TimeStructure Time;

double stdp_scale = 0.4;
double strength_bound = 1;
double strength_middle_bound = 0.012; //cap for middle layer only
double middle_excitability = 0.01;	 //commandline argument for spike_rate_target in different layers
double output_excitability = 0.50;
double punishment = -0.0001; //how fast should we punish no food obtained
double freezeTopPercentage = 0;
double train1TillPerformanceThreshold = 0.7;
double train2TillPerformanceThreshold = 0.7;

//simulated sleep increase
double percentWeightIncrease = 0;
double thresholdWeightIncrease = 0.3;

double poissonEpochInputLayerThreshold = 0.5;
double poissonStepInputLayerThreshold = 0.1;

double pubPrivPrivateStepInputLayerThreshold = 0.05;
double pubPivPublicStepInputLayerThreshold = 0.1;

double cellParamNoiseAl = 3.56;
double cellParamNoiseSig = 0.06;

double noiseWeightScaleFactor = 1;

#include "sdl.cpp"
#include "params.h"

high_resolution_clock::time_point simStartTime;
high_resolution_clock::time_point aeonStartTime;

inline void aeonTime(OutputLogService & LogService, string stageName){
  high_resolution_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = now-aeonStartTime;
  aeonStartTime = now;
  LogService.GetLog("PerformanceTime") << "Aeon Time: " << elapsed_seconds.count() << " " << stageName << endl;
  cout << "Aeon Time: " << elapsed_seconds.count() << " " << stageName << endl;
}

inline void simTime(OutputLogService & LogService){
  high_resolution_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = now-simStartTime;
  LogService.GetLog("PerformanceTime") << "Sim Time: " << elapsed_seconds.count() << endl;
  cout << "Sim Time: " << elapsed_seconds.count() << endl;
}

int main(int argc, char *argv[])
{

	int gui = 1; //whether to use graphical interface or no
	string init_network = "twol.conn";
  string init_stages = "TrainingStages.config";
	int suffix = 0; //run suffix to be added to happiness & output file && use suffix as rand seed

	add_int_param(mode_init);
	add_int_param(h_init);
	add_int_param(v_init);
	add_int_param(dpos_init);
	add_int_param(dneg_init);
	add_int_param(hidden_neuron_dropoff_seed);
  add_int_param(initialNumberHiddenLayerNeurons);

	add_int_param(mode_swt);
	add_int_param(h_swt);
	add_int_param(v_swt);
	add_int_param(dpos_swt);
	add_int_param(dneg_swt);
  add_int_param(properDynamics);
  add_int_param(applyPunishment);
  add_int_param(epochsPerAeon);

	add_int_param(switch_time1);
	add_int_param(switch_time2);
	add_int_param(train_time);
	add_int_param(sleepy_time);
	add_int_param(wake_time);

	add_int_param(enableSpikeRateAdaptationOverride);
	add_int_param(enableInputBalencingOverride);
  add_int_param(trainingSpikeRateAdaptation);
  add_int_param(trainingInputBalencing);

	add_int_param(initAmt);
	add_string_param(output_location);
	add_string_param(DeserializeCellStateFile);
	add_string_param(DeserializeWeightFile);

	add_int_param(gui);
	//add_int_param(aeons);
	add_int_param(suffix);
	add_double_param(middle_excitability);
	add_double_param(output_excitability);
	add_double_param(stdp_scale);
	add_double_param(strength_bound);
	add_double_param(strength_middle_bound);
	add_string_param(init_network); //initial strengths of connections
	add_string_param(init_stages); 
	add_double_param(punishment);
  add_double_param(freezeTopPercentage);
  add_double_param(train1TillPerformanceThreshold);
  add_double_param(train2TillPerformanceThreshold);

	add_double_param(percentWeightIncrease);
	add_double_param(thresholdWeightIncrease);
  add_double_param(poissonEpochInputLayerThreshold);
  add_double_param(poissonStepInputLayerThreshold);

  add_double_param(pubPrivPrivateStepInputLayerThreshold);
  add_double_param(pubPivPublicStepInputLayerThreshold);

  add_double_param(cellParamNoiseAl);
  add_double_param(cellParamNoiseSig);

  add_double_param(noiseWeightScaleFactor);

	//assert(load_parameters(argv[1]));
	assert(cmdline_parameters(argc, argv));

  Time.setEpochsPerAeon(epochsPerAeon);

	const char *output_location_char = output_location.c_str();
	struct stat st = {0};
	if (stat(output_location_char, &st) == -1)
	{
    int error = system(("mkdir -p "+ output_location).c_str()); 
    if(error != 0){
      cout << "Error making files" << endl;
      assert(false);
    }
	}

	OutputLogService LogService(output_location + "/", to_str(suffix), Time);
	print_parameters(LogService);

	//get random array of hidden layer coords to remove hidden layer neurons
	Coord removeHiddenCoord[HiddenLayerDim * HiddenLayerDim];
	for(int i = 0; i < HiddenLayerDim; i++){
		for(int j = 0; j < HiddenLayerDim; j++){
			int pos = (j * HiddenLayerDim) + i;
			removeHiddenCoord[pos].x = i;
			removeHiddenCoord[pos].y = j;
			removeHiddenCoord[pos].z = 1;
		}
	}

	srand(hidden_neuron_dropoff_seed);
	random_shuffle(removeHiddenCoord, removeHiddenCoord + (HiddenLayerDim * HiddenLayerDim));
	int removeHiddenCoord_Idx = 0;
  int numberHiddenNeuronsToRemove = ((HiddenLayerDim * HiddenLayerDim) - initialNumberHiddenLayerNeurons);

	srand(suffix);
	if (suffix){
		srand(suffix);
		//srand(suffix + getpid() + time(NULL));
  }
	
	Environment environment(mode_init, h_init, v_init, dpos_init, dneg_init, LogService);
	if (train_time != 0)
	{
		environment.switch_env_state(0, 0, 0, 0, 0);
	}
	initAmt = environment.getFoodAmt();
  environment.applyPunishment = applyPunishment;
	//Environment environment;
	//Environment environment("rand10.env");
	//environment.printEnvironment();
	//environment.printEnvironment(); //to screen only

	display screen;
	screen.init(gui);
	Network network(LogService);
  network.read_network(init_network, removeHiddenCoord, numberHiddenNeuronsToRemove);
	network.init_balancing();

  StageManager stageManager(LogService);
  stageManager.initStages(init_stages);

  aeonStartTime = simStartTime = std::chrono::system_clock::now();
	while (stageManager.continueSimulation())
	{
    Time.ctime++;
		if (Time.aeon_check())
		{
      //aeonTime(LogService, stageManager.getCurrentStageType());

			LogService.GetLog(LogService.HappinessLogKey)
				<< Time.aeons() << " "
				<< environment.critter.happy << " "
				<< environment.critter.judgement << " "
				<< (HiddenLayerDim * HiddenLayerDim) - removeHiddenCoord_Idx << " " // number of hidden layer neurons present
				<< environment.critter.getInstantaneousFoodAverage() << " " 
        << network.aeon_spikes << " "
        << network.aeon_spikes_output << endl;

			network.aeon_spikes = 0;
			network.aeon_spikes_output = 0;

			// LogService.GetLog(LogService.ConnSdLogKey)
			// 	<< Time.aeons() << " "
			// 	<< network.get_connections_variability(0) << endl;

			if (Time.aeons() % 100 == 0)
			{
				//network.write_output_matrix();
				network.write_network("output.conn." + to_str(Time.aeons()));
        cout << "Status:" << endl
          << "   Time: " << Time.aeons() << endl
          << "   Trial Number: " << suffix << endl
          << "   Happy: " << environment.critter.happy << endl
          << "   Judgement: " << environment.critter.judgement << endl
          << "   Instantaneous Food Average: " << environment.critter.getInstantaneousFoodAverage() << endl
          << "   Aeon Spikes:" << network.aeon_spikes << endl;
			}

      stageManager.aeonHandler(Time, network, environment);
			
			//remove hidden layer neurons over time randomly
			//run this assuming you have starting weights
			//   let happy and judgment euqalize first
      
      //   this should be moved to a stage in the stage manager
			//if ( Time.aeons() > 500 && Time.aeons() % 50 == 0 && removeHiddenCoord_Idx < HiddenLayerDim * HiddenLayerDim && hidden_neuron_dropoff_seed > 0)
			//{
			//	network.disableCell(
			//		removeHiddenCoord[removeHiddenCoord_Idx].x,
			//		removeHiddenCoord[removeHiddenCoord_Idx].y,
			//		removeHiddenCoord[removeHiddenCoord_Idx].z);
			//	removeHiddenCoord_Idx++;
			//}
		}
		if (Time.epoch_check())
		{
      stageManager.epochHandler(Time, network, environment);
      if(network.EnableHighFrequencyWeightPrinting){
				network.write_network("highFewqOutput.conn." + to_str(Time.epochs()));
      }
		}
    stageManager.stepHandler(Time, network, environment);

		network.calc_next_state(properDynamics);
		network.apply_firings(environment.critter);

		screen.drawNetwork(network);
		screen.toggleVideo(environment);

		if (Time.move_critter())
		{
			//cout<< environment.critter.position.x <<' ' << environment.critter.position.y << "\n";
			environment.move_critter(network); //(requires env, wrap happy; triggers reward)
			screen.drawEnvironment(environment);
		}
		if (Time.flash_cells_check())
			environment.flash_cells(network);

		if (Time.epoch_check() && network.ApplyPlasticity && network.EnableSpikeRateAdaptation && enableSpikeRateAdaptationOverride == 1)
		{
			network.spike_rate_adaptation();
		}
	}
  simTime(LogService);
  stageManager.printCompletedStages();
	return 0;
}
