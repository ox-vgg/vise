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

#include <iostream>

#include "index_entry.pb.h"
#include "proto_index.h"



int main() {
    float *aQuant, *bQuant, *cQuant;
    
    indexEntryUtil::preUnquantEllipse(aQuant, bQuant, cQuant);
    rr::indexEntry entry;
    
    {
        uint32_t size= 256*256*256;
        unsigned char *scale= new unsigned char[size];
        unsigned char *ratio= new unsigned char[size];
        unsigned char *angle= new unsigned char[size];
        
        uint32_t i= 0;
        for (unsigned int iScale= 0; iScale<256; ++iScale) {
            for (unsigned int iRatio= 0; iRatio<256; ++iRatio) {
                for (unsigned int iAngle= 0; iAngle<256; ++iAngle) {
                    scale[i]= static_cast<unsigned char>(iScale);
                    ratio[i]= static_cast<unsigned char>(iRatio);
                    angle[i]= static_cast<unsigned char>(iAngle);
                    ++i;
                }
            }
        }
        
        entry.set_qel_scale(reinterpret_cast<char*>(scale), size);
        delete []scale;
        entry.set_qel_ratio(reinterpret_cast<char*>(ratio), size);
        delete []ratio;
        entry.set_qel_angle(reinterpret_cast<char*>(angle), size);
        delete []angle;
    }
    
    indexEntryUtil::predUnquantEllipse(aQuant, bQuant, cQuant, entry);
    rr::indexEntry entryRaw= entry;
    indexEntryUtil::quantEllipse(entry);
    
    {
        std::string scaleStr= entry.qel_scale();
        std::string ratioStr= entry.qel_ratio();
        std::string angleStr= entry.qel_angle();
        unsigned char const *scale= reinterpret_cast<unsigned char const*>(scaleStr.c_str());
        unsigned char const *ratio= reinterpret_cast<unsigned char const*>(ratioStr.c_str());
        unsigned char const *angle= reinterpret_cast<unsigned char const*>(angleStr.c_str());
        
        uint32_t i= 0;
        for (unsigned int iScale= 0; iScale<256; ++iScale) {
            if (iScale%5==0) std::cout<<"iScale: "<<iScale<<" / 256\n";
            for (unsigned int iRatio= 0; iRatio<256; ++iRatio) {
                for (unsigned int iAngle= 0; iAngle<256; ++iAngle) {
                    ASSERT(scale[i]==iScale);
                    ASSERT(ratio[i]==iRatio);
                    bool angleRight= (angle[i]==iAngle) || (iRatio==0) ||
                        (angle[i]==255 && iAngle==0) || (angle[i]==0 && iAngle==255) ||
                        (angle[i]==128 && iAngle==0) || (angle[i]==0 && iAngle==255) ||
                        bQuant[i]<1e-9 ;
                    if (!angleRight){
                        std::cout<<(uint32_t)iScale<<" "<<(uint32_t)iRatio<<" "<<(uint32_t)iAngle<<"; ";
                        std::cout<<(uint32_t)scale[i]<<" "<<(uint32_t)ratio[i]<<" "<<(uint32_t)angle[i]<<"\n";
                        std::cout<<entryRaw.a(i)<<" "<<entryRaw.b(i)<<" "<<entryRaw.c(i)<<"\n";
                    }
                    ASSERT( angleRight );
                    ++i;
                }
            }
        }
    }
    
    delete []aQuant;
    delete []bQuant;
    delete []cQuant;
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
