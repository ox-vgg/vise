#ifndef _textureDescriptor_h_
#define _textureDescriptor_h_


#include "../ImageContent/imageContent.h"
#include "histoDescriptor.h"
#include "../wavelet/wavelet.h"

class DllExport TextureDescriptor: public HistoDescriptor{

 public:

    TextureDescriptor():HistoDescriptor(){;}
    TextureDescriptor(DARY *img_in,Zone *zone_in, const char* dirmodel ):HistoDescriptor(){
      init(dirmodel);computeComponents(img_in,zone_in);}
    void computeComponents(DARY *img_in,Zone *zone); 
    ~TextureDescriptor();
   
    private:
    void computeCodes(DARY *image,DARY *codes);
    void computeCodesDistribution(DARY *codes,int sxs, int sys, int sxe, int sye, 
			      int offset);
    double getCode(int a, int b, int c, int d);
    void init(const char* dirmodel);
    Wavelet *wave;
    double *thres;
};

double distance(TextureDescriptor *desc1,TextureDescriptor *desc2);

#endif
