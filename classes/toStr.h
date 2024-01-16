#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define to_str(x) dynamic_cast<ostringstream &>((ostringstream() << dec << x)).str()