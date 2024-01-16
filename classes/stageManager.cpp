#include "stageManager.h"
#include <assert.h>

using namespace std;

StageManager::StageManager(OutputLogService &logService) : LogService(logService)
{
  continueSim = false;
}

void StageManager::initStages(string filePath){//important to keep stages in proper order when loading
  stagesRemaining.clear();
  stagesCompleted.clear();

  ifstream stagesFile(filePath.c_str());// read in file
  assert(stagesFile.good());
  string line;

  while (getline(stagesFile, line)) //read in file line by line
  {
    istringstream iss(line);
    string stageTypeString;
    time_type duration;
    bool validReadLine = (iss >>  stageTypeString >> duration)? true : false;
    StageType st = stringToStageTypeEnum( stageTypeString);
    Stage * tempStage = typeEnumToStageObject(st,0);
    

    if(!validReadLine && (tempStage->isInstantaneousStage() ||tempStage->isUndefiniteStage())) // an unique action that will have an improper time value when reading in
    {
      duration = -1; 
    }
    else if(st == undefined || !validReadLine || duration < 1)
    {
      cout << "Error in reading in stages: "
        << "| stage string: " << stageTypeString  
        << "| stage type " << st  
        << "| validReadLine " << validReadLine  
        << "| duration " << duration << endl;  
      assert(false);
    }
    delete tempStage;

    stagesRemaining.push_back(typeEnumToStageObject(stringToStageTypeEnum(stageTypeString), duration));
    //add current section to the endtime
  }
  //reverse stages becasue we will be poping stages off the back
  reverse(stagesRemaining.begin(), stagesRemaining.end());
  if(stagesRemaining.size() > 0)
    continueSim = true;
  else{
    cout << "Error: No stages properly read in" << endl;
    assert(false);
  }
  curStage = stagesRemaining.back();
  printRemainingStages();
}

void StageManager::printCompletedStages(){
  time_type startTime = 0;
  for(unsigned int i = 0; i < stagesCompleted.size(); i++){
    LogService.GetLog("stagesStartTimes") 
      << typeEnumToStageString(stagesCompleted[i]->stageType) << " "
      << startTime << endl;
      startTime += stagesCompleted[i]->durationCompleted;

    LogService.GetLog("stagesDurations") 
      << typeEnumToStageString(stagesCompleted[i]->stageType) << " "
      << stagesCompleted[i]->durationCompleted << endl;
  }
}

void StageManager::printRemainingStages(){
  time_type startTime = 0;
  for(unsigned int i = 0; i < stagesRemaining.size(); i++){
    LogService.GetLog("stagesRemaining") 
      << typeEnumToStageString(stagesRemaining[i]->stageType) << " "
      << startTime << endl;
      startTime += stagesRemaining[i]->durationCompleted;

    LogService.GetLog("stagesRemaining") 
      << typeEnumToStageString(stagesRemaining[i]->stageType) << " "
      << stagesRemaining[i]->durationRemaining << endl;
  }
}

void StageManager::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  //if no stages or at end of simulation
  if(stagesRemaining.size() == 0){
    continueSim = false; // simulation is finished
    return;
  }
  //
  //put in loop to do all instantaneous sections sequentially
  bool continueLoop = true;
  while(continueLoop){
    if(curStage->isInstantaneousStage() || curStage->durationRemaining == 0){ 
      // if instantaneous stage then we need to call aeon handler once 
      if (curStage->isInstantaneousStage())
        curStage->aeonHandler(Time, network, environment);

      // perform cleanup for current stage
      curStage->stageCleanUp(Time, network, environment);
      environment.critter.clearInstantaneousFoodAverageArray();

      // move to next stage
      stagesCompleted.push_back(curStage);
      stagesRemaining.pop_back();
      if(stagesRemaining.size() > 0){
        curStage = stagesRemaining.back();
      }
      else{
        break;
      }
    }
    //stage needs to execute
    else{
      curStage->aeonHandler(Time, network, environment);

      //if a stage with a specific timer then decrement time remaining
      if(!curStage->isUndefiniteStage())
        curStage->durationRemaining--;

      curStage->durationCompleted++;

      continueLoop = false;
    }

  }
}

void StageManager::epochHandler(TimeStructure & Time, Network & network, Environment & environment){
  //each one of these cases will hit EVERY epoch the current stage matches the coresponding case
  curStage->epochHandler(Time, network, environment);
}

