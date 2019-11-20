#ifndef _jetLocalDirectional_h_
#define _jetLocalDirectional_h_

#include "jetLocal.h"

class DllExport JetLocalDirectional : public JetLocal {

 private:

 public:
    JetLocalDirectional():JetLocal(){;}
    
    void computeComponents(DARY *img_in);    

};
void computeJLDDescriptors(DARY *image,vector<CornerDescriptor *> &desc);
void computeJLDDescriptors(DARY *image, vector<CornerDescriptor *> &desc, Matrix *base);
#endif
