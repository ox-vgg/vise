#ifndef _jetLocalDirLum_h_
#define _jetLocalDirLum_h_

#include "jetLocalDirectional.h"

class DllExport JetLocalDirLum : public JetLocal {

 private:

 public:
    JetLocalDirLum ():JetLocal(){;}
    
    void computeComponents(DARY *img_in);    

};
void computeJLDLDescriptors(DARY *image,  vector<CornerDescriptor *> &desc);
#endif
