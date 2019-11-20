#ifndef _histoDescriptor_h_
#define _histoDescriptor_h_

#include<fstream.h>
typedef ofstream outfstream;
typedef ifstream inputfstream;
#ifdef WIN32
#ifndef DllExport
#define DllExport   __declspec( dllexport )
#define DllImport   __declspec( dllimport )
using namespace std;
#endif
#endif
#ifndef WIN32
#define DllExport  
#define DllImport  
#endif

#include "../ImageContent/imageContent.h"

class DllExport Zone{

private:
    int x,y,size;
    
protected:
public:
    Zone(){;}
    int isIn(int x, int y);
};

class DllExport HistoDescriptor{

private:
	void init();
 protected:
    double *vec;
    int size;
    unsigned long int model;
 public:

    HistoDescriptor(){init();}
    void write( outfstream &output);
    void read( inputfstream &input, int size_in);	
    void read_database(inputfstream &input);	
    void write_database(outfstream &output, long unsigned int model_in);
    void computeComponents(DARY *img_in);    
    inline double * const getVec(void) const {return vec;}
    inline double  getV(int i){ if(i<size)return (vec[i]);else return 0;}    
    inline void setV(int i, double val){if(i<size && i>=0)vec[i]=val;}
    
    void allocVec(int);
    
    inline int const getSize() const {return size;} 
    inline unsigned long int const getModel() const {return model;} 
    inline void setSize(int size_in){size=size_in;}    
    

    ~HistoDescriptor(){if(vec!=NULL)delete[] vec;}
  
};

#endif
