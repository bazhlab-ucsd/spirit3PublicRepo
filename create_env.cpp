#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
using namespace std;
int const size = 50;
int env[size][size];
void create_network(const string output_file){ 
  ofstream outfileext;
  if (!output_file.empty()) outfileext.open(output_file.c_str());

  //decide whether we flush to file or screen
  ostream & outfile = output_file.empty()?cout:outfileext;
  
  int a,i,j;
  
  env[0][0]=2;
  for(a=0;a<(size*size/10);a++){
    while(1){
      i=rand()%size;
      j=rand()%size;
      if(env[i][j]==0){
        env[i][j]=1;
        break;
      }
    }
  }
  for(int j=0; j<size; ++j){
    for(int i=0; i<size; ++i){
      outfile << env[i][j];
      outfile << ' ';
    }
    outfile << '\n';
  }
  if (!output_file.empty()) outfileext.close();
}

int main(){
  create_network("rand10.env");
  return 0;
}
