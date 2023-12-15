/*
  ==== Author:

  Relja Arandjelovic (relja@robots.ox.ac.uk)
  Visual Geometry Group,
  Department of Engineering Science
  University of Oxford


Updates:
12-Jun-2021 : simplified code for evaluation on Oxford-Buildings dataset

*/

#include "evaluator_oxford.h"

#include <fstream>
#include <set>
#include <string.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "timing.h"
#include "proto_db_file.h"
#include "util.h"



evaluator_oxford::evaluator_oxford( std::string gtPath, datasetV2 const *dset ) {
  convertOxford(gtPath);
  nQueries_= gt_.size();

  if (dset!=NULL){
    double t0= timing::tic();
    // populate with dataset specific IDs
    queries_.reserve(nQueries_);
    pos_.resize(nQueries_);
    ign_.resize(nQueries_);

    for (uint32_t iQuery= 0; iQuery<nQueries_; ++iQuery){
      rr::evalQuery const &eq= gt_[iQuery];
      // query
      uint32_t queryDocID= dset->getDocID(eq.filename());
      std::cout << "Processing [" << queryDocID << "] "
                << eq.filename() << std::endl;

      if (eq.has_xl()){
        ASSERT(eq.has_xl() && eq.has_xu() && eq.has_yl() && eq.has_yu());
        queries_.push_back( query(queryDocID, true, "", eq.xl(), eq.xu(), eq.yl(), eq.yu()) );
      }

      // positives
      for (int i= 0; i<eq.positives_size(); ++i){
        pos_[iQuery].insert( dset->getDocID(eq.positives(i)) );
      }
      // ignores
      for (int i= 0; i<eq.ignores_size(); ++i) {
        ign_[iQuery].insert( dset->getDocID(eq.ignores(i)) );
      }
    }
    std::cout << "evaluator_oxford::evaluator_oxford: Generated dataset specific gt in "<< timing::toc(t0) <<" ms\n";

    ignoreQuery_= false;
  }
}

void
evaluator_oxford::convertOxford(std::string gtPath, bool isParis) {

  boost::filesystem::path gtDir(gtPath);
  boost::filesystem::directory_iterator end;

  gt_.clear();

  std::set<std::string> queries;

  // get all queries
  for( boost::filesystem::directory_iterator it(gtDir);
       it != end;
       ++it){
    std::string fn= it->path().filename().string();
    if (boost::algorithm::ends_with(fn, "_query.txt"))
      queries.insert( std::string(fn.begin(), fn.end()-10) );
  }

  // go one by one in sorted order
  for (std::set<std::string>::const_iterator it= queries.begin();
       it!=queries.end();
       ++it){
    std::string queryName= *it;

    rr::evalQuery eq;
    eq.set_queryname(queryName);

    // read query info
    {
      // oxc1_all_souls_000013 136.5 34.1 648.5 955.7
      std::ifstream f( (gtPath+'/'+queryName+"_query.txt").c_str() );
      ASSERT(f.is_open());
      std::string queryImage;
      float xl, yl, xu, yu;
      f>>queryImage>>xl>>yl>>xu>>yu;
      eq.set_filename( std::string(queryImage.begin() + (isParis?0:5), queryImage.end()) +".jpg" );
      eq.set_xl(xl);
      eq.set_xu(xu);
      eq.set_yl(yl);
      eq.set_yu(yu);
      f.close();
    }

    // read positives and ignores
    for (int i= 0; i<3; ++i){
      std::string fn= gtPath+'/'+queryName+'_';
      if (i==0)
        fn+= "good.txt";
      else if (i==1)
        fn+= "ok.txt";
      else
        fn+= "junk.txt";
      std::ifstream f(fn.c_str());
      ASSERT(f.is_open());
      std::string imageFn;
      while (f>>imageFn)
        if (i==2)
          eq.add_ignores(imageFn+".jpg");
        else
          eq.add_positives(imageFn+".jpg");
      f.close();
    }
    gt_.push_back(eq);
  }
}


