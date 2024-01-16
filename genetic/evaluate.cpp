#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
using namespace std;


int main(){
  ifstream myfile("happiness.out.0");
  int x;
  double score=0;
  int dur=20000;
  int happy[dur][2];
  for(x=0;x<dur;x++){
    assert(myfile.good());
    myfile>>happy[x][0];
    myfile>>happy[x][1];
  }
  myfile.close();
  for(x=5000;x<dur;x++){
  score+=happy[x][1];
  }
  score=score/(dur-5000);
  return 0;
}