void StageManager::stepHandler(TimeStructure & Time, Network & network, Environment & environment){
  //each one of these cases will hit EVERY epoch the current stage matches the coresponding case
  curStage->stepHandler(Time, network, environment);
}

void SetNetEnvDefaults(Environment &environment, Network &network){
  environment.applyConstantReward = 0;
  network.ApplyPlasticity = true;
  network.EnableLTD = true;
  network.EnableLTP = true;
  network.logSpiking = false;
  network.logStdp = false;
  network.logAppliedStdp = false;
  network.EnableSpikeRateAdaptation = true;
  network.EnableInputBalencing = true;
  network.EnableSpikeCounts = true;
  network.EnableInhibitionPlasticity = true;
  network.InputBalencingType = multiplicitive;
  network.EnableHighFrequencyWeightPrinting = false;
  network.resetCellParameters();
  network.clearSpikecounts(network.spikeCountsObj);
}

void SetNetEnvUnsupervised(Environment &environment, Network &network){
  SetNetEnvDefaults(environment, network);
  environment.switch_env_state(0, 0, 0, 0, 0);
}

void SetNetEnvTrainTask1(Environment &environment, Network &network){
  SetNetEnvDefaults(environment, network);
  network.EnableInputBalencing = trainingInputBalencing == 1;
  network.EnableSpikeRateAdaptation = trainingSpikeRateAdaptation == 1;
  environment.switch_env_state(1, h_init, v_init, dpos_init, dneg_init);
  network.logSpiking = true;
}

void SetNetEnvTrainTask2(Environment &environment, Network &network){
  SetNetEnvDefaults(environment, network);
  network.EnableInputBalencing = trainingInputBalencing == 1;
  network.EnableSpikeRateAdaptation = trainingSpikeRateAdaptation == 1;
  environment.switch_env_state(1, h_swt, v_swt, dpos_swt, dneg_swt);
  network.logSpiking = true;
}

void SetNetEnvTestTask1(Environment & environment, Network &network){
  SetNetEnvDefaults(environment, network);
  environment.switch_env_state(1, h_init, v_init, dpos_init, dneg_init);
  network.ApplyPlasticity = false;
  network.logSpiking = true;
  network.EnableSpikeCounts = true;
  network.clearSpikecounts(network.spikeCountsObj);
}

void SetNetEnvTestTask2(Environment & environment, Network &network){
  SetNetEnvDefaults(environment, network);
  environment.switch_env_state(1, h_swt, v_swt, dpos_swt, dneg_swt);
  network.ApplyPlasticity = false;
  network.logSpiking = true;
  network.EnableSpikeCounts = true;
  network.clearSpikecounts(network.spikeCountsObj);
}

int returnGreaterAbsVal(int x, int y){
  return abs(x) > abs(y) ? x : y;
}

void SetNetEnvTestTask1and2(Environment & environment, Network &network){
  SetNetEnvDefaults(environment, network);
  environment.switch_env_state(0, returnGreaterAbsVal(h_init, h_swt), returnGreaterAbsVal(v_init, v_swt), returnGreaterAbsVal(dpos_init, dpos_swt), returnGreaterAbsVal(dneg_init, dneg_swt));
  network.ApplyPlasticity = false;
  network.logSpiking = true;
  network.EnableSpikeCounts = true;
  network.clearSpikecounts(network.spikeCountsObj);
}

void task1FoodNoiseEpoch(Environment & environment, Network &network){
  // get the rewarded an punished food types here
  FoodType rewardedFood = getFoodTypeHelper(h_init, v_init, dpos_init, dneg_init, 1);
  //FoodType punishedFood = getFoodTypeHelper(h_init, v_init, dpos_init, dneg_init, -1);

  //get the second pixel of the current rewarded food partlicle
  int secondPixelX = INT_MAX;
  int secondPixelY = INT_MAX;
  getSecondPixelCoord(secondPixelX, secondPixelY, rewardedFood);

  //get bounds of input layer based on coordinates of food particle
  int xMin = 0;
  int yMin = 0; 
  int xMax = network.input_size;
  int yMax = network.input_size;

  if(secondPixelX == -1)
    xMin = xMin + 1;
  else if(secondPixelX == 1)
    xMax = xMax - 1;

  if(secondPixelY == -1)
    yMin = yMin + 1;
  else if(secondPixelY == 1)
    yMax = yMax - 1;

  int xRange = xMax - xMin;
  int yRange = yMax - yMin;

  int xRand = (rand() % xRange) + xMin;
  int yRand = (rand() % yRange) + yMin;

  if(xRand < 0 || yRand < 0 || xRand >= network.input_size || yRand >= network.input_size){
    cout << "task1FoodNoiseEpoch first pixel out of bounds " << xRand << " " << yRand << endl;
    assert(false);
  }

  if(xRand + secondPixelX < 0 || yRand + secondPixelY < 0 || xRand + secondPixelX >= network.input_size || yRand + secondPixelY >= network.input_size){
    cout << "task1FoodNoiseEpoch second pixel out of bounds "<< xRand + secondPixelX << " " << yRand + secondPixelY << endl;
    assert(false);
  }

  Cell & cell1 = network.get_cell(xRand, yRand, 0);
  cell1.input += 0.025; 

  Cell & cell2 = network.get_cell(xRand + secondPixelX, yRand + secondPixelY, 0);
  cell2.input += 0.025; 
}

