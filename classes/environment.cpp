#include "environment.h"
#include <fstream>
#include <stdlib.h>
#include <assert.h>

void Environment::wrap(int &x, int &y)
{ // enfores Toroidal boundary conditions on the Environment
	if (x > size - 1)
		x = x - size;
	if (x < 0)
		x = x + size;
	if (y > size - 1)
		y = y - size;
	if (y < 0)
		y = y + size;
}

void Environment::flash_cells(Network &network)
{
	int x, y, i, j, in;
	in = network.input_size; //optimally should be network.input_size
	for (x = 0; x < in; x++)
	{
		for (y = 0; y < in; y++)
		{
			i = critter.position.x + (x - ((in - 1) / 2));
			j = critter.position.y + (y - ((in - 1) / 2));
			wrap(i, j);
			if (env[i][j] != 0)
			{
				network.input_layer[x][y].flash = 1;
			}
			if (env[i][j] == 0)
			{
				network.input_layer[x][y].flash = 0;
			}
		}
	}
	for (x = 0; x < network.input_size; x++)
	{
		for (y = 0; y < network.input_size; y++)
		{
			if (network.get_cell(x, y, 0).flash)
			{
				network.get_cell(x, y, 0).input += 0.025;
				//cout<<get_cell(x,y,0).flash<<"\n";
			}
		}
	}

} //flash_cell

int Environment::check_emptiness(int x, int y)
{ //helper routine - are all spot around x,y in environment nonfood?
	if (x > 0)
		if (env[x - 1][y])
			return 0;
	if (x < size - 1)
		if (env[x + 1][y])
			return 0;
	if (y > 0)
		if (env[x][y - 1])
			return 0;
	if (y < size - 1)
		if (env[x][y + 1])
			return 0;
	if (x > 0 && y > 0)
		if (env[x - 1][y - 1])
			return 0;
	if (x > 0 && y < size - 1)
		if (env[x - 1][y + 1])
			return 0; // ensure pairs don't share a grid vertex
	if (x < size - 1 && y > 0)
		if (env[x + 1][y - 1])
			return 0;
	if (x < size - 1 && y < size - 1)
		if (env[x + 1][y + 1])
			return 0;
	return 1;
} //check_emptiness

void Environment::switch_env_state(int mode, int h, int v, int p, int n)
{
	env_state[0] = mode;
	env_state[1] = h;
	env_state[2] = v;
	env_state[3] = p;
	env_state[4] = n;

	createEnvironment();
} //switch_env_state

