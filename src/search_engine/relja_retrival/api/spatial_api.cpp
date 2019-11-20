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

#include "spatial_api.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <string>

#include <boost/format.hpp>

#include "homography.h"
#include "ellipse.h"

#ifdef RR_REGISTER
#include "register_images.h"
#endif




void
API::returnResults( std::vector<indScorePair> const &queryRes, std::map<uint32_t,homography> const *Hs, uint32_t startFrom, uint32_t numberToReturn, std::string &output ){
    
    output+= ( boost::format("<results size=\"%d\">") % queryRes.size() ).str();
    
    double h[9];
    uint32_t docID;
    // for recording score
    //double siftScore;
    std::string Hstr;

    // for recording results
    //std::ofstream f;
    //f.open("/mnt/data2/Yujie/data/BBC/results.txt",std::ios::app);

    for (uint32_t iRes= startFrom; (iRes < queryRes.size()) && (iRes < startFrom+numberToReturn); ++iRes){
        docID= queryRes[iRes].first;
        if ( Hs!=NULL && Hs->count( docID ) ){
            Hs->find( docID )->second.exportToDoubleArray( h );
            Hstr= (boost::format( "H=\"%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\"" )
                   % h[0]%h[1]%h[2]%h[3]%h[4]%h[5]%h[6]%h[7]%h[8] ).str();
        }
        else
            Hstr= "";
        output+= (boost::format( "<result rank=\"%d\" docID=\"%d\" score=\"%.4f\" %s/>" )
                  % iRes
                  % docID
                  % queryRes[iRes].second
                  % Hstr
                  ).str();
	// for recording the score
	//siftScore= queryRes[iRes].second;

	//f<<docID<<'\n';
    }
    
    output+= "</results>";

    
    //std::ofstream fscore;
    //fscore.open("/mnt/data2/Yujie/data/BBC/results_score.txt",std::ios::app);

    
    //fscore<<siftScore<<'\n';
    //f.close();
    //fscore.close();
}



