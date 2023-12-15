/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

*/

#include "feat_standard.h"

#include <string>
#include <fstream>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "image_util.h"
#include "detect_points.h"
#include "compute_descriptors.h"



void readRegions( const char fileName[], uint32_t &numRegs, std::vector<ellipse> &regions ){
    
    std::ifstream fin( fileName );
    if (!fin.is_open()){
        regions.clear();
        numRegs= 0;
    }
    
    double temp_, x, y, a, b, c;
    fin>>temp_;
    fin>>numRegs;
    regions.resize( numRegs );
    for (uint32_t i=0; i<numRegs; ++i){
        fin>>x>>y>>a>>b>>c;
        regions[i].set(x,y,a,b,c);
    }
    
    fin.close();
    
}



void writeRegions( const char fileName[], std::vector<ellipse> const &regions ){
    
    std::ofstream fout( fileName );
    
    double x, y, a, b, c;
    fout<<"1.0\n";
    fout<<regions.size()<<"\n";
    for (uint32_t i=0; i<regions.size(); ++i){
        regions[i].get(x,y,a,b,c);
        fout<<x<<" "<<y<<" "<<a<<" "<<b<<" "<<c<<"\n";
    }
    
    fout.close();
    
}



void readRegsAndDescs( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ){
    
    double temp_, x, y, a, b, c;
    uint32_t numDims;
    
    std::ifstream fin( fileName );
    if (!fin.is_open()){
        regions.clear();
        numFeats= 0;
        descs= new float[0];
        return;
    }
    
    fin>>numDims>>numFeats;
    
    regions.resize( numFeats );
    uint32_t iDim;
    
    descs= new float[numFeats*numDims];
    float *descIter= descs;
    
    for (uint32_t i=0; i<numFeats; ++i){
        fin>>x>>y>>temp_>>a>>b>>c;
        regions[i].set(x,y,a,b,c);
        for (iDim=0; iDim<numDims; ++iDim, ++descIter)
            fin>> *descIter;
    }
    
    fin.close();
    
}



void reg_KM_HessAff::getRegs( const char fileName[], uint32_t &numRegs, std::vector<ellipse> &regions ) const {
    boost::filesystem::path tmpdir = boost::filesystem::temp_directory_path();
    boost::filesystem::path tmpfn = tmpdir / boost::filesystem::unique_path("vise_region_%%%%%%%%%%%%%%%%.bin");
    std::string tempRegsFn = tmpfn.string();
   
    // detect
    
    // form command line:
    // boost::format("detect_points_2.ln -i \"%s\" -hesaff -o \"%s\" > /dev/null") % fileName % tempRegsFn
    std::vector<std::string> args_;
    args_.push_back("detect_points_2.ln");
    args_.push_back("-i");
    args_.push_back(fileName);
    args_.push_back("-hesaff");
    args_.push_back("-o");
    args_.push_back(tempRegsFn);
    
    // convert to char
    std::vector<char*> args;
    for (uint32_t i= 0; i<args_.size(); ++i){
        args_[i]+= '\0';
        args.push_back(&args_[i][0]);
    }
    
    // execute
    KM_detect_points::lib_main(args.size(),&args[0]);
    
    // read
    readRegions( tempRegsFn.c_str(), numRegs, regions );
    
    // cleanup
    boost::filesystem::remove( tempRegsFn );
}



void desc_KM_SIFT::getDescs( const char fileName[], std::vector<ellipse> &regions, uint32_t &numFeats, float *&descs ) const {
    boost::filesystem::path tmpdir = boost::filesystem::temp_directory_path();
    boost::filesystem::path tmpregfn = tmpdir / boost::filesystem::unique_path("vise_featreg_%%%%%%%%%%%%%%%%.jpg");
    boost::filesystem::path tmpdescfn = tmpdir / boost::filesystem::unique_path("vise_featdesc_%%%%%%%%%%%%%%%%.jpg");
    std::string tempRegsFn = tmpregfn.string();
    std::string tempDescsFn= tmpdescfn.string();
        
    // write regions
    writeRegions( tempRegsFn.c_str(), regions );
    
    // compute SIFT
    
    // form command line:
    // boost::format("compute_descriptors_2.ln -i \"%s\" -p1 \"%s\" -sift -o3 \"%s\" -scale-mult %.3f") % fileName % tempRegsFn % tempDescsFn % scaleMulti
    std::vector<std::string> args_;
    args_.push_back("compute_descriptors_2.ln");
    args_.push_back("-i");
    args_.push_back(fileName);
    args_.push_back("-p1");
    args_.push_back(tempRegsFn);
    args_.push_back("-sift");
    args_.push_back("-o3");
    args_.push_back(tempDescsFn);
    args_.push_back("-scale-mult");
    args_.push_back( (boost::format("%.3f") % scaleMulti).str() );
    if (upright)
        args_.push_back("-noangle");
    
    // convert to char
    std::vector<char*> args;
    for (uint32_t i= 0; i<args_.size(); ++i){
        args_[i]+= '\0';
        args.push_back(&args_[i][0]);
    }
    
    // execute
    KM_compute_descriptors::lib_main(args.size(),&args[0]);
    
    // cleanup
    boost::filesystem::remove( tempRegsFn );
        
    // read
    readRegsAndDescs( tempDescsFn.c_str(), numFeats, regions, descs );
    
    // cleanup
    boost::filesystem::remove( tempDescsFn );
}




std::string
desc_KM_SIFT::getRawDescs(float const *descs, uint32_t numFeats) const {
    std::string res(numFeats*128, '\0');
    uint8_t *resIter= reinterpret_cast<uint8_t*>(&res[0]);
    float const *inIter= descs;
    float const *inIterEnd= descs + numFeats*128;
    for (; inIter!=inIterEnd; ++inIter, ++resIter)
        *resIter= static_cast<uint8_t>(*inIter + 0.1); // +0.1 is to counter numerical issues because cast does floor()
    return res;
}