class computeAPworker : public queueWorker<evaluator_oxford::APresultType> {
public:
  computeAPworker( evaluator_oxford const *aEvaluatorObj,
                   retriever const &aRetriever_obj )
    : evaluatorObj(aEvaluatorObj),
      retriever_obj(&aRetriever_obj)
  {}

  void operator() ( uint32_t queryID, evaluator_oxford::APresultType &result ) const {
    std::vector<double> precision, recall;
    double queryTime;
    double AP= evaluatorObj->computeAP( queryID, *retriever_obj, precision, recall, queryTime );
    result= std::make_pair(AP,queryTime);
  }

private:
  evaluator_oxford const *evaluatorObj;
  retriever const *retriever_obj;
};



class computeAPmanager : public queueManager<evaluator_oxford::APresultType> {
public:
  computeAPmanager( std::vector<double> *APs,
                    uint32_t nQueries,
                    std::vector<rr::evalQuery> const &gt,
                    bool verbose,
                    bool semiVerbose )
    : APs_(APs),
      deleteAPs_(false),
      nQueries_(nQueries),
      gt_(&gt),
      verbose_(verbose),
      semiVerbose_(semiVerbose),
      mAP_(0.0),
      time_(0.0),
      cumAP_(0.0),
      times_(nQueries,-1.0),
      nextToPrint_(0) {
    if (APs==NULL){
      deleteAPs_= true;
      APs_= new std::vector<double>;
    }
    APs_->clear();
    APs_->resize(nQueries,0);
  }

  ~computeAPmanager(){
    if (deleteAPs_)
      delete APs_;
  }

  void operator()( uint32_t queryID, evaluator_oxford::APresultType &result ){
    double AP= result.first, queryTime= result.second;
    mAP_+= AP;
    time_+= queryTime;
    APs_->at(queryID)= AP;
    times_[queryID]= queryTime;
    if ( (verbose_ || semiVerbose_) && nextToPrint_==queryID){
      for (; nextToPrint_<nQueries_ && times_[nextToPrint_]>-0.5; ++nextToPrint_){
        cumAP_+= APs_->at(nextToPrint_);
        if (verbose_ || (semiVerbose_ && nextToPrint_%5==0)){
          std::string queryName= gt_->at(nextToPrint_).has_queryname() ?
            gt_->at(nextToPrint_).queryname() :
            gt_->at(nextToPrint_).filename();
          printf("%.3d %s %.10f %.2f ms %.4f %.4f\n", nextToPrint_, queryName.c_str(), APs_->at(nextToPrint_), times_[nextToPrint_], cumAP_/(nextToPrint_+1), cumAP_/nQueries_ );
        }
      }
    }
  }
  std::vector<double> *APs_;
  bool deleteAPs_;
  uint32_t nQueries_;
  std::vector<rr::evalQuery> const *gt_;
  bool verbose_, semiVerbose_;
  double mAP_, time_, cumAP_;
  std::vector<double> times_;
  uint32_t nextToPrint_;
};



double
evaluator_oxford::computeMAP(
                        retriever const &retriever_obj,
                        std::vector<double> *APs,
                        bool verbose,
                        bool semiVerbose,
                        uint32_t numWorkerThreads) const {

  semiVerbose= semiVerbose && !verbose;

  if ( verbose || semiVerbose )
    printf("i query AP time mAP_proj mAP_sofar\n\n");

  computeAPworker computeAPworker_obj( this, retriever_obj );
  computeAPmanager computeAPmanager_obj( APs, nQueries_, gt_, verbose, semiVerbose );
  threadQueue<evaluator_oxford::APresultType>::start( nQueries_, computeAPworker_obj, computeAPmanager_obj, numWorkerThreads );

  double mAP= computeAPmanager_obj.mAP_ / nQueries_;
  double time= computeAPmanager_obj.time_;

  if ( verbose || semiVerbose )
    printf("\n\tmAP= %.10f, time= %.4f s, avgTime= %.4f ms\n\n", mAP, time/1000, time/nQueries_);

  return mAP;

}