void
API::queryExecute( query &query_obj, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const {
    
    std::vector<indScorePair> queryRes;
    std::map<uint32_t,homography> Hs;
    spatialRetriever_obj->spatialQuery( query_obj, queryRes, Hs, startFrom+numberToReturn );
    API::returnResults(queryRes, &Hs, startFrom, numberToReturn, output);
    
}



void
API::multipleQueries( std::vector<query> const &query_objs, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const {
    
    if (multiQuery_obj!=NULL){
        
        std::vector<indScorePair> queryRes;
        multiQuery_obj->queryExecute( query_objs, queryRes, startFrom+numberToReturn );
        API::returnResults(queryRes, NULL, startFrom, numberToReturn, output);
        
    } else {
        
        std::cerr << "API doesn't have multiQuery_obj, so cannot execute request\n";
        
    }
    
}



void
API::processImage( std::string imageFn, std::string compDataFn, std::string &output ) const {
    
    query queryObj(0, false, compDataFn);
    spatialRetriever_obj->externalQuery_computeData(imageFn, queryObj);
    
}



void
API::getMatches( query &query_obj, uint32_t docID2, std::string &output ) const {
    
    homography H;
    std::vector< std::pair<ellipse,ellipse> > matches;
    
    spatialRetriever_obj->getMatches( query_obj, docID2, H, matches );
    
    API::returnMatches(matches, output);
    
}



void
API::getPutativeMatches( query &query_obj, uint32_t docID2, std::string &output ) const {
    
    std::vector< std::pair<ellipse,ellipse> > matches;
    
    spatialRetriever_obj->getPutativeMatches( query_obj, docID2, matches );
    
    API::returnMatches(matches, output);
    
}



void
API::returnMatches( std::vector< std::pair<ellipse,ellipse> > &matches, std::string &output ){
    
    uint32_t numInliers= matches.size();
    std::string el1, el2;
    ellipse el;
    
    output+= ( boost::format("<matches size=\"%d\">") % numInliers ).str();
    
    for (uint32_t iMatch=0; iMatch < numInliers; ++iMatch){
        
        el= matches[iMatch].first;
        el1= ( boost::format("%.2f,%.2f,%.8f,%.8f,%.8f")
               % el.x % el.y % el.a % el.b % el.c ).str();
        
        el= matches[iMatch].second;
        el2= ( boost::format("%.2f,%.2f,%.8f,%.8f,%.8f")
               % el.x % el.y % el.a % el.b % el.c ).str();
        
        output+= ( boost::format("<match el1=\"%s\" el2=\"%s\"/>")
                   % el1 % el2
                   ).str();
        
    }
    
    output+= "</matches>";
    
}




std::string
API::getReply( boost::property_tree::ptree &pt, std::string const &request ) const {
    
    std::string reply;
    
    if ( pt.count("internalQuery") ){
        
        uint32_t docID= pt.get<uint32_t>("internalQuery.docID");
        
        query query_obj(
            docID,
            true,
            "",
            pt.get("internalQuery.xl", -inf),
            pt.get("internalQuery.xu",  inf),
            pt.get("internalQuery.yl", -inf),
            pt.get("internalQuery.yu",  inf)
            );
        
        std::cout<< pt.get<uint>("internalQuery.numberToReturn") <<"\n";
        queryExecute( query_obj,
                      pt.get("internalQuery.startFrom",0),
                      pt.get("internalQuery.numberToReturn",20),
                      reply );
    
    } else if ( pt.count("externalQuery") ) {
        
        // precomputed features
        std::string wordFn= pt.get<std::string>("externalQuery.wordFn");
        
        query query_obj(
            0,
            false,
            wordFn,
            pt.get("externalQuery.xl", -inf),
            pt.get("externalQuery.xu",  inf),
            pt.get("externalQuery.yl", -inf),
            pt.get("externalQuery.yu",  inf)
            );
        
        queryExecute( query_obj,
                      pt.get("externalQuery.startFrom",0),
                      pt.get("externalQuery.numberToReturn",20),
                      reply );
       
   } else if ( pt.count("multiQuery") ) {
        
        uint32_t startFrom= pt.get("multiQuery.startFrom",0);
        uint32_t numberToReturn= pt.get("multiQuery.numberToReturn",20);
        uint32_t numQ= pt.get<uint32_t>("multiQuery.numQ");
       
        // queries
        
        std::vector<query> query_objs;
        query_objs.reserve(numQ);
        
        for (uint32_t i=0; i<numQ; ++i){
            
            double xl= pt.get<double>( (boost::format("multiQuery.xl%d") % i).str(), -inf);
            double xu= pt.get<double>( (boost::format("multiQuery.xu%d") % i).str(),  inf);
            double yl= pt.get<double>( (boost::format("multiQuery.yl%d") % i).str(), -inf);
            double yu= pt.get<double>( (boost::format("multiQuery.yu%d") % i).str(),  inf);
            
            boost::optional<uint32_t> docID_opt= pt.get_optional<uint32_t>( ( boost::format("multiQuery.docID%d") % i).str() );
            
            if (docID_opt.is_initialized()) {
                // adding internal query
                query_objs.push_back( query(*docID_opt, true, "", xl, xu, yl, yu) );
            } else {
                // adding external query
                std::string wordFn= pt.get<std::string>( ( boost::format("multiQuery.wordFn%d") % i).str() );
                query_objs.push_back( query(0, false, wordFn, xl, xu, yl, yu) );
            }
            
        }
        
        multipleQueries( query_objs, startFrom, numberToReturn, reply );
        
    } else if ( pt.count("getPutativeInternalMatches") ) {
        
        uint32_t docID1= pt.get<uint32_t>("getPutativeInternalMatches.docID1");
        uint32_t docID2= pt.get<uint32_t>("getPutativeInternalMatches.docID2");
        
        query query_obj(
            docID1,
            true,
            "",
            pt.get("getPutativeInternalMatches.xl", -inf),
            pt.get("getPutativeInternalMatches.xu",  inf),
            pt.get("getPutativeInternalMatches.yl", -inf),
            pt.get("getPutativeInternalMatches.yu",  inf)
            );
        getPutativeMatches( query_obj, docID2, reply );
        
        
        
    } else if ( pt.count("getInternalMatches") ) {
        
        uint32_t docID1= pt.get<uint32_t>("getInternalMatches.docID1");
        uint32_t docID2= pt.get<uint32_t>("getInternalMatches.docID2");
        
        query query_obj(
            docID1,
            true,
            "",
            pt.get("getInternalMatches.xl", -inf),
            pt.get("getInternalMatches.xu",  inf),
            pt.get("getInternalMatches.yl", -inf),
            pt.get("getInternalMatches.yu",  inf)
            );
        getMatches( query_obj, docID2, reply );
        
        
        
    } else if ( pt.count("getExternalMatches") ) {
        
        std::string wordFn= pt.get<std::string>("getExternalMatches.wordFn1");
        uint32_t docID2= pt.get<uint32_t>("getExternalMatches.docID2");
        
        query query_obj(
            0,
            false,
            wordFn,
            pt.get("getExternalMatches.xl", -inf),
            pt.get("getExternalMatches.xu",  inf),
            pt.get("getExternalMatches.yl", -inf),
            pt.get("getExternalMatches.yu",  inf)
            );
        getMatches( query_obj, docID2, reply );
        
        
        
    } else if ( pt.count("getPutativeExternalMatches") ) {
        
        std::string wordFn= pt.get<std::string>("getPutativeExternalMatches.wordFn1");
        uint32_t docID2= pt.get<uint32_t>("getPutativeExternalMatches.docID2");
        
        query query_obj(
            0,
            false,
            wordFn,
            pt.get("getPutativeExternalMatches.xl", -inf),
            pt.get("getPutativeExternalMatches.xu",  inf),
            pt.get("getPutativeExternalMatches.yl", -inf),
            pt.get("getPutativeExternalMatches.yu",  inf)
            );
        getPutativeMatches( query_obj, docID2, reply );
        
        
        
    } else if ( pt.count("processImage") ) {
        
        std::string imageFn= pt.get<std::string>("processImage.imageFn");
        std::string compDataFn= pt.get<std::string>("processImage.compDataFn");
        processImage( imageFn, compDataFn, reply );
        
        
    } else if ( pt.count("register") ) {
        
        #ifdef RR_REGISTER
        
        uint32_t docID1=0;
        
        boost::optional<std::string> wordFn1_opt= pt.get_optional<std::string>("register.wordFn1");
        bool isInternal= !wordFn1_opt.is_initialized();
        
        std::string inFn1= "";
        
        if (isInternal)
            docID1= pt.get<uint32_t>("register.docID1");
        else
            inFn1= pt.get<std::string>("register.inFn1");
        
        uint32_t docID2= pt.get<uint32_t>("register.docID2");
        
        query query_obj(
            docID1,
            isInternal,
            isInternal ? "" : *wordFn1_opt,
            pt.get("register.xl", -inf),
            pt.get("register.xu",  inf),
            pt.get("register.yl", -inf),
            pt.get("register.yu",  inf)
            );
        
        std::string outFn1= pt.get<std::string>("register.outFn1");
        std::string outFn2= pt.get<std::string>("register.outFn2");
        std::string outFn2t= pt.get<std::string>("register.outFn2t");
        
        std::string fullSizeFn1= pt.get<std::string>("register.fullSizeFn1");
        std::string fullSizeFn2= pt.get<std::string>("register.fullSizeFn2");
        
        registerImages::registerFromQuery( query_obj, inFn1.c_str(), docID2, *dataset_, *spatialRetriever_obj, outFn1.c_str(), outFn2.c_str(), outFn2t.c_str(), fullSizeFn1.c_str(), fullSizeFn2.c_str() );
        
        #else
        
        std::cerr << "API compiled without registration support, cannot execute request: "<< request <<"\n";
        
        #endif
        
        
    } else {
        
        std::cerr << "Unrecognized request: "<< request <<"\n";
        
    }
    
    return reply;
}
