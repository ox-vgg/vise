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

#include "single_api.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <string>

#include <boost/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "dataset_v2.h"
#include "python_cfg_to_ini.h"
#include "util.h"
#include "product_quant.h"
#include "nn_evaluator.h"
#include "coarse_residual.h"
#include "index_with_data_file.h"
#include "index_with_data_file_fixed1.h"
#include "nn_single_retriever.h"
#include "util.h"
#include "timing.h"
#include "iidx_with_data_builder.h"
#include "desc_from_flat_file.h"
#include "desc_file_rotate.h"

#include "nn_raw_single_retriever.h"



void
singleAPI::returnResults( std::vector<indScorePair> const &queryRes, uint32_t startFrom, uint32_t numberToReturn, std::string &output ){
    
    output+= ( boost::format("<results size=\"%d\">") % queryRes.size() ).str();
    
    uint32_t docID;
    std::string Hstr;
    for (uint32_t iRes= startFrom; (iRes < queryRes.size()) && (iRes < startFrom+numberToReturn); ++iRes){
        docID= queryRes[iRes].first;
        output+= (boost::format( "<result rank=\"%d\" docID=\"%d\" score=\"%.4f\"/>" )
                  % iRes
                  % docID
                  % queryRes[iRes].second
                  ).str();
    }
    
    output+= "</results>";
    
}



void
singleAPI::queryExecute( query &query_obj, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const {
    
    std::vector<indScorePair> queryRes;
    retriever_obj->queryExecute( query_obj, queryRes, startFrom+numberToReturn );
    singleAPI::returnResults(queryRes, startFrom, numberToReturn, output);
    
}



void
singleAPI::multipleQueries( std::vector<query> const &queries, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const {
    
    if (mq==NULL){
        std::cout<<"haven't given me a multiQuery object!\n";
        return;
    }
    std::vector<indScorePair> queryRes;
    mq->queryExecute( queries, queryRes, startFrom+numberToReturn );
    singleAPI::returnResults(queryRes, startFrom, numberToReturn, output);
    
}



std::string
singleAPI::getReply( boost::property_tree::ptree &pt, std::string const &request ) const {
    
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
       
        
    
    } else if ( pt.count("externalQuery") ){
        
        // precomputed features
        std::string featFn= pt.get<std::string>("externalQuery.featFn");
        
        query query_obj(
            0,
            false,
            featFn,
            pt.get("externalQuery.xl", -inf),
            pt.get("externalQuery.xu",  inf),
            pt.get("externalQuery.yl", -inf),
            pt.get("externalQuery.yu",  inf)
            );
        
        std::cout<< pt.get<uint>("externalQuery.numberToReturn") <<"\n";
        queryExecute( query_obj,
                      pt.get("externalQuery.startFrom",0),
                      pt.get("externalQuery.numberToReturn",20),
                      reply );
        
    
    } else if ( pt.count("multiQuery") ){
        
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
                std::string wordFn= pt.get<std::string>( ( boost::format("multiQuery.featFn%d") % i).str() );
                query_objs.push_back( query(0, false, wordFn, xl, xu, yl, yu) );
            }
            
        }
        
        multipleQueries( query_objs, startFrom, numberToReturn, reply );
    
    } else {
        
        std::cerr << "Unrecognized request: "<< request <<"\n";
        
    }
    
    return reply;
}



