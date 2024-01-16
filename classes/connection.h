#ifndef CONNECTION_H
#define CONNECTION_H
#include <list>
#include "trace.h"
#include "timeStructure.h"
#include "const_expr.h"
#include <algorithm>
#include <math.h>
#include <vector>

using namespace std;

class Cell;

extern double stdp_scale;

class Connection
{
public:
  Cell *pre_cell;
  Cell *post_cell;
  double strength;
  double relative_inhib;                            //used in input balancing to keep negative strengths in same proportion to input_target
  list<Trace> trace;                                //#stdp traces for firing in this connection
  double trace_average;                             //exponential average for traces
  bool freeze;
  static const_expr double trace_average_d = 0.001; // should be 1-2 orders of magnitude higher than happy_d
  void clean_trace(time_type ctime);                //remove traces older than trace_mem

  //STDP related timing
  static const int spike_mem = 120;                      //time window for keeping spike mem for STDP
  static const int trace_mem = TimeStructure::epoch * 6; //time window for keeping the STDP trace
  static const_expr double STDP_tau = 40;                //STDP time constant, how unimportant is time difference between two spikes
  double post_pre_STDP_scale;                            // = -stdp_scale; // how fast do synaptic strength change (negative, rarely used) 
  double pre_post_STDP_scale;                            // = stdp_scale; // how fast do synaptic strength change (positive, commonly used)
  //expects current time and last spike time
  // inline functions must go in .h file
  inline double add_pre_post_stdp_trace(const time_type t, const time_type pre, bool apply)
  {
    if(!apply)
      return 0;

    if (t - pre < spike_mem) //deleting mechanism for old stuff can be put here as well
    {
      trace.push_back(Trace(t, pre_post_STDP_scale * exp(-(t - pre) / STDP_tau)));
    //trace.push_back(Trace(t,pre_post_STDP_scale*strength*exp(-(t-pre)/STDP_tau)));
      return pre_post_STDP_scale * exp(-(t - pre) / STDP_tau);
    }
    return 0;
  }
  // inline functions must go in .h file
  inline double add_post_pre_stdp_trace(const time_type t, const time_type pre, bool apply)
  {
    if(!apply)
      return 0;

    if (t - pre < spike_mem) //deleting mechanism for old stuff can be put here as well
    {
      trace.push_back(Trace(t, post_pre_STDP_scale * exp(-(t - pre) / STDP_tau)));
    //trace.push_back(Trace(t,post_pre_STDP_scale*strength*exp(-(t-pre)/STDP_tau)));
      return post_pre_STDP_scale * exp(-(t - pre) / STDP_tau);
    } 
    return 0;
  }
  //double hard_cap; //higher bound for connection strength, the idea is not to allow single input to fire
  Connection();
  Connection(Cell *pre, Cell *post);
  Connection(Cell *pre, Cell *post, double str);
  void dump(int level);
  static bool compareConnections(Connection * first, Connection * second); // returns true if first > second
  static void freezeTopPercentageOfSynapses(vector<Connection *> in_connections, double percentage);// freezes strongest top percentage of connections in the list
}; //Connection

#endif // CONNECTION_H
