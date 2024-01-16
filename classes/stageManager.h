#ifndef STAGEMANAGER_H
#define STAGEMANAGER_H

#include <assert.h>
#include <climits>
#include <sstream>
#include <string>
#include <string>
#include <vector>
#include <cstdlib>

#include "environment.h"
#include "network.h"
#include "outputLogService.h"
#include "timeStructure.h"

using namespace std;

//food values
extern int h_init;
extern int v_init;
extern int dpos_init;
extern int dneg_init;
extern int h_swt;
extern int v_swt;
extern int dpos_swt;
extern int dneg_swt;
extern int trainingSpikeRateAdaptation;
extern int trainingInputBalencing;
extern double percentWeightIncrease;
extern double thresholdWeightIncrease;
extern double train1TillPerformanceThreshold;
extern double train2TillPerformanceThreshold;
extern double poissonEpochInputLayerThreshold;
extern double poissonStepInputLayerThreshold;
extern double pubPrivPrivateStepInputLayerThreshold;
extern double pubPivPublicStepInputLayerThreshold;
extern double cellParamNoiseAl;
extern double cellParamNoiseSig;
extern double noiseWeightScaleFactor;

enum FoodType{none, horizontal, vertical, positive, negative};

enum StageType{undefined, unsupervised, train1, train2, test1, test2, test1and2, interleaved, freezeInputHiddenWeights, freezeTopHiddenOutputWeights, bump, train1TillPerformance, train2TillPerformance, poissonEpochSpikingInputLayer, poissonStepSpikingInputLayer, task1FoodNoise, pubPrivNoiseByStep, poissonByNeuron, interleavedSleepTrain2, interleavedSleepTrain1, interleavedSleepUniformNoiseTrain1};

inline StageType stringToStageTypeEnum(string stringType){
  if(stringType == "unsupervised") 
    return unsupervised;
  else if(stringType == "train1") 
    return train1;
  else if(stringType == "train2") 
    return train2;
  else if(stringType == "test1") 
    return test1;
  else if(stringType == "test2") 
    return test2;
  else if(stringType == "test1and2") 
    return test1and2;
  else if(stringType == "interleaved") 
    return interleaved;
  else if(stringType == "freezeInputHiddenWeights") 
    return freezeInputHiddenWeights;
  else if(stringType == "freezeTopHiddenOutputWeights") 
    return freezeTopHiddenOutputWeights;
  else if(stringType == "train1TillPerformance") 
    return train1TillPerformance;
  else if(stringType == "train2TillPerformance") 
    return train2TillPerformance;
  else if(stringType == "bump") 
    return bump;
  else if(stringType == "poissonEpochSpikingInputLayer") 
    return poissonEpochSpikingInputLayer;
  else if(stringType == "poissonStepSpikingInputLayer") 
    return poissonStepSpikingInputLayer;
  else if(stringType == "task1FoodNoise") 
    return task1FoodNoise;
  else if(stringType == "pubPrivNoiseByStep") 
    return pubPrivNoiseByStep;
  else if(stringType == "poissonByNeuron") 
    return poissonByNeuron;
  else if(stringType == "interleavedSleepTrain2") 
    return interleavedSleepTrain2;
  else if(stringType == "interleavedSleepTrain1") 
    return interleavedSleepTrain1;
  else if(stringType == "interleavedSleepUniformNoiseTrain1") 
    return interleavedSleepUniformNoiseTrain1;
  else
    return undefined;
}

