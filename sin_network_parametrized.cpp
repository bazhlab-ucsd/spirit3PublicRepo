#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include "params.h"
using namespace std;

#define I 7
#define M 28
#define O 9
const int layers = 3;
const int layer_size[layers] = {I, M, O};

#define sqr(x) ((x) * (x))
//connectivities
int c_m = 9; //number of incoming connections for each neuron in middle layer
const int n_m = sqr(M);
int c_i; //number of output connections from input layer single neuron
static const int n_i = sqr(I);
int e_i[sqr(I)];                //total number of output connections from input layer
bool in2mid[sqr(I)][sqr(M)];    //connectivity matrix
double in2midS[sqr(I)][sqr(M)]; //strength values
int n_o = sqr(layer_size[2]);

int middle_fan_in = c_m;
int r_direction_i = 3;
int r_direction_j = 3;
int findmin()
{
  int min = e_i[0];
  ;
  for (int i = 0; i < sqr(layer_size[0]); i++)
    if (min > e_i[i])
      min = e_i[i];
  return min;
}

void create_network(const string output_file)
{
  ofstream outfileext;
  if (!output_file.empty())
    outfileext.open(output_file.c_str());
  //decide whether we flush to file or screen
  ostream &outfile = output_file.empty() ? cout : outfileext;

  assert((c_m * n_m) % n_i == 0); //uncompatible network size and connectivity
  c_i = c_m * n_m / n_i;

  //create connectivity
  for (int m = 0; m < n_m; m++) //all middle layer neurons
    for (int e = 0; e < c_m; e++)
    {                       //all edges
      int x = rand() % n_i; //pick random input cell
      if (findmin() != e_i[x])
      {
        e--;
        continue;
      } //if we don't fill up space from the lowest we can easily get to the situation where there is no solution without using the same connection twice
      if (in2mid[x][m])
      {
        e--;
        continue;
      } //check it's not used yet
      if (e_i[x] >= c_i)
      {
        e--;
        continue;
      } //each 1st layer neuron needs to have the same number of output connections
      in2mid[x][m] = true;
      e_i[x]++;
    } //e

  //generate strengths, based on sin_network_random
  {
    //method to distribute strength
    //1. pick random connection and add chunk
    //2. do 1. ocoins times
    double chunk = 0.001;

    int ocoins = 10 * M * M * ((double)middle_fan_in / (I * I));
    //decide whether we flush to file or screen
    int y, x, i, j, coins, xx, yy;

    //init
    for (int i = 0; i < n_i; i++)
      for (int ii = 0; ii < n_m; ii++)
        if (in2mid[i][ii])
          in2midS[i][ii] = chunk;
    //distribute
    for (int i = 0; i < n_i; i++)
    {
      coins = ocoins;
      while (coins > 0)
        for (int ii = 0; ii < n_m && coins; ii++)
          if (in2mid[i][ii])
            if (((double)rand() / RAND_MAX) > .99)
            {
              in2midS[i][ii] += chunk;
              coins--;
            } //if
    }         //i
  }           //imported code with interfering variables

  //output values
  int z = 0;
  for (int i = 0; i < n_i; i++)
    for (int ii = 0; ii < n_m; ii++)
    {
      int y = i / I;
      int x = i % I;
      int II = ii / M;
      int J = ii % M;
      if (in2mid[i][ii])
      {
        outfile << z << ' ' << y << ' ' << x << ' ' << (z + 1) << ' ' << II << ' ' << J << ' ' << in2midS[i][ii] << "\n";
        outfile << z << ' ' << y << ' ' << x << ' ' << (z + 1) << ' ' << II << ' ' << J << ' ' << -in2midS[i][ii] << "\n";
      } //if
    }

  //This is for all to all connection
  // z = 1;
  // for (int i = 0; i < n_m; i++)
  //   for (int ii = 0; ii < n_o; ii++)
  //   {
  //     int y = i / layer_size[1];
  //     int x = i % layer_size[1];
  //     int II = ii / layer_size[2];
  //     int J = ii % layer_size[2];
  //     outfile << z << ' ' << y << ' ' << x << ' ' << (z + 1) << ' ' << II << ' ' << J << ' ' << 0.02 << "\n";
  //     outfile << z << ' ' << y << ' ' << x << ' ' << (z + 1) << ' ' << II << ' ' << J << ' ' << -0.02 << "\n";
  //   }

  // this is for sparse disjoint connectivity
  // for each group of neurons that dictates a direction, every nerunon in that cluster gets wired to a disjoint segment of the hidden layer
  // z = 1;
  // int r_direction = layer_size[2] / 3; // side of square that coresponds to output layer diretion
  // int hiddenNPerOuputN = n_m / sqr(r_direction); // split hidden layer evenly between number of output layer neurons in each output layer direction chunk
  // for(int i = 0; i < layer_size[2]; i++){
  //   for(int j = 0; j < layer_size[2]; j++){
  //     int directioni = i / r_direction;
  //     int directionj = j / r_direction;

  //     int reli = i - (directioni * r_direction); // relative i an j coors inside direction chunk
  //     int relj = j - (directionj * r_direction);

  //     int offset =(((relj * r_direction) + reli) * hiddenNPerOuputN);

  //     for(int k = offset; k < offset + hiddenNPerOuputN; k++){
  //       //unroll indexes
  //       int h_i = k / layer_size[1];
  //       int h_j = k % layer_size[1];
  //       outfile << z << ' ' << h_i << ' ' << h_j << ' ' << (z + 1) << ' ' << i << ' ' << j << ' ' << 0.02 << "\n";
  //       outfile << z << ' ' << h_i << ' ' << h_j << ' ' << (z + 1) << ' ' << i << ' ' << j << ' ' << -0.02 << "\n";

  //     }

  //   }
  // }

  // // this is for sparse random disjoint connectivity for square layers
  // int * idxs = new int[n_m];
  // for(int i = 0; i < n_m; i++)
  //   idxs[i] = i;

  //   z = 1;
  //   int r_direction = layer_size[2] / 3; // side of square that coresponds to output layer diretion
  //   int hiddenNPerOuputN = n_m / sqr(r_direction); // split hidden layer evenly between number of output layer neurons in each output layer direction chunk

  // //go direction by direction to ensure that every direction has a different shuffle of the array
  //   for (int directioni = 0; directioni < 3; directioni++)
  //   {
  //     for (int directionj = 0; directionj < 3; directionj++)
  //     {
  //       //each direction gets a shuffle of idxs
  //       random_shuffle(idxs, (idxs + n_m));
  //       //for each output layer cell in the direction
  //       for (int reli = 0; reli < r_direction; reli++)
  //       {
  //         for (int relj = 0; relj < r_direction; relj++)
  //         {
  //           //real output layer coords
  //           int i = reli + (directioni * r_direction);
  //           int j = relj + (directionj * r_direction);

  //           int offset =(((relj * r_direction) + reli) * hiddenNPerOuputN);

  //           for (int k = offset; k < offset + hiddenNPerOuputN; k++)
  //           {
  //             //unroll indexes
  //             int curIdx = idxs[k]; // get random index
  //             int h_i = curIdx / layer_size[1];
  //             int h_j = curIdx % layer_size[1];
  //             outfile << z << ' ' << h_i << ' ' << h_j << ' ' << (z + 1) << ' ' << i << ' ' << j << ' ' << 0.02 << "\n";
  //             outfile << z << ' ' << h_i << ' ' << h_j << ' ' << (z + 1) << ' ' << i << ' ' << j << ' ' << -0.02 << "\n";
  //           }
  //         }
  //       }
  //     }
  //   }

  // this is for sparse random disjoint connectivity for unsquare sections in output layers
  //int r_direction_i = 3; //how many neurons in x dimension for a given direction
  //int r_direction_j = 1; // how many neurons in y dimension for a given direction
  bool includeRecurrentInhibition = false;

  double excitationWeightValue = 0.02;
  double inhibitionWeightValue = -(excitationWeightValue / (r_direction_i * r_direction_j)); // exitation is disjoint, inhibition is all to all. more inhibition synapses so need to reduce magnitude

  int *idxs = new int[n_m];
  for (int i = 0; i < n_m; i++)
    idxs[i] = i;

  z = 1;
  int r_direction = layer_size[2] / 3;                          // side of square that coresponds to output layer diretion
  int hiddenNPerOuputN = n_m / (r_direction_i * r_direction_j); // split hidden layer evenly between number of output layer neurons in each output layer direction chunk

  //go direction by direction to ensure that every direction has a different shuffle of the array
  for (int directioni = 0; directioni < 3; directioni++)
  {
    for (int directionj = 0; directionj < 3; directionj++)
    {
      //dont include connections in middle cells to save on computation time
      if (directioni == 1 && directionj == 1)
        continue;

      //each direction gets a shuffle of idxs
      random_shuffle(idxs, (idxs + n_m));
      //for each output layer cell in the direction
      for (int reli = 0; reli < r_direction_i; reli++)
      {
        for (int relj = 0; relj < r_direction_j; relj++)
        {
          //real output layer coords
          int i = reli + (directioni * r_direction);
          int j = relj + (directionj * r_direction);

          int offset = (((relj * r_direction_i) + reli) * hiddenNPerOuputN);

          for (int k = offset; k < offset + hiddenNPerOuputN; k++)
          {
            //unroll indexes
            int curIdx = idxs[k]; // get random index
            int h_i = curIdx / layer_size[1];
            int h_j = curIdx % layer_size[1];
            outfile << z << ' ' << h_i << ' ' << h_j << ' ' << (z + 1) << ' ' << i << ' ' << j << ' ' << excitationWeightValue << "\n";
          }

	  //inhibition from hidden layer to output layer
	  //inhibition is all to all
	  for (int hi = 0; hi < M; hi++)
	  {
	    for (int hj = 0; hj < M; hj++)
	    {
	          outfile << z << ' ' << hi << ' ' << hj << ' ' << (z + 1) << ' ' << i << ' ' << j << ' ' << inhibitionWeightValue << "\n";
	    }
	  }

        }
      }
    }
  }

  // inhibition for neurons in a population direction
  if (includeRecurrentInhibition)
  {
    for (int directioni = 0; directioni < 3; directioni++)
    {
      for (int directionj = 0; directionj < 3; directionj++)
      {
        //dont include connections in middle cells to save on computation time
        if (directioni == 1 && directionj == 1)
          continue;

        //for each output layer cell in the direction
        for (int reli = 0; reli < r_direction_i; reli++)
        {
          for (int relj = 0; relj < r_direction_j; relj++)
          {
            //real output layer coords
            int i = reli + (directioni * r_direction);
            int j = relj + (directionj * r_direction);

            for (int relii = 0; relii < r_direction_i; relii++)
            {
              for (int reljj = 0; reljj < r_direction_j; reljj++)
              {
                //real output layer coords
                int ii = relii + (directioni * r_direction);
                int jj = reljj + (directionj * r_direction);
                if (i != ii || j != jj)
                {
                  outfile << (z + 1) << ' ' << i << ' ' << j << ' ' << (z + 1) << ' ' << ii << ' ' << jj << ' ' << -0.02 << "\n";
                }
              }
            }
          }
        }
      }
    }
  }

  if (!output_file.empty())
    outfileext.close();

  //double chunk=0.001;  //unit of available synaptic strength
}

int main(int argc, char *argv[])
{
  int rand_seed = -1;
  add_int_param(rand_seed);
  add_int_param(middle_fan_in);
  add_int_param(r_direction_i);
  add_int_param(r_direction_j);

  string network = "twol.conn"; //output file
  add_string_param(network);
  assert(cmdline_parameters(argc, argv));
  if (rand_seed != -1)
    srand(rand_seed);
  print_parameters();
  c_m = middle_fan_in;
  create_network(network);
  return 0;
}