double
evaluator_oxford::computeAP(
                       uint32_t queryID,
                       retriever const &retriever_obj,
                       std::vector<double> &precision,
                       std::vector<double> &recall,
                       double &time ) const {

  // query
  std::vector<indScorePair> queryRes;
  time= timing::tic();
  retriever_obj.queryExecute( queries_[queryID], queryRes );
  time= timing::toc( time );

  // compute AP
  return computeAPFromResults(
                              queryRes,
                              queries_[queryID].isInternal && ignoreQuery_, queryID,
                              pos_[queryID], ign_[queryID],
                              precision, recall);

}



double
evaluator_oxford::computeAPFromResults(
                                  std::vector<indScorePair> const &queryRes,
                                  bool isInternalAndIgnoreQuery,
                                  uint32_t queryDocID,
                                  std::set<uint32_t> const &pos,
                                  std::set<uint32_t> const &ign,
                                  std::vector<double> &precision,
                                  std::vector<double> &recall ) {

  uint32_t numPos= pos.size();

  precision.clear();
  precision.reserve( numPos );
  recall.clear();
  recall.reserve( numPos );

  uint32_t docID;

  uint32_t posSoFar= 0, nonIgnSoFar= 0;
  double AP= 0, currRec= 0, prevRec= 0, currPrec= 0;

  std::set<uint32_t> prevDocs;

  for (std::vector< std::pair<uint32_t,double> >::const_iterator it= queryRes.begin(); it!=queryRes.end(); ++it){

    docID= it->first;

    if ( prevDocs.count( docID ) ){
      // already encountered so ignore (so that for example returning a positive 100 times doesn't boost results)
      continue;
    } else {
      // add to list of encountered
      prevDocs.insert( docID );
    }

    if ( (isInternalAndIgnoreQuery && docID==queryDocID) ||
         ign.count( docID ) )
      continue;

    ++nonIgnSoFar;

    if ( pos.count( docID ) ) {

      ++posSoFar;
      currPrec= static_cast<double>(posSoFar)/nonIgnSoFar;
      currRec= static_cast<double>(posSoFar)/numPos;
      precision.push_back( currPrec );
      recall.push_back( currRec );

      AP+= (currRec-prevRec)*currPrec;
      prevRec= currRec;

    }

  }

  return AP;

}



double
evaluator_oxford::computeMultiMAP(
                             multiQuery const &multiQuery_obj,
                             std::vector<double> *APs,
                             bool verbose) const {

  std::vector<double> precision, recall;

  if (verbose)
    printf("i query AP time mAP_proj mAP_sofar\n\n");

  uint32_t nMultiQueries= nQueries_/5;
  ASSERT(nMultiQueries==11); // right now, this only works for Oxford/Paris as the evaluation setup doesn't support multiple queries (not too hard to enable but it would be an overkill..)
  ASSERT(ignoreQuery_==false);

  bool deleteAPs_= false;
  if (APs==NULL){
    deleteAPs_= true;
    APs= new std::vector<double>;
  }
  APs->clear();
  APs->resize(nMultiQueries, 0);

  double time= 0;
  double mAP= 0;

  for (uint32_t queryID= 0; queryID<nMultiQueries; ++queryID){

    uint32_t singleQueryID= queryID*5;

    // combine query images

    std::vector<query> multiQuerySpec;
    for (uint32_t iQueryImage= singleQueryID; iQueryImage<singleQueryID+5; ++iQueryImage)
      multiQuerySpec.push_back( queries_[iQueryImage] );

    // query
    std::vector<indScorePair> queryRes;

    double thisTime= timing::tic();
    multiQuery_obj.queryExecute( multiQuerySpec, queryRes );
    thisTime= timing::toc( thisTime );

    // compute AP
    double AP= computeAPFromResults(
                                    queryRes, false, 0,
                                    pos_[singleQueryID], ign_[singleQueryID],
                                    precision, recall);

    APs->at(queryID)= AP;
    mAP+= AP;
    time+= thisTime;

    if (verbose){
      std::string queryName= gt_[singleQueryID].has_queryname() ?
        gt_[singleQueryID].queryname() :
        gt_[singleQueryID].filename();
      printf("%.3d %s AP=%.10f %.2f ms %.4f %.4f\n", queryID, queryName.c_str(), AP, thisTime, mAP/(queryID+1), mAP/nMultiQueries );
    }
  }

  mAP/= nMultiQueries;

  if (verbose)
    printf("\n\tmAP= %.10f, time= %.4f s, avgTime= %.4f ms\n\n", mAP, time/1000, time/nMultiQueries);

  if (deleteAPs_) delete APs;

  return mAP;

}



