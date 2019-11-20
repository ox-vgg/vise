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

#ifndef _RETRIEVER_V2_H_
#define _RETRIEVER_V2_H_

#include "clst_centres.h"
#include "embedder.h"
#include "feat_getter.h"
#include "index_entry.pb.h"
#include "macros.h"
#include "proto_index.h"
#include "retriever.h"
#include "uniq_entries.h"

// for kd-tree based nearest neighbour search
#include <vl/generic.h>
#include <vl/kdtree.h>


class retrieverV2 : public retriever {
    
    public:
        
        // need fidx for internal queries
        retrieverV2( protoIndex const *fidx= NULL,
                     protoIndex const *iidx= NULL,
                     bool needXY= false,
                     bool needEllipse= false,
                     embedderFactory const *embFactory= NULL,
                     featGetter const *featGetterObj= NULL,
                     VlKDForest* kd_forest= NULL,
                     clstCentres const *clstCentresObj= NULL
                     ) : embFactory_(embFactory),
                         fidx_(fidx),
                         iidx_(iidx),
                         needXY_(needXY),
                         needEllipse_(needEllipse),
                         featGetter_(featGetterObj),
                         kd_forest_(kd_forest),
                         clstCentres_(clstCentresObj) {}
        
        virtual
            ~retrieverV2() {}
        
        virtual void
            queryExecute( rr::indexEntry &queryRep, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const =0;
        
        inline void
            queryExecute( query const &queryObj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const {
                rr::indexEntry queryRep;
                getQueryRep( queryObj, queryRep );
                queryExecute(queryRep, queryRes, toReturn);
            }
        
        virtual void
            externalQuery_computeData( std::string imageFn, query const &queryObj ) const;
        
        virtual uint32_t
            numDocs() const =0;
        
        embedderFactory const *embFactory_;
        
        void
            getQueryRep(query const &queryObj, rr::indexEntry &queryRep ) const;
    
    protected:
        
        protoIndex const *fidx_, *iidx_;
        bool const needXY_, needEllipse_;
        
        featGetter const *featGetter_;
        VlKDForest* kd_forest_;
        clstCentres const *clstCentres_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(retrieverV2)
};




class retrieverFromIter : public retrieverV2 {
    
    public:
        
        retrieverFromIter( protoIndex const *iidx,
                           protoIndex const *fidx= NULL,
                           bool needXY= false,
                           bool needEllipse= false,
                           embedderFactory const *embFactory= NULL,
                           featGetter const *featGetterObj= NULL,
                           VlKDForest* kd_forest= NULL,
                           clstCentres const *clstCentresObj= NULL)
                           : retrieverV2( fidx, iidx, needXY, needEllipse, embFactory, featGetterObj, kd_forest, clstCentresObj ) {}
        
        virtual
            ~retrieverFromIter() {}
        
        inline void
            queryExecute( rr::indexEntry &queryRep, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const {
                ASSERT(iidx_!=NULL);
                #if 1
                onlineUEIterator ueIter(queryRep, *iidx_);
                #else
                uniqEntries ue;
                iidx_->getUniqEntries(queryRep, ue);
                precompUEIterator ueIter(ue);
                #endif
                queryExecute(queryRep, &ueIter, queryRes, toReturn);
            }
        
        virtual void
            queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const =0;
        
        virtual bool
            changesEntryWeights() const { return false; }
    
    private:
        DISALLOW_COPY_AND_ASSIGN(retrieverFromIter)
};

#endif