void Environment::move_critter(Network &network)
{
	int i, j;
	int x, y;
	//against infinite loops, which can happen even if spirit correctly learned
	if (rand() % 100 <= (critter.p_random_mvmt * 100 * critter.hunger) - 1)
	{
		critter.direction.x = (rand() % 3) - 1;
		critter.direction.y = (rand() % 3) - 1;
		//cout<<"random move "<<critter.direction.x<<critter.direction.y<<"\n";
	}
	critter.position.x += critter.direction.x;
	critter.position.y += critter.direction.y;
	wrap(critter.position.x, critter.position.y);

	//cout<< critter.position.x << critter.position.y <<'\n';
	int food = 0;
  double reward = 0;
	int in_pattern = 0;
	int food_eat = 0;
	int food_place = 0;
	// Check for food
  
  // if in a noisy replay state and a constant reward should be applied then
  if(applyConstantReward == 1){
		reward = 0.5;
		network.apply_STDP_traces(reward);
  }
	else if ((food = env[x = critter.position.x][y = critter.position.y]) != 0)
	{
		env[x][y] = 0;
		// Case 1: Food is horiz
		if (food >= horiz && food < vert)
		{
			if (env[(x + 1) % size][y] == food)
				env[(x + 1) % size][y] = 0;
			if (x > 0 && env[x - 1][y] == food)
				env[x - 1][y] = 0;
			in_pattern = 1;
			food_eat = 1;
			while (!starving)
			{
				x = rand() % size;
				y = rand() % size;
				if (env[x][y] == 0 && x + 1 < size && env[x + 1][y] == 0)
				{
					if (!(check_emptiness(x, y) && check_emptiness(x + 1, y)))
						continue;
					env[x][y] = env[x + 1][y] = food;
					food_place = 1;
					break;
				}
			}
		} // Case 2: Food is vert
		else if (food >= vert && food < d_pos)
		{
			if (env[x][(y + 1) % size] == food)
				env[x][(y + 1) % size] = 0;
			if (y > 0 && env[x][y - 1] == food)
				env[x][y - 1] = 0;
			in_pattern = 2;
			food_eat = 1;
			while (!starving)
			{
				x = rand() % size;
				y = rand() % size;
				if (env[x][y] == 0 && y + 1 < size && env[x][y + 1] == 0)
				{
					if (!(check_emptiness(x, y) && check_emptiness(x, y + 1)))
						continue;
					env[x][y] = env[x][y + 1] = food;
					food_place = 1;
					break;
				}
			}
		} // Case 3: Food is d_pos
		else if (food >= d_pos && food < d_neg)
		{
			if (env[(x + 1) % size][(y + 1) % size] == food)
				env[(x + 1) % size][(y + 1) % size] = 0;
			if (x > 0 && y > 0 && env[x - 1][y - 1] == food)
				env[x - 1][y - 1] = 0;
			in_pattern = 3;
			food_eat = 1;
			while (!starving)
			{
				x = rand() % size;
				y = rand() % size;
				if (env[x][y] == 0 && x + 1 < size && y + 1 < size && env[x + 1][y + 1] == 0)
				{
					if (!(check_emptiness(x, y) && check_emptiness(x + 1, y + 1)))
						continue;
					env[x][y] = env[x + 1][y + 1] = food;
					food_place = 1;
					break;
				}
			}
		} // Case 4: Food is d_neg
		else if (food >= d_neg)
		{
			if (y > 0 && env[(x + 1) % size][y - 1] == food)
				env[(x + 1) % size][y - 1] = 0;
			if (x > 0 && env[x - 1][(y + 1) % size] == food)
				env[x - 1][(y + 1) % size] = 0;
			in_pattern = 4;
			food_eat = 1;
			while (!starving)
			{
				x = rand() % size;
				y = rand() % size;
				if (env[x][y] == 0 && x + 1 < size && y > 0 && env[x + 1][y - 1] == 0)
				{
					if (!(check_emptiness(x, y) && check_emptiness(x + 1, y - 1)))
						continue;
					env[x][y] = env[x + 1][y - 1] = food;
					food_place = 1;
					break;
				}
			}
		}
		else
		{
			cout << "error: found food without eating. Food =" << food << "\n";
		}
		if (food_place && !food_eat)
		{
			cout << "No food eaten, but placed food=" << food << "\n";
		}
		if (!food_place && food_eat)
		{
			cout << "No food placed, but eaten food=" << food << "\n";
		}

		// update metric functions and reward
    
    // Rewarded food particle eaten
		if (in_pattern != 0 && env_state[in_pattern] > 0)
		{
			critter.hunger = 0;
			reward = 1;
			critter.happy = critter.happy * (1 - critter.happy_d) + critter.happy_d;
			critter.judgement = critter.judgement * (1 - critter.judgement_d) + critter.judgement_d;
      critter.addFoodToInstantaneousAverage(1);
			network.apply_STDP_traces(reward);
			//cout<<"applied reward \n";
		}
    //Punished food particle eaten
		else if (in_pattern != 0 && env_state[in_pattern] < 0)
		{
			critter.hunger = 0;
      if(applyPunishment == 1)
        reward = -0.5;
      else if(applyPunishment ==0)
        reward = -0.001;
      else{
        cout <<  "invalid applyPunishment value of: " << applyPunishment << endl;
        assert(0);
      }
			critter.happy = critter.happy * (1 - critter.happy_d) - critter.happy_d;
			critter.judgement = critter.judgement * (1 - critter.judgement_d);
      critter.addFoodToInstantaneousAverage(0);
			network.apply_STDP_traces(reward);
			//cout<<"applied reward \n";
		}
    //NonTask food particle eaten
		else if (in_pattern != 0 && env_state[in_pattern] == 0)
		{
			critter.hunger++;
			critter.happy = critter.happy * (1 - critter.happy_d);
			//critter.judgement=critter.judgement*(1-critter.judgement_d);
      //critter.addFoodToInstantaneousAverage(0.5);
			network.apply_STDP_traces(punishment);
			//cout<<"applied punishment \n";
		}
		//log the food that was eaten and if that food was rewarded
		totalFoodEaten[in_pattern]++;
		LogService.GetLog(LogService.EatenFoodLogKey) 
		<< LogService.Time.ctime << " "  
		<< in_pattern << " " 
		<< env_state[in_pattern] << " " 
		<< totalFoodEaten[1] << " " 
		<< totalFoodEaten[2] << " " 
		<< totalFoodEaten[3] << " " 
		<< totalFoodEaten[4] << " "
		<< env_state[1] << " "
		<< env_state[2] << " "
		<< env_state[3] << " "
		<< env_state[4] << endl;
	}
	// No food -> hungry
	else
	{
		critter.happy = critter.happy * (1 - critter.happy_d);
		network.apply_STDP_traces(punishment);
		critter.hunger++;
		//cout<<"applied punishment \n";
	}
	// reset memory
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			critter.dirtotal[i][j] = 0;
		}
	}
} // move_critter