inline string typeEnumToStageString(StageType type){
  if(type == unsupervised) 
    return "unsupervised";
  else if(type == train1) 
    return "train1";
  else if(type == train2) 
    return "train2";
  else if(type == test1) 
    return "test1";
  else if(type == test2) 
    return "test2";
  else if(type == test1and2) 
    return "test1and2";
  else if(type == interleaved) 
    return "interleaved";
  else if(type == freezeInputHiddenWeights) 
    return "freezeInputHiddenWeights";
  else if(type == freezeTopHiddenOutputWeights) 
    return "freezeTopHiddenOutputWeights";
  else if(type == train1TillPerformance) 
    return "train1TillPerformance";
  else if(type == train2TillPerformance) 
    return "train2TillPerformance";
  else if(type == bump) 
    return "bump";
  else if(type == poissonEpochSpikingInputLayer) 
    return "poissonEpochSpikingInputLayer";
  else if(type == poissonStepSpikingInputLayer) 
    return "poissonStepSpikingInputLayer";
  else if(type == task1FoodNoise) 
    return "task1FoodNoise";
  else if(type == pubPrivNoiseByStep) 
    return "pubPrivNoiseByStep";
  else if(type == poissonByNeuron) 
    return "poissonByNeuron";
  else if(type == interleavedSleepTrain2) 
    return "interleavedSleepTrain2";
  else if(type == interleavedSleepTrain1) 
    return "interleavedSleepTrain1";
  else if(type == interleavedSleepUniformNoiseTrain1) 
    return "interleavedSleepUniformNoiseTrain1";
  else
    return "invalid Enum";
}

// Base class for all stages
// currently dont need to worry about "c++ big 3" becasue all ariable s should be fine to 
// pass by value
// !!! USE POINTERS FOR ALL STAGES - this is the only way the virtual functions will work 
// properly in the StageManager lists 
class Stage{
public:
  StageType stageType; // type of stage

  //these following two members are implemented to manage stages of variable length
  time_type durationRemaining; // duration remaining in aeons
  time_type durationCompleted; // duration completed in aeons

  Stage(StageType st,time_type durRemaining){
    stageType = st; 
    durationRemaining = durRemaining;
    durationCompleted = 0;
  }
  virtual ~Stage() {}
  
  // every stage should override these if the default value is wrong
  virtual inline bool isInstantaneousStage(){ return false; } 
  virtual inline bool isUndefiniteStage(){ return false; } 
  virtual void stageCleanUp(TimeStructure & Time, Network & network, Environment & environment){}

  // every stage should override these as needed
  virtual void aeonHandler(TimeStructure & Time, Network & network, Environment & environment){}
  virtual void epochHandler(TimeStructure & Time, Network & network, Environment & environment){}
  // stepHandler is inlined to increase performance. It is called every iteration so inlining should save time.
  virtual inline void stepHandler(TimeStructure & Time, Network & network, Environment & environment){}
};

