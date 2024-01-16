#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "network.h"
#include "critter.h"
#include <string>
#include <math.h>
#include "outputLogService.h"

using namespace std;

class Environment
{
private:
	OutputLogService &LogService;
public:
	static const int size = 50; // size of Environment grid

	static const int horiz = 10;	 // food particles are indexed via increasing numbers,
	static const int vert = 1000;	// these constants are used as shifts to distinguish between
	static const int d_pos = 2000; // rewarded and punished patterns
	static const int d_neg = 3000;

  int applyPunishment;
  int applyConstantReward;
	int env[size][size]; // grid coordinate structure projected onto Environment
	int env_state[5];		 // {mode, val_horiz, val_vert, val_pos, val_neg};
	long long int totalFoodEaten[5];
	Critter critter;		 // agent placed in the Environment
	bool starving;			 // if TRUE - starves the Critter by not replacing eaten food

	void wrap(int &x, int &y);
	void flash_cells(Network &network);
	int check_emptiness(int x, int y); //helper routine - are all spot around x,y in environment nonfood?
	void switch_env_state(int mode, int h, int v, int p, int n);
	void move_critter(Network &network);
  void clearEnvironment();
	void createEnvironment();
	int getFoodAmt();
	int foodAmtConstant(int initAmt);
	//read environment file
	void loadEnvironment(const string fn);

	//print to screen
	void printEnvironment();
	//rewrite environment file
	void printEnvironment(const string output_file);

	Environment(OutputLogService& logServie); //for gui cache the init will rewrite create routine results
	Environment(const string e, OutputLogService& logServie);

	Environment(int mode, int h, int v, int d_pos, int d_neg, OutputLogService& logServie);
}; //Environment

#endif // ENVIRONMENT_H
