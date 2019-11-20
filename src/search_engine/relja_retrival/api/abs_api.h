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

#ifndef _ABS_API_H_
#define _ABS_API_H_

#include <string>
#include <stdint.h>

/*
If these 2 are not included here, before boost/asio.hpp: Arthur has some problems:
/usr/local/include/boost/interprocess/detail/os_file_functions.hpp:363:33: error: ‘SEEK_SET’ cannot appear in a constant-expression
/usr/local/include/boost/interprocess/detail/os_file_functions.hpp:364:33: error: ‘SEEK_END’ cannot appear in a constant-expression
/usr/local/include/boost/interprocess/detail/os_file_functions.hpp:365:33: error: ‘SEEK_CUR’ cannot appear in a constant-expression
*/
#ifdef MPI
#include <boost/mpi.hpp>
#endif
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

#include "dataset_abs.h"



using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;


class absAPI {
    
    public:
        
        absAPI( datasetAbs const &datasetObj ) : dataset_(&datasetObj) {}
        
        virtual ~absAPI() {}
        
        virtual void
            server(boost::asio::io_service& io_service, short int port);
        
        virtual std::string
            getReply( boost::property_tree::ptree &pt, std::string const &request ) const =0;
        
    protected:
        
        void
            session( socket_ptr sock );
        
        datasetAbs const *dataset_;
    
};

#endif
