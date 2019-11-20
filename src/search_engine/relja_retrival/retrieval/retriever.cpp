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

#include "retriever.h"
#include "util.h"

#include <algorithm>



void
retriever::externalQuery( std::string imageFn, query const &query_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    // TODO cache management
    std::string cacheFn= util::getTempFileName(); // TODO: give dir
    
    externalQuery_computeData( imageFn, query_obj );
    queryExecute( query_obj, queryRes, toReturn );
    
    remove( cacheFn.c_str() );
}



void
retriever::sortResults( std::vector<indScorePair> &queryRes, uint32_t firstN, uint32_t toReturn ){
    //TODO this can be more efficient for toReturn << min(firstN, size) ; stl partial_sort
    if (firstN==0 || firstN>=queryRes.size()) {
        sort( queryRes.begin(), queryRes.end(), compare );
    } else {
        sort( queryRes.begin(), queryRes.begin()+firstN, compare );
    }
    if (toReturn!=0 && toReturn<queryRes.size()){
        queryRes.resize( toReturn );
    }
}



void
retriever::sortResults( std::vector<double> &scores, std::vector<indScorePair> &queryRes, uint32_t toReturn ){
    
    queryRes.clear();
    queryRes.reserve( scores.size() );
    
    uint32_t i= 0;
    for ( std::vector<double>::iterator it= scores.begin(); it!=scores.end(); ++it, ++i) {
        queryRes.push_back( std::make_pair( i, *it ) );
    }
    
    retriever::sortResults( queryRes, 0, toReturn );
    
}