int main(int argc, char* argv[]){
    
    
    int APIport= 35200;
    if (argc>1){
        APIport= atoi(argv[1]);
    }
    
    std::string dsetname= "BBCc_vlad";
    if (argc>2){
        dsetname= argv[2];
    }
    
    std::string configFn= "~/Relja/Code/relja_retrieval/src/ui/web/config/config.cfg";
    if (argc>3 && argv[3][0]!='_'){
        configFn= argv[3];
    }
    configFn= util::expandUser(configFn);
    std::string tempConfigFn= util::getTempFileName();
    pythonCfgToIni( configFn, tempConfigFn );
    
    
    uint32_t vladDim= 256;
    bool doApprox= true;
    if (argc>4){
        if (argv[4][0]=='5'){ vladDim= 512; doApprox= false; }
        else if (argv[4][0]=='2'){ vladDim= 256; doApprox= false; }
        else if (argv[4][0]=='1'){ vladDim= 128; doApprox= false; }
    }
    
    uint32_t numDiv= 1;
//     uint32_t numDiv= 3;
    
    
    std::cout<<"APIport= "<<APIport<<"\n";
    std::cout<<"\t vladDim= "<<vladDim<<"\n";
    std::cout<<"\t doApprox= "<<doApprox<<"\n";
    
    
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(tempConfigFn, pt);
    
    remove(tempConfigFn.c_str());
    
    std::string const docMapFindPath= pt.get<std::string>( dsetname+".docMapFindPath", "" );
    boost::optional<std::string> const docMapReplacePath= pt.get_optional<std::string>( dsetname+".docMapReplacePath" );
    std::string databasePath= pt.get<std::string>( dsetname+".databasePath", "");
    ASSERT( !(docMapReplacePath.is_initialized() && databasePath.length()>0) );
    if (docMapReplacePath.is_initialized())
        databasePath= *docMapReplacePath;
    std::string dsetFn= util::expandUser(pt.get<std::string>( dsetname+".dsetFn" ));
    datasetV2 dset( dsetFn, databasePath, docMapFindPath );
    
    if (doApprox){
        
        uint32_t nCoarseK= 8192;
        uint32_t nSubQuant= 32;
        uint32_t subQuantK= 4096;
        uint32_t w= 128;
        
        std::string coarseClstFn= util::expandUser((boost::format("~/Relja/Data/BBC/BBCb/vlad/clst_BBCb_test_hesaff_rootsift_k256_a0_norm3_dim%d_%d_43.e3bin") % vladDim % nCoarseK ).str());
        std::vector<std::string> clstFns(nSubQuant);
        for (unsigned i= 0; i<nSubQuant; ++i)
            clstFns[i]= util::expandUser((boost::format("~/Relja/Data/BBC/BBCb/vlad/pq/clst_BBCb_test_hesaff_rootsift_k256_a0_norm3_dim%d_ns%d_k%d_43_res_%d_43_pq%02d.e3bin") % vladDim % nSubQuant % subQuantK % nCoarseK % i ).str());
        
        productQuant pq(clstFns, true);
        
        std::string numDiv_str= ( numDiv==1 ? "" : (boost::format("_%d") % numDiv).str() );
        
        // read fidx/desc filenames from the config
        
        std::string fidxFnsString= pt.get<std::string>( dsetname+".fidxFn" );
        std::vector<std::string> fidxFns_template;
        boost::split(fidxFns_template,
                     fidxFnsString,
                     boost::is_any_of(" ,\n,\t"),
                     boost::token_compress_on);
        
        boost::optional<std::string> descFnsString= pt.get_optional<std::string>( dsetname+".descFn" );
        std::vector<std::string> descFns;
        if (descFnsString.is_initialized()){
            boost::split(descFns,
                         *descFnsString,
                         boost::is_any_of(" ,\n,\t"),
                         boost::token_compress_on);
        }
        
        if (descFnsString.is_initialized())
            ASSERT( fidxFns_template.size()==descFns.size() );
        
        std::vector<std::string> fidxFns;
        
        for (uint32_t iFidx=0; iFidx<fidxFns_template.size(); ++iFidx) {
            
            std::string fidxFn= util::expandUser((boost::format(fidxFns_template[iFidx]) % nSubQuant % subQuantK % nCoarseK ).str());
            fidxFns.push_back(fidxFn);
            
            if ( !boost::filesystem::exists( fidxFn ) ){
                double t0= timing::tic();
                
                // compress set
                ASSERT(descFns.size()>0);
                std::string descFn= util::expandUser(descFns[iFidx]);
                
                descGetterFromFile *descFile;
                std::vector< descGetterFromFile* > descGettersToDelete;
                
                indexWithDataBuilder *fidxBuilder;
                
                if (numDiv>1){
                    
                    uint32_t totCnt= numDiv*(numDiv+1)*(2*numDiv+1)/6; // = sum_i=0^numDiv (i^2);
                    descGetterFromFile *origGetter= new descFromFlatFile( descFn.c_str(), 512, totCnt);
                    descGettersToDelete.push_back(origGetter);
                    
                    descFile= new descFileRotate( *origGetter, util::expandUser((boost::format("~/Relja/Data/BBC/BBCc/rotation_BBCc_test_hesaff_rootsift_k256_a0_norm3_dim%d.bin") % vladDim).str()), true );
                    descGettersToDelete.push_back(descFile);
                    
                    fidxBuilder= new indexWithDataFileBuilder(fidxFn);

                } else {
                    
                    descGetterFromFile *origGetter= new descFromFlatFile( descFn.c_str(), 512, 1, vladDim, true);
                    descGettersToDelete.push_back(origGetter);
                    
                    descFile= new descFileRotate( *origGetter, util::expandUser((boost::format("~/Relja/Data/BBC/BBCc/rotation_BBCc_test_hesaff_rootsift_k256_a0_norm3_dim%d.bin") % vladDim).str()), true );
                    descGettersToDelete.push_back(descFile);
                    
                    fidxBuilder= new indexWithDataFileFixed1Builder(fidxFn, pq.numBytesPerVector() );
                }
                
                coarseResidualFidxBuilder::buildFromFile( *fidxBuilder, coarseClstFn, pq, *descFile, false );
                delete fidxBuilder;
                
                std::cout<<"elapsed time: "<< timing::toc(t0)/1000 <<" s\n";
                
                for (uint32_t i= 0; i<descGettersToDelete.size(); ++i)
                    delete descGettersToDelete[i];
                
            }
            
        }
        
        // load fidx's
        std::vector<indexWithData const *> fidxs;
        
        for (uint32_t iFidx=0; iFidx<fidxFns.size(); ++iFidx) {
            fidxs.push_back(
                numDiv>1 ?
                    static_cast<indexWithData const *>(new indexWithDataFile(fidxFns[iFidx])) :
                    static_cast<indexWithData const *>(new indexWithDataFileFixed1(fidxFns[iFidx]))
                );
        }
        indexesWithData fidx(fidxs);
        
        std::string iidxFn_template= util::expandUser(pt.get<std::string>( dsetname+".iidxFn" ));
        std::string iidxFn= (boost::format(iidxFn_template) % nSubQuant % subQuantK % nCoarseK ).str();
        
        if ( !boost::filesystem::exists( iidxFn ) ){
            // compute iidx
            double t0= timing::tic();
            iidxWithDataBuilder::build(fidx, iidxFn, pq, util::expandUser("~/Relja/Data/tmp/"));
            std::cout<<"elapsed time: "<< timing::toc(t0)/1000 <<" s\n";
        }
        
        if (APIport!=0) {
            
            #if 0
            indexWithDataFile const iidx(iidxFn);
            #else
                
                #if 0
                indexWithData const iidx_(iidxFn);
                indexWithDataInRam iidx(iidx_);
                #else
                
                indexWithData *iidxOnDisk= new indexWithDataFile(iidxFn);
                boost::function<indexWithData*()> iidxInRamConstructor= boost::lambda::bind( boost::lambda::new_ptr<indexWithDataInRam>(), boost::cref(*iidxOnDisk) );
                
                indexWithDataInRamStartDisk iidx(*iidxOnDisk, iidxInRamConstructor, true );
                #endif
                
            #endif
            
            coarseResidual coarseResidual_obj( coarseClstFn, pq, iidx, w, false );
            nnSingleRetriever nnSR( &fidx, coarseResidual_obj );
            multiQueryMax mq(nnSR);
            
            singleAPI API_obj( dset, nnSR, &mq );
            boost::asio::io_service io_service;
            API_obj.server(io_service, APIport);
            
        }
        
        for (uint32_t iFidx=0; iFidx<fidxs.size(); ++iFidx)
            delete fidxs[iFidx];
        
    } else {
        
        std::string descFn= util::expandUser("~/Relja/Data/BBC/BBCc/shortVLAD_BBCc_test_2008_BBCc_train_BBCc_train_k256_rs1_a0_BBCc_train_norm3.bin");
        
        descFromFlatFile descFile( descFn.c_str(), 512, 1, vladDim, true);
        rawSingleRetriever nnSR( descFile );
        
        singleAPI API_obj( dset, nnSR );
        boost::asio::io_service io_service;
        API_obj.server(io_service, APIport);
        
    }
    
    return 0;
    
}
