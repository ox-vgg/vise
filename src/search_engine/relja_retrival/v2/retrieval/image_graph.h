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

#ifndef _IMAGE_GRAPH_H_
#define _IMAGE_GRAPH_H_

#include <map>

#include "macros.h"
#include "retriever.h"


class imageGraph {
    
    public:
        
        typedef std::map<uint32_t, std::vector<indScorePair> > imageGraphType;
        
        imageGraph(){}
        
        // load
        imageGraph( std::string filename ){
            loadFromFile(filename);
        }
        
        // compute sequentially
        void
            computeSingle( std::string filename,
                           uint32_t numDocs,
                           retriever const &retriever,
                           uint32_t maxNeighs= 0,
                           double scoreThr= -inf );
        
        // compute in parallel
        void
            computeParallel( std::string filename,
                             uint32_t numDocs,
                             retriever const &retriever,
                             uint32_t maxNeighs= 0,
                             double scoreThr= -inf );
        
        void
            loadFromFile( std::string filename );
        
        imageGraphType graph_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(imageGraph)
        
};

#endif