// ----------StageUnsupervised----------
void StageUnsupervised::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvUnsupervised(environment, network);
  }
}

// ----------StageTrain1----------
void StageTrain1::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvTrainTask1(environment, network);
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StageTrain1::stageCleanUp(TimeStructure & Time, Network & network, Environment & environment){
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTrain1);
}

// ----------StageTrain2----------
void StageTrain2::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvTrainTask2(environment, network);
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StageTrain2::stageCleanUp(TimeStructure & Time, Network & network, Environment & environment){
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTrain2);
}

// ----------StageTest1----------
void StageTest1::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvTestTask1(environment, network);
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StageTest1::stageCleanUp(TimeStructure & Time, Network & network, Environment & environment){
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTest1);
}

// ----------StageTest2----------
void StageTest2::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvTestTask2(environment, network);
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StageTest2::stageCleanUp(TimeStructure & Time, Network & network, Environment & environment){
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTest2);
}

// ----------StageTest1and2----------
// 
void StageTest1and2::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvTestTask1and2(environment, network);
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

// ----------StageInterleaved----------
void StageInterleaved::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  SetNetEnvDefaults(environment, network);

  if ((Time.aeons()) % 2 == 0) {
    SetNetEnvTrainTask1(environment, network);
  }
  else
  {
    SetNetEnvTrainTask2(environment, network);
  }
  if(durationCompleted <=1) 
  {
    network.logSpiking = true;
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

// ----------StageFreezeInputHiddenWeights----------
void StageFreezeInputHiddenWeights::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  SetNetEnvDefaults(environment, network);
  network.FreezeHiddenLayerWeights = true;
}
//
// ----------StageFreezeTopHiddenOutputWeights----------
void StageFreezeTopHiddenOutputWeights::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  network.FreezeTopPercentagePerNeuron(0.1);
}


// ----------StageBump----------
void StageBump::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  SetNetEnvDefaults(environment, network);
  network.StrengthenStrongWeights();
}

// ----------StageTrain1TillPerformance----------
void StageTrain1TillPerformance::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvDefaults(environment, network);
    //set the environment and continue
    SetNetEnvTrainTask1(environment, network);
    network.logSpiking = true;
  }
  //check performance and see if you should continue
  else if(environment.critter.getInstantaneousFoodAverage() >= train1TillPerformanceThreshold){
    durationRemaining = 0; //set durationRemainin to zero so that we can continue to the next stage
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StageTrain1TillPerformance::stageCleanUp(TimeStructure & Time, Network & network, Environment & environment){
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTrain1Perf);
}

// ----------StageTrain2TillPerformance----------
void StageTrain2TillPerformance::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvDefaults(environment, network);
    //set the environment and continue
    SetNetEnvTrainTask2(environment, network);
    network.logSpiking = true;
  }
  //check performance and see if you should continue
  else if(environment.critter.getInstantaneousFoodAverage() >= train2TillPerformanceThreshold){
    durationRemaining = 0; //set durationRemainin to zero so that we can continue to the next stage
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StageTrain2TillPerformance::stageCleanUp(TimeStructure & Time, Network & network, Environment & environment){
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTrain2Perf);
}

// ----------StagePoissonEpochSpikingInputLayer----------
void StagePoissonEpochSpikingInputLayer::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    network.logSpiking = true;
    network.EnableSpikeRateAdaptation = false;
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StagePoissonEpochSpikingInputLayer::epochHandler(TimeStructure & Time, Network & network, Environment & environment){
  network.SpikeNeuronsInLayerByPoisson(0, poissonEpochInputLayerThreshold);
}