void Environment::clearEnvironment(){
  for (int j = 0; j < size; ++j)
    for (int i = 0; i < size; ++i)
      env[i][j] = 0;
}

void Environment::createEnvironment()
{
  clearEnvironment();
	// Case 1: All patterns present
	if (env_state[0] == 0)
	{
		for (int j = 0; j < size * size * 0.1 / 8; ++j)
		{ //2.5% horiz
			int x = rand() % size;
			int y = rand() % size;
			if (env[x][y] == 0 && x + 1 < size && env[x + 1][y] == 0 && check_emptiness(x, y) && check_emptiness(x + 1, y))
				env[x][y] = env[x + 1][y] = horiz + j;
			else
				j--;
		}
		for (int j = 0; j < size * size * 0.1 / 8; ++j)
		{ //2.5% vert
			int x = rand() % size;
			int y = rand() % size;
			if (env[x][y] == 0 && y + 1 < size && env[x][y + 1] == 0 && check_emptiness(x, y) && check_emptiness(x, y + 1))
				env[x][y] = env[x][y + 1] = vert + j;
			else
				j--;
		}
		for (int j = 0; j < size * size * 0.1 / 8; ++j)
		{ //2.5% d_pos
			int x = rand() % size;
			int y = rand() % size;
			if (env[x][y] == 0 && x + 1 < size && y + 1 < size && env[x + 1][y + 1] == 0 && check_emptiness(x, y) && check_emptiness(x + 1, y + 1))
				env[x][y] = env[x + 1][y + 1] = d_pos + j;
			else
				j--;
		}
		for (int j = 0; j < size * size * 0.1 / 8; ++j)
		{ //2.5% d_neg
			int x = rand() % size;
			int y = rand() % size;
			if (env[x][y] == 0 && x + 1 < size && y > 0 && env[x + 1][y - 1] == 0 && check_emptiness(x, y) && check_emptiness(x + 1, y - 1))
				env[x][y] = env[x + 1][y - 1] = d_neg + j;
			else
				j--;
		}
	}
	//Case 2: Only task relevant patterns presented
	else if (env_state[0] == 1)
	{
		// horiz
		if (env_state[1] != 0)
		{
			for (int j = 0; j < size * size * 0.1 / 4; ++j)
			{ //5%
				int x = rand() % size;
				int y = rand() % size;
				if (env[x][y] == 0 && x + 1 < size && env[x + 1][y] == 0 && check_emptiness(x, y) && check_emptiness(x + 1, y))
					env[x][y] = env[x + 1][y] = horiz + j;
				else
					j--;
			}
		}
		// vert
		if (env_state[2] != 0)
		{
			for (int j = 0; j < size * size * 0.1 / 4; ++j)
			{ //5%
				int x = rand() % size;
				int y = rand() % size;
				if (env[x][y] == 0 && y + 1 < size && env[x][y + 1] == 0 && check_emptiness(x, y) && check_emptiness(x, y + 1))
					env[x][y] = env[x][y + 1] = vert + j;
				else
					j--;
			}
		}
		// d_pos
		if (env_state[3] != 0)
		{
			for (int j = 0; j < size * size * 0.1 / 4; ++j)
			{ //5%
				int x = rand() % size;
				int y = rand() % size;
				if (env[x][y] == 0 && x + 1 < size && y + 1 < size && env[x + 1][y + 1] == 0 && check_emptiness(x, y) && check_emptiness(x + 1, y + 1))
					env[x][y] = env[x + 1][y + 1] = d_pos + j;
				else
					j--;
			}
		}
		// d_neg
		if (env_state[4] != 0)
		{
			for (int j = 0; j < size * size * 0.1 / 4; ++j)
			{ //5%
				int x = rand() % size;
				int y = rand() % size;
				if (env[x][y] == 0 && x + 1 < size && y > 0 && env[x + 1][y - 1] == 0 && check_emptiness(x, y) && check_emptiness(x + 1, y - 1))
					env[x][y] = env[x + 1][y - 1] = d_neg + j;
				else
					j--;
			}
		}
	}

} //createEnvironment

