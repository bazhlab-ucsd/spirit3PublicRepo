#ifndef CRITTER_H
#define CRITTER_H

#include <iostream>
#include "coord.h"

class Critter
{
private:
  int instantaneousFoodAverageIdx;
  static const int numberInAverageFood = 100;
  double instantaneousFoodAverageArray[numberInAverageFood];

public:
  int dirtotal[3][3]; //count of cells fired in this epoch (we need it to figure out where to go)
  Coord position;     //position on map
  int hunger;         //number of consecutive unrewarded moves
  Coord direction;    // direction moved last epoch
  double happy;       //success rate, exponential moving average
  double happy_d;     //rate of change of happiness
  double judgement;   //correct vs incorrect food choices (not including no food movements)
  double judgement_d;
  double p_random_mvmt; //probabilty of random movememt

  double getInstantaneousFoodAverage();
  void addFoodToInstantaneousAverage(double food);
  void clearInstantaneousFoodAverageArray();
  Critter();
}; //Critter

#endif // CRITTER_H
