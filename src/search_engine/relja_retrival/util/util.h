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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <boost/lexical_cast.hpp>

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <math.h>
#include <cstring>
#include <stdint.h>

#include "macros.h"




enum MPItags {
    // queue
    tagQueueStopGo, tagQueueJob, tagQueueResult,
    // api
    tagAPIStart
};



namespace util {
    
    
    
    inline void
        normTo0_1( std::vector<double>::iterator begin, std::vector<double>::iterator end ){
            
            std::vector<double>::iterator itS;
            
            // find min/max
            
            double min= *begin;
            double max= min;
            
            for (itS= begin; itS!=end; ++itS ){
                min= std::min( min, *itS );
                max= std::max( min, *itS );
            }
            
            // normalize to [0,1)
            
            double scale= 1.0/(max+0.1-min);
            
            for (itS= begin; itS!=end; ++itS )
                *itS= (*itS - min) * scale;
            
        }
    
    
    
    inline void
        normTo0_1( std::vector<double> &scores ){
            util::normTo0_1( scores.begin(), scores.end() );
        }
    
    
    
    inline void
        l2normalize( float x[], uint32_t numDims ){
            float invnorm= 0;
            for (uint32_t iDim= 0; iDim<numDims; ++iDim)
                invnorm+= x[iDim] * x[iDim];
            invnorm= 1.0f/sqrt(invnorm);
            for (uint32_t iDim= 0; iDim<numDims; ++iDim)
                x[iDim] *= invnorm;
        }
    
    
    
    template <class itemType>
    void
        del( uint32_t N, itemType **&matrix ){
            for (uint32_t i= 0; i<N; ++i)
                delete []matrix[i];
            delete []matrix;
        }
    
    
    
    template <class itemType>
    void
        delPointerVector( std::vector<itemType> &v ){
            for (uint32_t i= 0; i<v.size(); ++i)
                delete v[i];
        }
    
    
    
    // only use for POD types!
    template <class itemType>
    void
        matrixToFlat( uint32_t N, uint32_t D, itemType **matrix, itemType *&matrixFlat){
            
            matrixFlat= new itemType[N*D];
            itemType *thisMatrixFlat= matrixFlat;
            for (uint32_t i= 0; i<N; ++i){
                std::memcpy( thisMatrixFlat, matrix[i], D * sizeof(itemType) );
                thisMatrixFlat+= D;
            }
            
        }
    
    
    
    inline std::string
        getTempFileWB( FILE *&f, std::string tmpDir= "", std::string prefix= "", std::string suffix= "", bool closeFile= false ){
            
            if (tmpDir.length()==0){
                tmpDir= P_tmpdir;
            }
            
            if (tmpDir[ tmpDir.length()-1 ]!='/')
                tmpDir+= '/';
            tmpDir+= prefix+"XXXXXX"+suffix;
            const char *tmpNameTemplate= tmpDir.c_str();
            char *tmpName= new char[ tmpDir.length()+1 ]; // +1 for null terminated
            strcpy( tmpName, tmpNameTemplate );
            
            int fd= mkstemps( tmpName, suffix.length() );
            std::string tmpName_str= tmpName;
            delete []tmpName;
            
            if (closeFile){
                f= NULL;
                close(fd);
            } else
                f= fdopen(fd, "wb");
            return tmpName_str;
        }
    
    
    
    inline std::string
        getTempFileName( std::string tmpDir= "", std::string prefix= "", std::string suffix= "" ){
            
            FILE *ftemp;
            std::string fn= getTempFileWB(ftemp, tmpDir, prefix, suffix, true);
            return fn;
            
        }
    
    
    
    inline std::string
        expandUser( std::string path ){
            if (path.length()>0 && path[0]=='~')
                return std::string(getenv("HOME")) + path.substr(1, std::string::npos);
            return path;
        }
    
    
    
    inline std::string
        uintToShortStr(uint32_t const num){
            if (num<1000)
                return boost::lexical_cast<std::string>(num);
            uint32_t const divBy= num<1000000 ? 1000 : 1000000;
            float div= static_cast<float>(num)/divBy;
            return boost::lexical_cast<std::string>( round(div) )
                    + ( num%divBy==0 ? "" : "."+boost::lexical_cast<std::string>( static_cast<uint32_t>(round( (div - round(div) )*10 ) ) ) )
                    + (divBy==1000 ? "k" : "M");
        }
    
    
    
    inline uint64_t
        fileSize( std::string fn ){
            
            FILE *f= fopen(fn.c_str(), "rb");
            ASSERT(f!=NULL);
            fseeko64(f, 0, SEEK_END);
            uint64_t byteSize= ftello64(f);
            fclose(f);
            
            return byteSize;
        }
    
    
    
    inline void
        visitFile( std::string fn ){
            
            std::cout<< "visitFile::Start\n";
            
            // get file size
            FILE *f= fopen(fn.c_str(), "rb");
            fseeko64(f, 0, SEEK_END);
            uint64_t byteSize= ftello64(f);
            fseeko64(f, 0, SEEK_SET);
            
            const uint32_t chunkSize= 128*1024*1024; // read 128 MB chunks
            char *buf= new char[chunkSize];
            
            // read the entire file
            uint64_t numReads= byteSize / chunkSize;
            
            uint32_t temp_;
            for (uint64_t iRead= 0; iRead < numReads; ++iRead)
                temp_= fread(buf, 1, 1, f);
            
            // last read
            uint64_t numLast= byteSize - numReads*chunkSize;
            if (numLast>0)
                temp_= fread(buf, numLast, 1, f);
            
            fclose(f);
            delete []buf;
            
            if (false && temp_) {} // to avoid the warning about not checking temp_
            
            std::cout<< "visitFile::Done\n";
        }
    
};

#endif
