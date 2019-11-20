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

#ifndef _WEIGHTER_V2_H_
#define _WEIGHTER_V2_H_

#include <stdint.h>
#include <vector>

#include "index_entry.pb.h"
#include "uniq_entries.h"



namespace weighterV2 {

// assumes sorted queryRep.id
void
    queryExecute( rr::indexEntry const &queryRep,
                  ueIterator *ueIter,
                  std::vector<double> const &idf,
                  std::vector<double> const &docL2,
                  std::vector<double> &scores,
                  double defaultScore= 0.0 );

// queryRep.id should be sorted for efficiency
void
    queryExecuteWGC( rr::indexEntry const &queryRep,
                     ueIterator *ueIter,
                     std::vector<double> const &idf,
                     std::vector<double> const &docL2,
                     std::vector<double> &scores,
                     uint16_t numScales,
                     double defaultScore= 0.0 );

};


#endif
