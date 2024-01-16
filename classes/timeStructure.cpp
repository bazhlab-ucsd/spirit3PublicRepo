#include "timeStructure.h"

TimeStructure::TimeStructure()
{
  ctime = -1; //we want to start at zero
  aeon = 100;
}

void TimeStructure::set_total_aeons(int aeons)
{
  total_aeons = aeons;
  total_time = (time_type)aeon * epoch * total_aeons;
}

void TimeStructure::setEpochsPerAeon(int epochsPerAeon){
  aeon = epochsPerAeon;
}

time_type TimeStructure::aeons() { return (ctime / (epoch * aeon)); } //returns time in aeons

time_type TimeStructure::epochs() { return (ctime / epoch); } //returns time in aeons

int TimeStructure::flash_cells_check() { return (ctime % epoch == 0); }

int TimeStructure::ballence_connections() { return (ctime % epoch == 0); }

int TimeStructure::move_critter() { return (ctime % epoch == 0); }

int TimeStructure::epoch_check() { return (ctime % epoch == 0); }

int TimeStructure::aeon_check() { return (ctime % (epoch * aeon) == 0); }

int TimeStructure::long_check() { return (ctime % epoch * 1000 * aeon == 0); }
