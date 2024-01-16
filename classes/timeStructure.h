#ifndef TIMESTURCTUIRE_H
#define TIMESTURCTUIRE_H

//We might want to rather define structured time and save some time&space in stdp traces
typedef long long time_type; //32 bits won't do for us;

class TimeStructure
{
public:
  time_type ctime;              //current time
  time_type total_time;         //how long should the simulation be
  int total_aeons;              //how many aeons in total for simulation
  int aeon; //numbers epochs in aeon 
  static const int epoch = 600; //numbers of timesteps in in epoch

  TimeStructure();
  void setEpochsPerAeon(int epochsPerAeon);
  void set_total_aeons(int aeons);
  time_type aeons(); //returns time in aeons
  time_type epochs();
  int flash_cells_check();
  int ballence_connections();
  int move_critter();
  int epoch_check();
  int aeon_check();
  int long_check();
}; //TimeStructure

#endif // TIMESTURCTUIRE_H
