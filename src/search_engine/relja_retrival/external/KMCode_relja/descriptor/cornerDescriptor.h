#ifndef _cornerDescriptor_h_
#define _cornerDescriptor_h_

#include <iostream>
#include <fstream>
using namespace std;
#include "corner.h"
#include "../matrix/matrix.h"

class DllExport CornerDescriptor: public Corner{

private:
	void initd();
 protected:
    float *vec;
    int size;
    float d_scale;
    unsigned long int model;
    int order;//derivative order
 public:

    CornerDescriptor():Corner(){initd();}
    CornerDescriptor(int size_in):Corner(){initd();allocVec(size_in);}
    CornerDescriptor(float xin, float yin):Corner(xin,yin){initd();}
    CornerDescriptor(float xin, float yin, float cornerness_in, float scale_in, float angle_in)
					:Corner(xin,yin,cornerness_in,scale_in,angle_in){initd();}
    CornerDescriptor(float xin, float yin, float cornerness_in, float scale_in)
					:Corner(xin,yin,cornerness_in,scale_in){initd();}
    CornerDescriptor(float xin, float yin, float scale_in, float ori_in, int ext)
					:Corner(xin,yin,scale_in,ori_in,ext){initd();}
    CornerDescriptor(float xin, float yin, float cornerness_in, float scale_in,float l1_in, float l2_in)
      :Corner(xin,yin,cornerness_in,scale_in,l1_in,l2_in){initd();}

    inline Corner * const getCorner(void) const {return (Corner *)this;}

    void copy(CornerDescriptor* ds);
    void setOrder(int order_in){order=order_in;}
    inline int const getOrder()const {return order;}

    /*****READ WRITE***/
    void readFred( ifstream &input, int size_in);
    void readCommon( ifstream &input, int size_in);
    void write( ofstream &output);
    void writeCommon( ofstream &output);
    void writeCommonWA( ofstream &output);
    void read( ifstream &input, int size_in);	
    void read_database( ifstream &input);	
    void writeDescriptor(const char* filename, const char* id);
    void write_database( ofstream &output, long unsigned int model_in);

    inline float * getVec(void){return vec;}
    // inline float * const getWriteVec(void) const {return write_vec;}

    inline float getV(int i){ if(i<size)return (vec[i]);else return 0;}    
    inline void setV(int i, float val){if(i<size && i>=0)vec[i]=val;}

    void allocVec(int);
    inline int const getType() const {return type;} 
    inline void setType(int type_in){type = type_in;} 

    inline int const getSize() const {return size;} 
    inline unsigned long int const getModel() const {return model;} 
    inline void setSize(int size_in){size=size_in;}    
    
    inline void setDescriptorScale(float scale_in){d_scale=scale_in;} 
    inline float const getDescriptorScale() const {return d_scale;} 

    void changeBase(Matrix *base);
    void changeBase(int dim, float* mat);
    void changeBase(float* mat);
    void pca(int dim, float *avg, float *base);

    ~CornerDescriptor(){if(vec!=NULL)delete[] vec;}

    // James.
    bool is_fully_inside(float x1, float x2, float y1, float y2);
    // ---
  
};

typedef vector <CornerDescriptor*> CornerDescList;
typedef vector <CornerDescList> CornerDescSequence;


DllExport void changeBase(vector<CornerDescriptor *> &desc, Matrix *base); 
DllExport void writeCorners(vector<CornerDescriptor*> cor, const char* points_out, int format=0);
//DllExport void drawCorners(DARY *image, vector<CornerDescriptor*> cor, const char* image_out);
DllExport void loadCorners( const char* points1, vector<CornerDescriptor*> &cor1, int format=0);
void loadFredCorners( const char* points1, vector<CornerDescriptor*> &cor1);


#endif