// ----------StagePoissonStepSpikingInputLayer----------
void StagePoissonStepSpikingInputLayer::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    network.logSpiking = true;
    network.EnableSpikeRateAdaptation = false;

  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

// ----------StageTask1FoodNoise----------
void StageTask1FoodNoise::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    network.logSpiking = true;
    network.EnableSpikeRateAdaptation = false;
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

void StageTask1FoodNoise::epochHandler(TimeStructure & Time, Network & network, Environment & environment){
  task1FoodNoiseEpoch(environment, network);
}



// ----------StagePubPrivNoiseByStep----------
void StagePubPrivNoiseByStep::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    network.logSpiking = true;
    network.EnableSpikeRateAdaptation = false;
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
}

// ----------StagePoissonByNeuron----------
void StagePoissonByNeuron::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  //int layer = 2;
  if(durationCompleted == 0) // if first time begining this stage
  {
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    network.logSpiking = true;
    //network.logStdp = true;
    //network.logAppliedStdp = true;
    network.EnableSpikeRateAdaptation = true;
    network.EnableInputBalencing = true;
    network.EnableInhibitionPlasticity = true;
    //network.InputBalencingType = additive;
    //network.EnableLTD = false;
    network.printSpikeCounts("train1Perf", network.spikeCountsTrain1Perf);
    network.printSpikeCounts("train2Perf", network.spikeCountsTrain2Perf);
    //network.EnableHighFrequencyWeightPrinting = true;
    // modify sensitivity for output layer
    //network.setCellParameters(layer, cellParamNoiseSig, cellParamNoiseAl);
    //network.write_network("weightsBeforeBump");
    //network.scaleWeights(layer, noiseWeightScaleFactor);
    //network.write_network("weightsAfterBump");
  }
  if(durationCompleted == 5) 
  {
    network.logSpiking = false;
  }
  //if(durationCompleted == 500) 
  //{
  //  network.EnableHighFrequencyWeightPrinting = false;
  //}
  //if(durationRemaining == 1){
  //// scale weights donwn and freeze network so no noise is applied for nonscaled weights
  //// this should be changed for real implementation but it is fine for preliminary testing. it will juest waste 1 aeon

  //  network.ApplyPlasticity = false;
  //  network.scaleWeights(layer, 1/noiseWeightScaleFactor);
  //}
}

void StagePoissonByNeuron::epochHandler(TimeStructure & Time, Network & network, Environment & environment){
  network.SpikeNeuronsByAverageBetweenToSpikeTimesArrays(network.spikeCountsTrain1Perf, network.spikeCountsTrain2, 1);
}

// ----------StagePoissonByNeuron----------
void StageInterleavedSleepTrain2::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  // train two
  if(durationCompleted % 2 == 0) // if first time begining this stage
  {
    SetNetEnvTrainTask2(environment, network);
    applyNoiseFlag = false;
  }
  else{ // noise averaging 1 and 2
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTrain2);
    applyNoiseFlag = true;
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    //network.logStdp = true;
    //network.logAppliedStdp = true;
    network.EnableSpikeRateAdaptation = true;
    network.EnableInputBalencing = true;
    network.EnableInhibitionPlasticity = true;
    //network.InputBalencingType = additive;
    //network.EnableLTD = false;
    //network.EnableHighFrequencyWeightPrinting = true;
    // modify sensitivity for output layer
    //network.setCellParameters(layer, cellParamNoiseSig, cellParamNoiseAl);
    //network.write_network("weightsBeforeBump");
    //network.scaleWeights(layer, noiseWeightScaleFactor);
    //network.write_network("weightsAfterBump");

  }
  if(durationCompleted  <= 1) 
  {
    network.logSpiking = true;
  }
  if(durationCompleted == 10) 
  {
    network.logSpiking = false;
  }
  //if(durationCompleted == 500) 
  //{
  //  network.EnableHighFrequencyWeightPrinting = false;
  //}
  //if(durationRemaining == 1){
  //// scale weights donwn and freeze network so no noise is applied for nonscaled weights
  //// this should be changed for real implementation but it is fine for preliminary testing. it will juest waste 1 aeon

  //  network.ApplyPlasticity = false;
  //  network.scaleWeights(layer, 1/noiseWeightScaleFactor);
  //}
}