int Environment::getFoodAmt()
{
	int amt = 0;
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			if (env[i][j] != 0)
				++amt;
		}
	}
	return amt;
} //getFoodAmt

int Environment::foodAmtConstant(int initAmt)
{
	int currAmt = 0;
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			if (env[i][j] != 0)
				++currAmt;
		}
	}
	if (initAmt == currAmt)
	{
		return 1;
	}
	if (initAmt > currAmt)
	{
		cout << "Food was eaten but not replaced: initAmt=" << initAmt << ", currAmt=" << currAmt << "\n";
		return 0;
	}
	cout << "Food was placed but none was eaten: initAmt=" << initAmt << ", currAmt=" << currAmt << "\n";
	return 0;
} //foodAmtConstant

//read environment file
void Environment::loadEnvironment(const string fn)
{
	ifstream myfile(fn.c_str()); //environment file name
	for (int j = 0; j < size; ++j)
		for (int i = 0; i < size; ++i)
		{
			assert(myfile.good());
			myfile >> env[i][j];
			if (env[i][j] == 2)
			{
				critter.position.x = i;
				critter.position.y = j;
				env[i][j] = 0;
			}
		}
	myfile.close();
} //loadEnvironment

//print to screen
void Environment::printEnvironment() { printEnvironment(string()); }
//rewrite environment file
void Environment::printEnvironment(const string output_file)
{
	ofstream outfileext;
	if (!output_file.empty())
		outfileext.open(output_file.c_str());

	//decide whether we flush to file or screen
	ostream &outfile = output_file.empty() ? cout : outfileext;

	env[critter.position.x][critter.position.y] = 2;
	for (int j = 0; j < size; ++j)
	{
		for (int i = 0; i < size; ++i)
		{
			outfile << env[i][j];
			outfile << ' ';
		}
		outfile << '\n';
	}
	env[critter.position.x][critter.position.y] = 0;

	if (!output_file.empty())
		outfileext.close();
} //printEnvironemt

Environment::Environment(OutputLogService& logServie) : LogService(logServie), starving(false) 
{ 
  applyConstantReward = 0;
  applyPunishment = 0;
	createEnvironment(); 
	for(int i = 0; i < 5; i++){
		totalFoodEaten[i] = 0;
	}
} //for gui cache the init will rewrite create routine results
Environment::Environment(const string e,OutputLogService& logServie) : LogService(logServie), starving(false)
{
  applyConstantReward = 0;
  applyPunishment = 0;
	loadEnvironment(e);
	for(int i = 0; i < 5; i++){
		totalFoodEaten[i] = 0;
	}
}

Environment::Environment(int mode, int h, int v, int d_pos, int d_neg,OutputLogService& logServie) : LogService(logServie), starving(false)
{
  applyConstantReward = 0;
  applyPunishment = 0;
	switch_env_state(mode, h, v, d_pos, d_neg);
	for(int i = 0; i < 5; i++){
		totalFoodEaten[i] = 0;
	}
}

