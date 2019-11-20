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

#include "evaluator_v2.h"

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



evaluatorV2::evaluatorV2( std::string gtPath, std::string dsetName, datasetV2 const *dset ) {
    std::string gtFileName= gtPath+"/rr_format/"+dsetName+".v2bin";
    
    std::string gtFilenamePrefix= "";
    if (dsetName=="Oxford")
        gtFilenamePrefix= "oxc1/";
    
    if (boost::filesystem::exists(gtFileName))
        loadGt( gtFileName );
    else {
        if (dsetName=="Oxford")
            convertOxford(gtPath+"/datasets/Oxford");
        else if (dsetName=="Paris")
            convertOxford(gtPath+"/datasets/Paris", true);
        else if (dsetName=="Holidays")
            convertHolidays(gtPath+"/datasets/Holidays/holidays_images.dat");
        else if (boost::starts_with(dsetName, "SF_landmarks")){
            ASSERT(dset!=NULL);
            convertSF(gtPath+"/datasets/"+dsetName+"/cartoid_groundTruth.txt", *dset);
        } else if (boost::starts_with(dsetName, "Pittsburgh")){
            ASSERT(dset!=NULL);
            convertPitts(gtPath+"/datasets/"+dsetName+"/gt_export.txt", *dset);
        } else
            throw std::runtime_error("Unknown dataset");
        
        saveGt( gtFileName );
    }
    
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
            uint32_t queryDocID= (boost::starts_with(dsetName, "SF_landmarks") || boost::starts_with(dsetName, "Pittsburgh") ) ?
                0 :
                dset->getDocID(gtFilenamePrefix+eq.filename());
            
            if (eq.has_xl()){
                ASSERT(eq.has_xl() && eq.has_xu() && eq.has_yl() && eq.has_yu());
                ASSERT(dsetName!="Holidays");
                queries_.push_back( query(queryDocID, true, "", eq.xl(), eq.xu(), eq.yl(), eq.yu()) );
            } else {
                ASSERT(dsetName!="Oxford");
                if (boost::starts_with(dsetName, "SF_landmarks"))
                    queries_.push_back( query(queryDocID, false,
                        (boost::format("/home/relja/Relja/Data/SF_landmarks/query/%04d.v2bin") % iQuery).str()
                    ) );
                else if (boost::starts_with(dsetName, "Pittsburgh"))
                    queries_.push_back( query(queryDocID, false,
                        (boost::format("/home/relja/Relja/Data/Pittsburgh/query/%05d.v2bin") % iQuery).str()
                    ) );
                else
                    queries_.push_back( query(queryDocID, true, "") );
            }
            
            // positives
            for (int i= 0; i<eq.positives_size(); ++i){
                try {
                    pos_[iQuery].insert( dset->getDocID(gtFilenamePrefix+eq.positives(i)) );
                } catch (std::runtime_error) {
                    // happens because the revisited ground truth for SF form 2014_04 includes additional positives which I didn't add into my SFmini dataset
                    ASSERT(dsetName=="SF_landmarks_2014_04");
                    ASSERT(dset->getNumDoc()<200000); // make sure it is the SFmini version
                }
            }
            // ignores
            for (int i= 0; i<eq.ignores_size(); ++i)
                ign_[iQuery].insert( dset->getDocID(gtFilenamePrefix+eq.ignores(i)) );
        }
        std::cout << "evaluatorV2::evaluatorV2: Generated dataset specific gt in "<< timing::toc(t0) <<" ms\n";
        
        if (dsetName=="Oxford" || dsetName=="False")
            ignoreQuery_= false;
        else if (dsetName=="Holidays")
            ignoreQuery_= true;
        else if (boost::starts_with(dsetName, "SF_landmarks") || boost::starts_with(dsetName, "Pittsburgh"))
            ignoreQuery_= true; // not really important as query is an external image
        else
            throw std::runtime_error("Unknown dataset");
    }
    
}



void
evaluatorV2::saveGt( std::string fileName ) const{
    std::cout<<"evaluatorV2::saveGt to "<<fileName<<"\n";
    protoDbFileBuilder dbBuilder(fileName, "gt");
    for (uint32_t i= 0; i<gt_.size(); ++i)
        dbBuilder.addProto(i, gt_[i]);
}



