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

#ifndef _FEAT_GETTER_H_
#define _FEAT_GETTER_H_

#include <stdint.h>
#include <vector>

#include "ellipse.h"
#include "macros.h"
#include "util.h"


class descGetter {
    
    public:
        
        virtual void
            getDescs( const char fileName[], std::vector<ellipse> &regions, uint32_t &numFeats, float *&descs ) const =0;
        
        virtual std::string
            getRawDescs(float const *descs, uint32_t numFeats) const {
                // overwrite if not float
                return std::string(
                           reinterpret_cast<const char*>(descs),
                           numFeats*numDims()*sizeof(float)/sizeof(char) );
            }
        
        // supportedDtypes= np.array( [np.dtype(x) for x in ['uint8', 'uint16', 'uint32', 'uint64', 'float32', 'float64']] )
        virtual uint8_t
            getDtypeCode() const {
                return 4; // float32
            }
        
        virtual uint32_t
            numDims() const =0;
        
        virtual ~descGetter()
            {}
    
};



class regionGetter {
    
    public:
        
        virtual void
            getRegs( const char fileName[], uint32_t &numRegs, std::vector<ellipse> &regions ) const =0;
        
        virtual ~regionGetter()
            {}
    
};



class featGetter {
    
    public:
        
        virtual void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const =0;
        
        virtual ~featGetter()
            {}
        
        virtual uint32_t
            numDims() const =0;
        
        virtual void
            getFeats( const char fileName[], uint32_t xl, uint32_t xu, uint32_t yl, uint32_t yu, uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const;
        
        virtual std::string
            getRawDescs(float const *descs, uint32_t numFeats) const =0;
        
        virtual uint8_t
            getDtypeCode() const =0;
};


class splitRegDesc : public featGetter {
    
    public:
        
        splitRegDesc( regionGetter const *aRegionGetterObj, descGetter const *aDescGetterObj ) : regionGetterObj(aRegionGetterObj), descGetterObj(aDescGetterObj)
            {}
        
        void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const {
                regionGetterObj->getRegs( fileName, numFeats, regions );
                descGetterObj->getDescs( fileName, regions, numFeats, descs );
            }
        
        inline uint32_t
            numDims() const { return descGetterObj->numDims(); }
        
        inline std::string
            getRawDescs(float const *descs, uint32_t numFeats) const {
                return descGetterObj->getRawDescs(descs, numFeats);
            }
        
        inline uint8_t
            getDtypeCode() const {
                return descGetterObj->getDtypeCode();
            }
    
    private:
        regionGetter const *regionGetterObj;
        descGetter const *descGetterObj;
        DISALLOW_COPY_AND_ASSIGN(splitRegDesc)
    
};


#endif
