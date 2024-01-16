#include "connection.h"
#include <assert.h>
#include <iostream>
#include <math.h>

using namespace std;

void Connection::clean_trace(time_type ctime)
{ //remove traces older than trace_mem
  list<Trace>::iterator it = trace.begin();
  while (!trace.empty() && ctime - it->time > trace_mem)
    it = trace.erase(it);
}

//double hard_cap; //higher bound for connection strength, the idea is not to allow single input to fire
Connection::Connection() : strength(0), trace_average(0)
{
  post_pre_STDP_scale = -stdp_scale;
  pre_post_STDP_scale = stdp_scale;
  freeze = false;
};
Connection::Connection(Cell *pre, Cell *post) : pre_cell(pre), post_cell(post), strength(0), trace_average(0)
{
  post_pre_STDP_scale = -stdp_scale;
  pre_post_STDP_scale = stdp_scale;
  freeze = false;
}
Connection::Connection(Cell *pre, Cell *post, double str) : pre_cell(pre), post_cell(post), strength(str), trace_average(0)
{
  post_pre_STDP_scale = -stdp_scale;
  pre_post_STDP_scale = stdp_scale;
  freeze = false;
}

bool Connection::compareConnections(Connection * first, Connection * second){
  if(first == NULL || second == NULL)
    assert(false);
  return first->strength > second->strength;
}

void Connection::freezeTopPercentageOfSynapses(vector<Connection *> connections, double percentage){
  vector<Connection *> sorted;
  std::copy_if (connections.begin(), connections.end(), std::back_inserter(sorted), [](Connection * i){return i->strength >=0;} );
  sort(sorted.begin(), sorted.end(), compareConnections);

  int size = sorted.size();
  if(size == 0)
    return;

  if(percentage < 0 || percentage > 1){
    cout << "Invalid percentage value : " << percentage << endl;
    assert(0);
  }

  int freezeNumber = ceil(((double)size) * percentage);
  if(freezeNumber < 1)
    freezeNumber = 1;

  for(int i = 0; i < freezeNumber; i++){
    sorted[i]->freeze = true;
  }
}

/*
	void Connection::dump(int level) {
		if (pre_cell==NULL || post_cell==NULL) {cout<<"NULL!\n";return;}
		cout<<pre_cell->location.z<<pre_cell->location.y<<pre_cell->location.x<<"->"
		<<post_cell->location.z<<post_cell->location.y<<post_cell->location.x<<": "<<	
		strength<<"\n";
	 } 

*/
