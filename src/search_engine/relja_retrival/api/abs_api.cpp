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

#include "abs_api.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "timing.h"



void
absAPI::session( socket_ptr sock ){
    
    // Read the client's request.
    std::string request = "";
    while (1) {
        char buffer[1024];
        //boost::asio::error error;
        boost::system::error_code error;
        size_t len = sock->read_some(boost::asio::buffer(buffer), error);
        if (error == boost::asio::error::eof) { // Connection closed by peer.
            // Nothing to do, return.
            return;
        }
        else if (error) {
            std::cerr << error.message() << "\n";
            throw error; // Some error?
        }
        request+= std::string(buffer, len);
        std::string requestTrim= request;
        boost::algorithm::trim_right(requestTrim);
        
        if (std::string(requestTrim.begin() + requestTrim.size() - 6, requestTrim.end()) == " $END$"){
            request= requestTrim;
            break;
        }
    }
    
    request= request.substr(0, request.length()-6); // remove end
    
    double t0= timing::tic();
    
    // parse the request
    std::stringstream ss( request );
    
    boost::property_tree::ptree pt;
    read_xml( ss, pt );
    
    std::string reply;
    
    if ( pt.count("dsetGetNumDocs") ){
        
        reply= ( boost::format("%d") % dataset_->getNumDoc() ).str();
        
    } else if ( pt.count("dsetGetFn") ){
        
        uint32_t docID= pt.get<uint32_t>("dsetGetFn.docID");
        reply= ( boost::format("%d") % dataset_->getFn(docID) ).str();
        
    } else if ( pt.count("dsetGetDocID") ){
        
        std::string fn= pt.get<std::string>("dsetGetDocID.fn");
        reply= ( boost::format("%d") % dataset_->getDocIDFromAbsFn(fn) ).str();;
        
    } else if ( pt.count("dsetGetWidthHeight") ){
        
        uint32_t docID= pt.get<uint32_t>("dsetGetWidthHeight.docID");
        std::pair<uint32_t, uint32_t> wh= dataset_->getWidthHeight(docID);
        reply= ( boost::format("%d %d") % wh.first % wh.second ).str();
        
    } else if ( pt.count("containsFn") ){
        
        std::string fn= pt.get<std::string>("containsFn.fn");
        reply= ( boost::format("%d") % dataset_->containsFn(fn) ).str();
        
    } else {
        
//         std::cout<< timing::getTimeString() <<" Request= "<<request<<"\n";
        std::cout<< timing::getTimeString() <<" Request= "<< request.substr(0,300) << ( request.length()>300 ? " (...) \n" : "\n" ) ;
        
        reply= getReply(pt, request);
        
//         std::cout<<"Response= "<<reply<<"\n";
        std::cout<<"Response= "<< reply.substr(0,300) << ( reply.length()>300 ? " (...) \n" : "\n" ) ;
        std::cout<<timing::getTimeString()<<" Request - DONE ("<< timing::toc(t0) <<" ms)\n";
    }
    
    boost::asio::write(*sock, boost::asio::buffer(reply));
}



void
absAPI::server(boost::asio::io_service& io_service, short int port) {
    
    std::cout << "Waiting for requests" << "\n";
try_again:
    try {
        tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
        a.set_option(tcp::acceptor::reuse_address(true));
        while (1) {
            socket_ptr sock(new tcp::socket(io_service));
            a.accept(*sock);
            boost::thread t(boost::bind(&absAPI::session, this, sock));
        }
    }
    catch (std::exception& e) {
        std::cerr<<"exception: "<<e.what()<<"\n"; std::cerr.flush();
        sleep(1);
        goto try_again;
    }
    
}

