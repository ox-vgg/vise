#include "histoDescriptor.h"

/**********************************************/
void HistoDescriptor::init(void){
    vec=NULL;
    size=0; 
}

/**********************************************/
void HistoDescriptor::allocVec(int size_in){
    if(size_in >0){
	if(vec!=NULL)delete [] vec;
	size=size_in;
	vec = new double[size];
	for(int i=0;i<size;i++)vec[i]=0;
    }
}
  

/**********************************************/
void HistoDescriptor::read_database( inputfstream &input){
    double *write_vec = new double[size+6];
    input.read(write_vec,(size+6)*sizeof(double));
    model=(long unsigned int)write_vec[0];
    for(int i=0;i<size;i++)
	vec[i]=write_vec[i+6];
    delete [] write_vec;
    
}
/**********************************************/
void HistoDescriptor::write_database( outfstream &output, long unsigned int model_in){
    model=model_in;
    double *write_vec = new double[size+6];
    write_vec[0]=(double)model;
    for(int i=0;i<size;i++)
	write_vec[i+6]=vec[i];
    output.write(write_vec,(size+6)*sizeof(double));
    delete [] write_vec;    
}

 
/**********************************************/
void HistoDescriptor::read( inputfstream &input, int size_in){
  if(size_in>0){
      //input >> d_scale;
    allocVec(size_in);
    for(int j=0;j<size;j++){
      input >> vec[j];
    }
  } 
}

/**********************************************/
void HistoDescriptor::write(outfstream &output){
  if(size>0){
    output  << size<< endl;//"  " << d_scale << " ";
    for(int j=0; j<size;j++){
      output << vec[j] << " ";
    }
  }
  output << endl;
}

  /**********************************************/
int Zone::isIn(int x, int y){
    return 1;
}
