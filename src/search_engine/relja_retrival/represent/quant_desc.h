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

#ifndef _QUANT_DESC_H_
#define _QUANT_DESC_H_

#include <vector>
#include <stdint.h>
#include <math.h>



typedef std::pair<uint32_t, float> wordWeightPair;

class quantDesc {
    
    public:
        
        quantDesc() {}
        
        quantDesc( uint32_t wordID, float freq= 1.0f ) {
            rep.push_back( std::make_pair( wordID, freq ) );
        }
        
        bool
            operator==( quantDesc const &qD2 ) const {
                if (rep.size()!=qD2.rep.size())
                    return false;
                for (size_t i=0; i<rep.size(); ++i)
                    if (rep[i].first!=qD2.rep[i].first || rep[i].second!=qD2.rep[i].second)
                        return false;
                return true;
        }
        
        std::vector< wordWeightPair > rep;
        
        static void
            flattenHard( std::vector<quantDesc> const &qDs, std::vector<uint32_t> &words ){
                
                words.clear();
                words.reserve( qDs.size() );
                
                for (std::vector<quantDesc>::const_iterator itQD= qDs.begin(); itQD!=qDs.end(); ++itQD)
                    words.push_back( itQD->rep[0].first );
                
            }
        
        static void
            flatten( std::vector<quantDesc> const &qDs, std::vector< wordWeightPair > &wW, std::vector<uint32_t> *inds= NULL, bool doL2norm= false ){
                
                wW.clear();
                uint32_t estSize= qDs.size()*( qDs.size()>0?qDs[0].rep.size():0 );
                wW.reserve( estSize );
                if (inds!=NULL){
                    inds->clear();
                    inds->reserve( estSize );
                }
                
                uint32_t ind= 0;
                double l2norm;
                
                for (std::vector<quantDesc>::const_iterator itQD= qDs.begin(); itQD!=qDs.end(); ++itQD, ++ind){
                    
                    if (doL2norm){
                        l2norm= 0.0;
                        for (std::vector< wordWeightPair >::const_iterator itWW= itQD->rep.begin(); itWW!=itQD->rep.end(); ++itWW)
                            l2norm+= pow(itWW->second,2);
                        l2norm= sqrt(l2norm);
                    } else
                        l2norm= 1.0;
                        
                    for (std::vector< wordWeightPair >::const_iterator itWW= itQD->rep.begin(); itWW!=itQD->rep.end(); ++itWW){
                        wW.push_back( std::make_pair(itWW->first, itWW->second/l2norm) );
                        if (inds!=NULL)
                            inds->push_back( ind );
                    }
                    
                }
                
            }
    
};

#endif
