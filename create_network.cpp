#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
using namespace std;

void create_network(const string output_file){ 
  ofstream outfileext;
  if (!output_file.empty()) outfileext.open(output_file.c_str());

  //decide whether we flush to file or screen
  ostream & outfile = output_file.empty()?cout:outfileext;
  int z,y,x,i,j;
  for(z=0;z<4;z++){
    for(y=0; y<7; ++y){
      for(x=0; x<7; ++x){
        if(z==0){
          outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< y <<' '<< x <<' '<<"ex"<<' '<<.02<<"\n";
          outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< y <<' '<< x <<' '<<"ex"<<' '<<.02<<"\n";
        }
        if(z==1){
          for(i=0;i<3;i++){
            for(j=0;j<3;j++){
              if((i!=1 || j!=1) && (x!=3 || y!=3))
                outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< i <<' '<< j <<' '<<"ex"<<' '<<.06<<"\n";
            }
          }
        }
        if(z==2){
          for(i=0;i<3;i++){
            for(j=0;j<3;j++){
              if((i!=1 || j!=1) && (x!=3 || y!=3))
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
  create_network("flat.conn");
  return 0;
}