void
evaluatorV2::loadGt( std::string fileName ){
    std::cout<<"evaluatorV2::loadGt from "<<fileName<<"\n";
    protoDbFile db(fileName);
    gt_.clear();
    gt_.resize(db.numIDs());
    for (uint32_t i= 0; i<gt_.size(); ++i){
        std::vector<rr::evalQuery> temp;
        db.getProtos(i, temp);
        ASSERT(temp.size()==1);
        gt_[i]= temp[0];
    }
}



void
evaluatorV2::convertOxford(std::string gtPath, bool isParis) {
    
    std::cout<<"evaluatorV2::convertOxford\n";
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
    
    ASSERT(gt_.size()==55);
    
}



void
evaluatorV2::convertHolidays(std::string gtFn) {
    
    std::cout<<"evaluatorV2::convertHolidays\n";
    
    gt_.clear();
    uint32_t numLines= 0;
    
    std::ifstream f(gtFn.c_str());
    ASSERT(f.is_open());
    rr::evalQuery *eq= NULL;
    std::string imageFn, currPrefix= "", prefix;
    
    while (f>>imageFn){
        ++numLines;
        ASSERT(boost::ends_with(imageFn, ".jpg"));
        prefix= imageFn.substr(0, imageFn.length()-4-2);
        if (boost::ends_with(imageFn, "00.jpg")){
            // query image
            currPrefix= prefix;
            gt_.resize(gt_.size()+1);
            eq= &(gt_.back());
            eq->set_filename(imageFn);
            eq->set_queryname(currPrefix+"00");
            eq->add_ignores(imageFn);
        } else {
            // result image
            ASSERT( prefix==currPrefix );
            eq->add_positives(imageFn);
        }
    }
    f.close();
    ASSERT(numLines==1491);
    ASSERT(gt_.size()==500);
    
}



void
evaluatorV2::convertSF(std::string gtFn, datasetV2 const &dset) {
    
    std::cout<<"evaluatorV2::convertSF\n";
    
    gt_.clear();
    
    std::ifstream f(gtFn.c_str());
    ASSERT(f.is_open());
    rr::evalQuery *eq= NULL;
    std::string gtLine, imageFn;
    uint32_t queryID= 0;
    
    std::vector<uint64_t> allCartoIDs(dset.getNumDoc());
    std::cout<<"evaluatorV2::convertSF: get all cartoIDs\n";
    for (uint32_t docID= 0; docID<dset.getNumDoc(); ++docID){
        if (docID%50000==0)
            std::cout<<docID<<"\n";
        imageFn= dset.getInternalFn(docID);
        // #PCI_sp_<panorama id>_<latitude>_<longitude>_<building id>_<tile>_<carto id>_<yaw>_<pitch>.jpg
        size_t pos= imageFn.find("PCI_sp_");
        ASSERT(pos!=std::string::npos);
        imageFn= imageFn.substr(pos);
        std::vector<std::string> strList;
        boost::split(strList, imageFn, boost::is_any_of("_"));
        if (strList.size()!=10){
            std::cout<<imageFn<<"\n"<<strList.size()<<"\n";
        }
        ASSERT(strList.size()==10);
        allCartoIDs[docID]= boost::lexical_cast<uint32_t>(strList[7]);
    }
    
    while (std::getline(f, gtLine)){
        boost::algorithm::trim(gtLine);
        
        if (gtLine.length()<1)
            continue;
        std::vector<std::string> strList;
        boost::split(strList, gtLine, boost::is_any_of(" "));
        ASSERT(strList.size()>1);
        uint32_t queryIDtmp= boost::lexical_cast<uint32_t>(strList[0]);
        ASSERT(queryID==queryIDtmp);
        
        if (queryID%50==0)
            std::cout<<"evaluatorV2::convertSF: queryID= "<<queryID<<"\n";
        
        // add query
        gt_.resize(gt_.size()+1);
        eq= &(gt_.back());
        eq->set_filename(strList[0]);
        eq->set_queryname(strList[0]);
        
        for (uint32_t i= 1; i<strList.size(); ++i){
            if (strList[i][0]=='x') continue;
            
            uint32_t cartoID= boost::lexical_cast<uint64_t>(strList[i]);
            
            bool foundOne= false;
            for (uint32_t docID= 0; docID<dset.getNumDoc(); ++docID){
                if (cartoID==allCartoIDs[docID]){
                    eq->add_positives( dset.getInternalFn(docID) );
                    foundOne= true;
                }
            }
            if (!foundOne)
                std::cout<< "Warning !foundOne: "<<gtLine <<" | " <<cartoID <<"\n";
//             ASSERT(foundOne);
        }
        
        ++queryID;
    }
    f.close();
    ASSERT(gt_.size()==803);
    
}



