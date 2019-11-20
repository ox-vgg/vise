#ifndef _waveDescriptor_h_
#define _waveDescriptor_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "../wavelet/wavelet.h"

class DllExport WaveDescriptor : public CornerDescriptor{

 protected:
  void computeCodeHistogram(DARY *img);
  void computeCodeHistogram(DARY *img, int sxs, int sys, int sw, int level);    
  int code(DARY *img, int x, int y);

 public:
	WaveDescriptor():CornerDescriptor(){;}
	WaveDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	WaveDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image, Wavelet *wave);

};
void computeWaveDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
