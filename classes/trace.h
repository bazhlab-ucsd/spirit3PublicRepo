#ifndef TRACE_H
#define TRACE_H
#include "timeStructure.h"

//STDP trace
class Trace
{
  public:
    time_type time;
    double mag; //magnitude
    Trace(time_type t, double m) : time(t), mag(m){};
};

#endif // TRACE_H