void StageInterleavedSleepTrain2::epochHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(applyNoiseFlag) // if first time begining this stage
  {
    network.SpikeNeuronsByAverageBetweenToSpikeTimesArrays(network.spikeCountsTrain1, network.spikeCountsTrain2, 1);
  }
}

void StageInterleavedSleepTrain1::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  // train two
  if(durationCompleted % 2 == 0) // if first time begining this stage
  {
    SetNetEnvTrainTask1(environment, network);
    applyNoiseFlag = false;
  }
  else{ // noise averaging 1 and 2
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTrain1);
    applyNoiseFlag = true;
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    //network.logStdp = true;
    //network.logAppliedStdp = true;
    network.EnableSpikeRateAdaptation = true;
    network.EnableInputBalencing = true;
    network.EnableInhibitionPlasticity = true;
    //network.InputBalencingType = additive;
    //network.EnableLTD = false;
    //network.EnableHighFrequencyWeightPrinting = true;
    // modify sensitivity for output layer
    //network.setCellParameters(layer, cellParamNoiseSig, cellParamNoiseAl);
    //network.write_network("weightsBeforeBump");
    //network.scaleWeights(layer, noiseWeightScaleFactor);
    //network.write_network("weightsAfterBump");

  }
  if(durationCompleted  <= 1) 
  {
    network.logSpiking = true;
  }
  if(durationCompleted == 10) 
  {
    network.logSpiking = false;
  }
  //if(durationCompleted == 500) 
  //{
  //  network.EnableHighFrequencyWeightPrinting = false;
  //}
  //if(durationRemaining == 1){
  //// scale weights donwn and freeze network so no noise is applied for nonscaled weights
  //// this should be changed for real implementation but it is fine for preliminary testing. it will juest waste 1 aeon

  //  network.ApplyPlasticity = false;
  //  network.scaleWeights(layer, 1/noiseWeightScaleFactor);
  //}
}

void StageInterleavedSleepTrain1::epochHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(applyNoiseFlag) // if first time begining this stage
  {
    network.SpikeNeuronsByAverageBetweenToSpikeTimesArrays(network.spikeCountsTrain1, network.spikeCountsTrain2, 1);
  }
}

void StageInterleavedSleepUniformNoiseTrain1::aeonHandler(TimeStructure & Time, Network & network, Environment & environment){
  // train two
  if(durationCompleted % 2 == 0) // if first time begining this stage
  {
    SetNetEnvTrainTask1(environment, network);
    applyNoiseFlag = false;
  }
  else{ // noise averaging 1 and 2
    network.copySpikeCountsToStageSpikeCounts(network.spikeCountsTrain1);
    applyNoiseFlag = true;
    SetNetEnvDefaults(environment, network);
    environment.clearEnvironment();
    environment.applyConstantReward = 1;
    //network.logStdp = true;
    //network.logAppliedStdp = true;
    network.EnableSpikeRateAdaptation = true;
    network.EnableInputBalencing = true;
    network.EnableInhibitionPlasticity = true;
    //network.InputBalencingType = additive;
    //network.EnableLTD = false;
    //network.EnableHighFrequencyWeightPrinting = true;
    // modify sensitivity for output layer
    //network.setCellParameters(layer, cellParamNoiseSig, cellParamNoiseAl);
    //network.write_network("weightsBeforeBump");
    //network.scaleWeights(layer, noiseWeightScaleFactor);
    //network.write_network("weightsAfterBump");

  }
  if(durationCompleted  <= 1) 
  {
    network.logSpiking = true;
  }
  if(durationCompleted == 10) 
  {
    network.logSpiking = false;
  }
  //if(durationCompleted == 500) 
  //{
  //  network.EnableHighFrequencyWeightPrinting = false;
  //}
  //if(durationRemaining == 1){
  //// scale weights donwn and freeze network so no noise is applied for nonscaled weights
  //// this should be changed for real implementation but it is fine for preliminary testing. it will juest waste 1 aeon

  //  network.ApplyPlasticity = false;
  //  network.scaleWeights(layer, 1/noiseWeightScaleFactor);
  //}
}

void StageInterleavedSleepUniformNoiseTrain1::epochHandler(TimeStructure & Time, Network & network, Environment & environment){
  if(applyNoiseFlag) // if first time begining this stage
  {
    network.SpikeNeuronsByUniformAverageBetweenToSpikeTimesArrays(network.spikeCountsTrain1, network.spikeCountsTrain2, 1);
  }
}
