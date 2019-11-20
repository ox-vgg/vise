/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#ifndef _SOFT_ASSIGNER_
#define _SOFT_ASSIGNER_

#include "quant_desc.h"
#include <stdint.h>
#include <sys/types.h>
#include <math.h>



class softAssigner {
    
    public:
        
        virtual bool
            needsFeat() const = 0;
        
        virtual void
            getWeights( quantDesc &ww, float *feat= NULL ) const =0;
        
        virtual ~softAssigner() {}
    
};



class SA_exp : public softAssigner {
    
    public:
        
        SA_exp( float aSigmaSq= 6250 ) : sigmaSq(aSigmaSq)
            {}
        
        bool
            needsFeat() const { return false; };
        
        void
            getWeights( quantDesc &ww, float *feat= NULL ) const {
                
                uint iNN, KNN= ww.rep.size();
                float total= 0, curr, distSq;
                for (iNN= 0; iNN<KNN; ++iNN){
                    distSq= ww.rep[iNN].second;
                    curr= exp( -distSq/(2*sigmaSq) );
                    ww.rep[iNN].second= curr;
                    total+= curr;
                }
                
                // L1 normalize
                for (iNN= 0; iNN<KNN; ++iNN)
                    ww.rep[iNN].second /= total;
                
            }
    
    private:
        
        const float sigmaSq;
        
};


#endif