double
evaluator_oxford::computeAvgRecallAtN(
                                 retriever const &retriever_obj,
                                 uint32_t N,
                                 uint32_t printN,
                                 std::vector<double> *recall,
                                 bool verbose,
                                 bool semiVerbose ) const {

  ASSERT(N>0);
  semiVerbose= semiVerbose && !verbose;

  std::vector<double> *recallNew= NULL;
  if (recall==NULL){
    recallNew= new std::vector<double>(N, 0);
    recall= recallNew;
  }

  if ( verbose || semiVerbose )
    printf("i query rec time rec_proj\n\n");

  uint32_t printStep= nQueries_ / 11;
  if (printN==0)
    printN= N;
  double const &cumRecAtPN= (*recall)[printN-1];
  double time= 0;

  for (uint32_t queryID= 0; queryID < nQueries_; ++queryID ){
    std::vector<bool> thisRecall;
    double queryTime;
    computeRecallAtN( queryID, retriever_obj, N, thisRecall, queryTime );
    time+= queryTime;

    for (uint32_t i= 0; i<N; ++i)
      (*recall)[i]+= thisRecall[i];

    if (verbose || (semiVerbose && (queryID%printStep==0 || queryID+1==nQueries_))){
      std::string queryName= gt_[queryID].has_queryname() ?
        gt_[queryID].queryname() :
        gt_[queryID].filename();

      printf("%.3d %s ",
             queryID,
             queryName.c_str());
      for (uint32_t j= 0; j<5 && j<N; ++j)
        printf("%.1d", static_cast<uint8_t>(thisRecall[j]));
      printf(" %.1d %.2f ms %.4f\n",
             static_cast<uint8_t>(thisRecall[printN-1]),
             queryTime,
             cumRecAtPN / (queryID+1)
             );

    }
  }

  double res= cumRecAtPN/nQueries_;
  if ( verbose || semiVerbose )
    printf("\n\trec@%d= %.4f, time= %.4f s, avgTime= %.4f ms\n", printN, res, time/1000, time/nQueries_);

  for (uint32_t i= 0; i<N; ++i){
    (*recall)[i]/= nQueries_;
    if ( (verbose || semiVerbose) && (i<5 || (i+1)%5==0) )
      printf("%d %.4f\n", i+1, (*recall)[i]);
  }

  if (recallNew!=NULL)
    delete recallNew;

  return res;
}



void
evaluator_oxford::computeRecallAtN(
                              uint32_t queryID,
                              retriever const &retriever_obj,
                              uint32_t N,
                              std::vector<bool> &recall,
                              double &time ) const {

  recall.clear();
  recall.resize(N, false);

  uint32_t docID;

  uint32_t nonIgnSoFar= 0;

  // query
  std::vector<indScorePair> queryRes;
  time= timing::tic();
  retriever_obj.queryExecute( queries_[queryID], queryRes, N+ign_[queryID].size() );
  time= timing::toc( time );

  for (std::vector< std::pair<uint32_t,double> >::iterator it= queryRes.begin();
       it!=queryRes.end() && nonIgnSoFar<N;
       ++it){

    docID= it->first;

    if ( (ignoreQuery_ && queries_[queryID].isInternal && docID==queries_[queryID].docID) ||
         ign_[queryID].count( docID ) )
      continue;

    if ( pos_[queryID].count( docID ) ) {

      // fill all with true
      for (; nonIgnSoFar<N; ++nonIgnSoFar)
        recall[nonIgnSoFar]= true;

    } else
      ++nonIgnSoFar;

  }

}
