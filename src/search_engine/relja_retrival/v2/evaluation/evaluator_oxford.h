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

#ifndef _EVALUATOR_OXFORD_H_
#define _EVALUATOR_OXFORD_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "dataset_v2.h"
#include "eval_query.pb.h"
#include "query.h"
#include "retriever.h"
#include "multi_query.h"
#include "thread_queue.h"


class evaluator_oxford {

    public:
        evaluator_oxford( std::string gtPath, datasetV2 const *dset= NULL );

        typedef std::pair<double,double> APresultType;

        double
            computeMAP( retriever const &retriever_obj,
                        std::vector<double> *APs= NULL,
                        bool verbose= false,
                        bool semiVerbose= false,
                        uint32_t numWorkerThreads= 0) const;

        double
            computeAP( uint32_t queryID,
                       retriever const &retriever_obj,
                       std::vector<double> &precision,
                       std::vector<double> &recall,
                       double &time ) const;

        double
            computeMultiMAP( multiQuery const &multiQuery_obj,
                             std::vector<double> *APs= NULL,
                             bool verbose= false) const;

        double
            computeAvgRecallAtN( retriever const &retriever_obj,
                                 uint32_t N,
                                 uint32_t printN= 0,
                                 std::vector<double> *recall= NULL,
                                 bool verbose= false,
                                 bool semiVerbose= false ) const;

        void
            computeRecallAtN( uint32_t queryID,
                              retriever const &retriever_obj,
                              uint32_t N,
                              std::vector<bool> &recall,
                              double &time ) const;

        uint32_t nQueries_;
        std::vector<query> queries_;
        std::vector< std::set<uint32_t> > pos_, ign_;
        bool ignoreQuery_;
        std::vector<rr::evalQuery> gt_;

    private:

        void
            convertOxford( std::string gtPath, bool isParis= false );

        static double
            computeAPFromResults(
                std::vector<indScorePair> const &queryRes,
                bool isInternalAndIgnoreQuery,
                uint32_t queryDocID, // only needed if above is true
                std::set<uint32_t> const &pos,
                std::set<uint32_t> const &ign,
                std::vector<double> &precision,
                std::vector<double> &recall);

    private:
        DISALLOW_COPY_AND_ASSIGN(evaluator_oxford)

};

#endif
