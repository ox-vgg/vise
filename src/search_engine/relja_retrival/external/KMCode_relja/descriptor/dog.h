#ifndef _dog_h_
#define  _dog_h_
#include <assert.h>                 
#include "cornerDescriptor.h"                                     
#include "../gauss_iir/gauss_iir.h" 

void dog(DARY *im, vector<CornerDescriptor*> &corners);

                                                            
#endif
