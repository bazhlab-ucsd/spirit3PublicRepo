#include "critter.h"

using namespace std;

Critter::Critter()
{
  happy = 0;
  happy_d = 0.00001;
  judgement = 0.5;
  judgement_d = 0.0001;
  hunger = 0;
  direction.x = 1;
  direction.y = 1;
  p_random_mvmt = 0.005;
  instantaneousFoodAverageIdx = 0;
  clearInstantaneousFoodAverageArray();
} //constructor

void Critter::clearInstantaneousFoodAverageArray(){
  for(unsigned int i = 0; i < numberInAverageFood; i++){
    instantaneousFoodAverageArray[i] = 0.5; //start instantaneous average at 50%
  }
}

//food = 1 | rewarded food  
//food = 0.5 | task irrelevant food
//food = 0 | punished food
void Critter::addFoodToInstantaneousAverage(double food){
  instantaneousFoodAverageArray[(instantaneousFoodAverageIdx % numberInAverageFood)] = food;
  instantaneousFoodAverageIdx++;
}

//we want this average to reflect judgment
double Critter::getInstantaneousFoodAverage(){
  double total = 0;
  for(unsigned int i = 0; i < numberInAverageFood; i++){
    total += instantaneousFoodAverageArray[i];
  }
  return total / ((double)numberInAverageFood);
}
