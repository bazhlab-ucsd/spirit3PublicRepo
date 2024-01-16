#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
using namespace std;

void create_network(const string output_file){ 
  ofstream outfileext;
  if (!output_file.empty()) outfileext.open(output_file.c_str());
  int l1,l23,l4;
  l1=7;
  l23=28;
  l4=3;

  //decide whether we flush to file or screen
  ostream & outfile = output_file.empty()?cout:outfileext;
  int z,y,x,i,j;
  for(z=0;z<4;z++){
    for(y=0; y<7; ++y){
      for(x=0; x<7; ++x){
        if(z==0){
          for(i=0;i<28;i++){
            for(j=0;j<28;j++){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<"ex"<<' '<<.1<<"\n";
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<"ex"<<' '<<-.1<<"\n";
            }
          }
        }
      }
    }
    for(y=0; y<28; ++y){
      for(x=0; x<28; ++x){
        if(z==1){
          for(i=0;i<3;i++){
            for(j=0;j<3;j++){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<"ex"<<' '<<.06<<"\n";
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<"ex"<<' '<<-.06<<"\n";
            }
          }
        }
      }
    }
  } 
  if (!output_file.empty()) outfileext.close();
}

int main(){
  create_network("twol.conn");
  return 0;
}