void
evaluatorV2::convertPitts(std::string gtFn, datasetV2 const &dset) {
    
    std::cout<<"evaluatorV2::convertSF\n";
    
    gt_.clear();
    
    std::ifstream f(gtFn.c_str());
    ASSERT(f.is_open());
    rr::evalQuery *eq= NULL;
    std::string gtLine;
    uint32_t queryID= 0, panoramaID;
    
    while (std::getline(f, gtLine)){
        boost::algorithm::trim(gtLine);
        
        if (gtLine.length()<1)
            continue;
        
        if (queryID%50==0)
            std::cout<<"evaluatorV2::convertPitts: queryID= "<<queryID<<"\n";
        
        std::vector<std::string> positives;
        
        std::vector<std::string> strList;
        boost::split(strList, gtLine, boost::is_any_of(" "));
        ASSERT(strList.size()==2);
        uint32_t queryIDtmp= boost::lexical_cast<uint32_t>(strList[0]);
        ASSERT(queryIDtmp==queryID);
        uint32_t numPoss= boost::lexical_cast<uint32_t>(strList[1]);
        
        for (uint32_t iPos= 0; iPos<numPoss; ++iPos){
            f>>panoramaID;
            // add 24 positives per positive panorama
            for (uint32_t iImage= 0; iImage<24; ++iImage)
                positives.push_back( dset.getInternalFn(panoramaID*24 + iImage) );
        }
        
        // add 24 query panoramas
        for (uint32_t iQueryImage= 0; iQueryImage<24; ++iQueryImage){
            
            gt_.resize(gt_.size()+1);
            eq= &(gt_.back());
            eq->set_filename("");
            eq->set_queryname( (boost::format("%05d") % (queryID*24+iQueryImage)).str() );
            
            for (uint32_t i= 0; i<positives.size(); ++i)
                eq->add_positives( positives[i] );
        }
        
        ++queryID;
    }
    
    ASSERT(gt_.size()==24000);
}



class computeAPworker : public queueWorker<evaluatorV2::APresultType> {
    public:
        computeAPworker( evaluatorV2 const *aEvaluatorObj,
                         retriever const &aRetriever_obj )
                : evaluatorObj(aEvaluatorObj),
                  retriever_obj(&aRetriever_obj)
            {}
        
        void operator() ( uint32_t queryID, evaluatorV2::APresultType &result ) const {
            std::vector<double> precision, recall;
            double queryTime;
            double AP= evaluatorObj->computeAP( queryID, *retriever_obj, precision, recall, queryTime );
            result= std::make_pair(AP,queryTime);
        }
        
    private:
        evaluatorV2 const *evaluatorObj;
        retriever const *retriever_obj;
};



class computeAPmanager : public queueManager<evaluatorV2::APresultType> {
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
        
        void operator()( uint32_t queryID, evaluatorV2::APresultType &result ){
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
evaluatorV2::computeMAP(
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
    threadQueue<evaluatorV2::APresultType>::start( nQueries_, computeAPworker_obj, computeAPmanager_obj, numWorkerThreads );
    
    double mAP= computeAPmanager_obj.mAP_ / nQueries_;
    double time= computeAPmanager_obj.time_;
    
    if ( verbose || semiVerbose )
        printf("\n\tmAP= %.10f, time= %.4f s, avgTime= %.4f ms\n\n", mAP, time/1000, time/nQueries_);
    
    return mAP;
    
}



double
evaluatorV2::computeAP(
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
evaluatorV2::computeAPFromResults(
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
evaluatorV2::computeMultiMAP(
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
            printf("%.3d %s %.10f %.2f ms %.4f %.4f\n", queryID, queryName.c_str(), AP, thisTime, mAP/(queryID+1), mAP/nMultiQueries );
        }
    }
    
    mAP/= nMultiQueries;
    
    if (verbose)
        printf("\n\tmAP= %.10f, time= %.4f s, avgTime= %.4f ms\n\n", mAP, time/1000, time/nMultiQueries);
    
    if (deleteAPs_) delete APs;
    
    return mAP;
    
}



double
evaluatorV2::computeAvgRecallAtN(
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
evaluatorV2::computeRecallAtN(
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
