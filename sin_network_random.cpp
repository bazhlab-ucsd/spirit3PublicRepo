#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
using namespace std;

void create_network(const string output_file){ 
  ofstream outfileext;
  if (!output_file.empty()) outfileext.open(output_file.c_str());
  int l1,l23,l4,ocoins;
  l1=7;
  l23=28;
  l4=3;
  //method to distribute strength
  //1. pick random connection and add chunk
  //2. do 1. ocoins times
  // we don't just pull from normal distribution, beacause we want to make sure that total sum of strength is always the same
  double chunk=0.001;  
  ocoins=10*28*28;
  //decide whether we flush to file or screen
  ostream & outfile = output_file.empty()?cout:outfileext;
  int z,y,x,i,j,coins,xx,yy;
  double conarray[28][28];
  for(z=0;z<4;z++){
    for(y=0; y<7; ++y){
      for(x=0; x<7; ++x){
        if(z==0){		//from input to middle layer
          coins=ocoins;
          for(xx=0;xx<28;xx++){   //avoid zero strength
            for(yy=0;yy<28;yy++){
              conarray[yy][xx]=chunk;
            }
          }
          while(coins>0){ 
            for(i=0;i<28;i++){
              for(j=0;j<28;j++){
                if(((double) rand()/RAND_MAX)>.99){
                  if(coins>0){
                    conarray[i][j]+=chunk;
                    coins--;
                  //printf("%f \n",conarray[i][j]);   
                  }   
                }
              }
            }        
          }          

          for(i=0;i<28;i++){
            for(j=0;j<28;j++){
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<conarray[i][j]<<"\n";
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<conarray[i][j]*-1<<"\n";
               
             
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
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<.06<<"\n";
              outfile << z<< ' ' << y << ' '<< x <<' '<< (z+1) <<' '<< i <<' '<< j <<' '<<-.06<<"\n";

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