class StageUndefined : public Stage{
  public:
    StageUndefined(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
};

class StageUnsupervised : public Stage{
  public:
    StageUnsupervised(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StageTrain1 : public Stage{
  public:
    StageTrain1(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void stageCleanUp(TimeStructure & Time, Network & network, Environment & environment);
};

class StageTrain2 : public Stage{
  public:
    StageTrain2(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void stageCleanUp(TimeStructure & Time, Network & network, Environment & environment);
};

class StageTest1 : public Stage{
  public:
    StageTest1(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void stageCleanUp(TimeStructure & Time, Network & network, Environment & environment);
};

class StageTest2 : public Stage{
  public:
    StageTest2(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void stageCleanUp(TimeStructure & Time, Network & network, Environment & environment);
};

class StageTest1and2 : public Stage{
  public:
    StageTest1and2(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StageInterleaved : public Stage{
  public:
    StageInterleaved(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StageFreezeInputHiddenWeights : public Stage{
  public:
    inline bool isInstantaneousStage(){ return true; } 
    StageFreezeInputHiddenWeights(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StageFreezeTopHiddenOutputWeights : public Stage{
  public:
    inline bool isInstantaneousStage(){ return true; } 
    StageFreezeTopHiddenOutputWeights(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StageBump : public Stage{
  public:
    inline bool isInstantaneousStage(){ return true; } 
    StageBump(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StageTrain1TillPerformance : public Stage{
  public:
    inline bool isUndefiniteStage(){ return true; } 
    StageTrain1TillPerformance(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void stageCleanUp(TimeStructure & Time, Network & network, Environment & environment);
};

class StageTrain2TillPerformance : public Stage{
  public:
    inline bool isUndefiniteStage(){ return true; } 
    StageTrain2TillPerformance(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void stageCleanUp(TimeStructure & Time, Network & network, Environment & environment);
};

class StagePoissonEpochSpikingInputLayer : public Stage{
  public:
    StagePoissonEpochSpikingInputLayer(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void epochHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StagePoissonStepSpikingInputLayer : public Stage{
  public:
    StagePoissonStepSpikingInputLayer(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    inline void stepHandler(TimeStructure &Time, Network &network, Environment &environment){
      time_type timeInEpoch = Time.ctime % Time.epoch;
      if(timeInEpoch <= 50)
        network.SpikeNeuronsInLayerByPoisson(0, poissonStepInputLayerThreshold);
    }
};

class StageTask1FoodNoise : public Stage{
  public:
    StageTask1FoodNoise(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void epochHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StagePubPrivNoiseByStep : public Stage{
  public:
    StagePubPrivNoiseByStep(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    inline void stepHandler(TimeStructure &Time, Network &network, Environment &environment){
      time_type timeInEpoch = Time.ctime % Time.epoch;
      if (timeInEpoch <= 50)
        network.SpikeNeuronsInLayerByPubPrivPoisson(0, pubPivPublicStepInputLayerThreshold, pubPrivPrivateStepInputLayerThreshold);
    }
};

class StagePoissonByNeuron : public Stage{
  public:
    StagePoissonByNeuron(StageType st, time_type durRemaining) : Stage(st, durRemaining){}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void epochHandler(TimeStructure & Time, Network & network, Environment & environment);
};

class StageInterleavedSleepTrain2 : public Stage{
  public:
    StageInterleavedSleepTrain2(StageType st, time_type durRemaining) : Stage(st, durRemaining){applyNoiseFlag = false;}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void epochHandler(TimeStructure & Time, Network & network, Environment & environment);
    bool applyNoiseFlag;
};

class StageInterleavedSleepTrain1 : public Stage{
  public:
    StageInterleavedSleepTrain1(StageType st, time_type durRemaining) : Stage(st, durRemaining){applyNoiseFlag = false;}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void epochHandler(TimeStructure & Time, Network & network, Environment & environment);
    bool applyNoiseFlag;
};

class StageInterleavedSleepUniformNoiseTrain1 : public Stage{
  public:
    StageInterleavedSleepUniformNoiseTrain1(StageType st, time_type durRemaining) : Stage(st, durRemaining){applyNoiseFlag = false;}
    void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
    void epochHandler(TimeStructure & Time, Network & network, Environment & environment);
    bool applyNoiseFlag;
};

// here it is fine to return the Stage by value since all members of Stage
// are fine to pass by value
inline Stage * typeEnumToStageObject(StageType type, time_type durRemaining){
  if(type == unsupervised) 
    return new StageUnsupervised(type, durRemaining);
  else if(type == train1) 
    return new StageTrain1(type, durRemaining);
  else if(type == train2) 
    return new StageTrain2(type, durRemaining);
  else if(type == test1) 
    return new StageTest1(type, durRemaining);
  else if(type == test2) 
    return new StageTest2(type, durRemaining);
  else if(type == test1and2) 
    return new StageTest1and2(type, durRemaining);
  else if(type == interleaved) 
    return new StageInterleaved(type, durRemaining);
  else if(type == freezeInputHiddenWeights) 
    return new StageFreezeInputHiddenWeights(type, durRemaining);
  else if(type == freezeTopHiddenOutputWeights) 
    return new StageFreezeTopHiddenOutputWeights(type, durRemaining);
  else if(type == train1TillPerformance) 
    return new StageTrain1TillPerformance(type, durRemaining);
  else if(type == train2TillPerformance) 
    return new StageTrain2TillPerformance(type, durRemaining);
  else if(type == bump) 
    return new StageBump(type, durRemaining);
  else if(type == poissonEpochSpikingInputLayer) 
    return new StagePoissonEpochSpikingInputLayer(type, durRemaining);
  else if(type == poissonStepSpikingInputLayer) 
    return new StagePoissonStepSpikingInputLayer(type, durRemaining);
  else if(type == task1FoodNoise) 
    return new StageTask1FoodNoise(type, durRemaining);
  else if(type == pubPrivNoiseByStep) 
    return new StagePubPrivNoiseByStep(type, durRemaining);
  else if(type == poissonByNeuron) 
    return new StagePoissonByNeuron(type, durRemaining);
  else if(type == interleavedSleepTrain2) 
    return new StageInterleavedSleepTrain2(type, durRemaining);
  else if(type == interleavedSleepTrain1) 
    return new StageInterleavedSleepTrain1(type, durRemaining);
  else if(type == interleavedSleepUniformNoiseTrain1) 
    return new StageInterleavedSleepUniformNoiseTrain1(type, durRemaining);
  else
    return new StageUndefined(type, durRemaining);
}

class StageManager
{
public: 
  StageManager(OutputLogService &logService);
  void initStages(string filePath);
  //used to handle aeon level events like setting current stage
  void aeonHandler(TimeStructure & Time, Network & network, Environment & environment);
  //used to handle epoch level events 
  void epochHandler(TimeStructure & Time, Network & network, Environment & environment);
  //used to handle step level events 
  void stepHandler(TimeStructure & Time, Network & network, Environment & environment);
  void printCompletedStages();
  void printRemainingStages();

  inline bool continueSimulation(){return continueSim;}

  inline string getCurrentStageType(){
    if(stagesRemaining.size() == 0)
      return "No stages remaining";
    Stage * temp = stagesRemaining.back();
    return typeEnumToStageString(temp->stageType);

  }

  ~StageManager() {
    for(unsigned int i = 0; i < stagesRemaining.size(); i++){
      delete stagesRemaining[i];
    }
    for(unsigned int i = 0; i < stagesCompleted.size(); i++){
      delete stagesCompleted[i];
    }
  }

private:
  OutputLogService &LogService;
  vector<Stage*> stagesRemaining; //holds stages to be completed in reverse order
  vector<Stage*> stagesCompleted;
  Stage * curStage;
  bool continueSim;

  //all stages
  void SetNetEnvUnsupervised(Environment &environment, Network &network);
  void FreezeHiddelLayerWeights(Environment &environment, Network &network);
  void SetNetEnvTrainTask1(Environment &environment, Network &network);
  void SetNetEnvTrainTask2(Environment &environment, Network &network);
  void SetNetEnvTestTask1(Environment & environment, Network &network);
  void SetNetEnvTestTask2(Environment & environment, Network &network);
  void task1FoodNoiseEpoch(Environment & environment, Network &network);
}; //StageManager

// Helpers

// returns food type of first food that matches type
inline FoodType getFoodTypeHelper(int h, int v, int p, int n, int value){
  FoodType returnFood = none;
  if(h == value)
    returnFood = horizontal; 
  else if(v == value)
    returnFood = vertical;
  else if(p == value)
    returnFood = positive;
  else if(n == value)
    returnFood = negative;
  return returnFood;
}

inline void getSecondPixelCoord(int & x, int & y, FoodType foodType){

  switch(foodType) {

    case horizontal: 
      x = 1;
      y = 0;
      break;

    case vertical: 
      x = 0;
      y = 1;
      break;

    case positive: 
      x = 1;
      y = 1;
      break;

    case negative: 
      x = 1;
      y = -1;
      break;

    default: 
      break;
  }
}

#endif // STAGEMANAGER_H
