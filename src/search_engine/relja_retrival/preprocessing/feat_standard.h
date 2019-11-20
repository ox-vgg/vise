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

#ifndef _FEAT_STANDARD_H_
#define _FEAT_STANDARD_H_

#include <stdexcept>
#include <set>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "colour_sift.h"
#include "desc_to_hell.h"
#include "ellipse.h"
#include "feat_getter.h"
#include "holidays_public.h"
#include "macros.h"



// Hessian-Affine detector by Krystian Mikolajczyk
class reg_KM_HessAff : public regionGetter {
    public: void getRegs( const char fileName[], uint32_t &numRegs, std::vector<ellipse> &regions ) const;
};



// SIFT by Krystian Mikolajczyk
class desc_KM_SIFT : public descGetter {
    public:
        desc_KM_SIFT(float aScaleMulti= 3.0, bool aUpright= false) : scaleMulti(aScaleMulti), upright(aUpright) {}
        void getDescs( const char fileName[], std::vector<ellipse> &regions, uint32_t &numFeats, float *&descs ) const;
        std::string getRawDescs(float const *descs, uint32_t numFeats) const;
        inline uint8_t getDtypeCode() const { return 0; /* uint8 */ }
        uint32_t numDims() const { return 128; }
    private:
        float const scaleMulti;
        bool const upright;
};



class featGetter_standard : public featGetter {
    
    public:
        
        featGetter_standard( const char id[] ) :
            featGetterObj(NULL), regionGetterObj(NULL), descGetterObj(NULL) {
            
            bool correctSpec= false;
            
            std::vector<std::string> optionList;
            boost::split(optionList, id, boost::is_any_of("-"));
            std::set<std::string> optionSet(optionList.begin(), optionList.end());
            
            bool SIFTscale3= optionSet.count("scale3");
            
            if ( optionSet.count("hesaff") ){
                
                regionGetterObj= new reg_KM_HessAff();
                bool upright= optionSet.count("up");
                
                if (SIFTscale3)
                    descGetterObj= new desc_KM_SIFT(3, upright);
                else
                    // for some reason this was the original setting in James's engine_3, but 3 is default and works better
                    descGetterObj= new desc_KM_SIFT(1.732, upright);
                
                if ( optionSet.count("sift") ){
                    
                    correctSpec= true;
                    
                } else if ( optionSet.count("rootsift") ) {
                    
                    descGetter *dg= descGetterObj;
                    descGetterObj= new descToHell(dg,true);
                    correctSpec= true;
                }
                
                featGetterObj= new splitRegDesc( regionGetterObj, descGetterObj );
                
            } else if ( optionSet.count("sande") ) {
                
                if (optionSet.count("sift")){
                    featGetterObj= new vanDeSande("sift");
                    correctSpec= true;
                } else if (optionSet.count("opponentsift")){
                    featGetterObj= new vanDeSande("opponentsift");
                    correctSpec= true;
                }
                
            } else if ( optionSet.count("holidays") ) {
                
                if (boost::starts_with(id,"holidays-sift")){
                    featGetterObj= new holidaysPublic(false);
                    correctSpec= true;
                } else if (boost::starts_with(id,"holidays-rootsift")){
                    featGetterObj= new holidaysPublic(true);
                    correctSpec= true;
                }
                
            }
            
            if (!correctSpec) {
                throw std::runtime_error( "Unknown standard featGetter" );
            }
            
        }
        
        ~featGetter_standard(){
            
            delete featGetterObj;
            if (regionGetterObj!=NULL)
                delete regionGetterObj;
            if (descGetterObj!=NULL)
                delete descGetterObj;
            
        }
        
        inline void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const {
            return featGetterObj->getFeats( fileName, numFeats, regions, descs );
        }
        
        inline std::string getRawDescs(float const *descs, uint32_t numFeats) const {
            return featGetterObj->getRawDescs(descs, numFeats);
        }
        
        inline uint8_t getDtypeCode() const {
            return featGetterObj->getDtypeCode();
        }
        
        inline uint32_t
            numDims() const { return featGetterObj->numDims(); }
    
    private:
        
        featGetter *featGetterObj;
        regionGetter *regionGetterObj;
        descGetter *descGetterObj;
        
        DISALLOW_COPY_AND_ASSIGN(featGetter_standard)
    
};

#endif
