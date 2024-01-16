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
  int z,y,x;
  for(z=0;z<4;z++){
    for(y=0; y<7; ++y){
      for(x=0; x<7; ++x){
        if(z==0){
          outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< y <<' '<< x <<' '<<"ex"<<' '<<.02<<"\n";
          outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< y <<' '<< x <<' '<<"ex"<<' '<<.02<<"\n";
        }
        if(z==1){
          if(x<3){
            if(y<3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 0 <<' '<< 0 <<' '<<"ex"<<' '<<.02<<"\n";
            }
            if(y==3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 1 <<' '<< 0 <<' '<<"ex"<<' '<<.02<<"\n";
            }
            if(y>3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 2 <<' '<< 0 <<' '<<"ex"<<' '<<.02<<"\n";
            }
          }
          if(x==3){
            if(y<3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 0 <<' '<< 1 <<' '<<"ex"<<' '<<.02<<"\n";
            }
            if(y==3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 1 <<' '<< 1 <<' '<<"ex"<<' '<<.02<<"\n";
            }
            if(y>3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 2 <<' '<< 1 <<' '<<"ex"<<' '<<.02<<"\n";
            }
          }
          if(x>3){
            if(y<3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 0 <<' '<< 2 <<' '<<"ex"<<' '<<.02<<"\n";
            }
            if(y==3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 1 <<' '<< 2 <<' '<<"ex"<<' '<<.02<<"\n";
            }
            if(y>3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+2) <<' '<< 2 <<' '<< 2 <<' '<<"ex"<<' '<<.02<<"\n";
            }
          }
        }
        if(z==2){
          if(x<3){
            if(y<3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 0 <<' '<< 0 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
            if(y==3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 1 <<' '<< 0 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
            if(y>3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 2 <<' '<< 0 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
          }
          if(x==3){
            if(y<3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 0 <<' '<< 1 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
            if(y==3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 1 <<' '<< 1 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
            if(y>3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 2 <<' '<< 1 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
          }
          if(x>3){
            if(y<3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 0 <<' '<< 2 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
            if(y==3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 1 <<' '<< 2 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
            if(y>3){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< 2 <<' '<< 2 <<' '<<"ex"<<' '<<-.001<<"\n";
            }
          }
        }
      }
    }
  }
  if (!output_file.empty()) outfileext.close();
}

int main(){
  create_network("prelearned.conn");
  return 0;
}
