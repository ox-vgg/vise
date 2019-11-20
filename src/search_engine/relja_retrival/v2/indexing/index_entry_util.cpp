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

#include "index_entry_util.h"


#include <cmath>

#include "protobuf_util.h"



uint32_t
indexEntryUtil::getNum(rr::indexEntry const &entry){
    return (entry.id_size()>0) ? entry.id_size() : entry.diffid_size();
}



uint32_t
indexEntryUtil::getNum(std::vector<rr::indexEntry> const &entries){
    uint32_t total= 0;
    for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry)
        total+= getNum(entries[iEntry]);
    return total;
}



bool
indexEntryUtil::isEmpty(std::vector<rr::indexEntry> const &entries){
    if (entries.empty())
        return true;
    for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry)
        if (entries[iEntry].id_size()>0 || entries[iEntry].diffid_size()>0)
            return false;
    return true;
}



uint32_t
indexEntryUtil::getUniqNum(std::vector<rr::indexEntry> const &entries){
    bool isFirst= true;
    uint32_t prevID= 0, currID= 0, N= 0;
    
    for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
        rr::indexEntry const &entry= entries[iEntry];
        ASSERT( (entry.id_size()>0) != (entry.diffid_size()>0) );
        
        if (entry.id_size()>0) {
            for (int i= 0; i<entry.id_size(); ++i){
                currID= entry.id(i);
                if (isFirst || currID!=prevID){
                    ASSERT(isFirst || currID>prevID);
                    ++N;
                    prevID= currID;
                    isFirst= false;
                }
            }
        } else {
            currID= 0;
            for (int i= 0; i<entry.diffid_size(); ++i){
                currID+= entry.diffid(i);
                if (isFirst || currID!=prevID){
                    ++N;
                    prevID= currID;
                    isFirst= false;
                }
            }
        }
    }
    
    return N;
}



void
indexEntryUtil::toDiff(rr::indexEntry &entry){
    if (entry.diffid_size()==0 && entry.id_size()!=0 ){
        entry.mutable_diffid()->Reserve(entry.id_size());
        entry.add_diffid(entry.id(0));
        for (int i= 1; i < entry.id_size(); ++i){
            ASSERT( entry.id(i) >= entry.id(i-1) );
            entry.add_diffid( entry.id(i) - entry.id(i-1) );
        }
        entry.clear_id();
    }
}



void
indexEntryUtil::fromDiff(rr::indexEntry &entry){
    if (entry.id_size()==0 && entry.diffid_size()!=0 ){
        /*
        // the following code is equivalent (but a bit faster) to:
        entry.mutable_id()->Reserve(entry.diffid_size());
        uint32_t ID= entry.diffid(0);
        entry.add_id(ID);
        for (int i= 1; i < entry.diffid_size(); ++i){
            ID+= entry.diffid(i);
            entry.add_id(ID);
        }
        entry.clear_diffid();
        */
        
        entry.mutable_id()->Swap(entry.mutable_diffid());
        uint32_t size= entry.id_size();
        uint32_t *idIt= entry.mutable_id()->mutable_data();
        uint32_t *idEnd= idIt+size;
        
        uint32_t ID= *idIt;
        
        for (++idIt; idIt != idEnd; ++idIt){
            ID+= *idIt;
            *idIt= ID;
        }
    }
}



void
indexEntryUtil::quantXY(rr::indexEntry &entry){
    if (entry.qx_size()==0 && entry.x_size()!=0 ){
        ASSERT(entry.x_size()==entry.y_size());
        entry.mutable_qx()->Reserve(entry.x_size());
        entry.mutable_qy()->Reserve(entry.x_size());
        for (int i= 0; i < entry.x_size(); ++i){
            entry.add_qx(round(entry.x(i)));
            entry.add_qy(round(entry.y(i)));
        }
        entry.clear_x();
        entry.clear_y();
    }
}



void
indexEntryUtil::unquantXY(rr::indexEntry &entry){
    if (entry.qx_size()!=0 && entry.x_size()==0 ){
        ASSERT(entry.qx_size()==entry.qy_size());
        entry.mutable_x()->Reserve(entry.qx_size());
        entry.mutable_y()->Reserve(entry.qx_size());
        for (int i= 0; i < entry.qx_size(); ++i){
            entry.add_x(entry.qx(i));
            entry.add_y(entry.qy(i));
        }
        entry.clear_qx();
        entry.clear_qy();
    }
}



void
indexEntryUtil::quantEllipse(rr::indexEntry &entry){
    
    if (!entry.has_qel_scale() && entry.a_size()!=0 ){
        ASSERT(entry.a_size()==entry.b_size() && entry.a_size()==entry.c_size());
        
        uint32_t size= entry.a_size();
        unsigned char *scale= new unsigned char[size];
        unsigned char *ratio= new unsigned char[size];
        unsigned char *angle= new unsigned char[size];
        double tr, det, sqrtpart, lam1, lam2, rat, alpha;
        
        for (int i= 0; i < entry.a_size(); ++i){
            tr= entry.a(i)+entry.c(i);
            det= entry.a(i)*entry.c(i) - entry.b(i)*entry.b(i);
            sqrtpart= sqrt(std::max(tr*tr/4-det, 1e-20)); // max to avoid numerical problems
            lam1= tr/2+sqrtpart;
            lam2= tr/2-sqrtpart;
            rat= lam1/lam2;
            alpha= (fabs(entry.b(i))<1e-9) ? 0 :
                        // both give the same answer, but depending on numerics some could be better
                        ( (fabs(lam1-entry.c(i)) > fabs(lam1-entry.a(i))) ?
                            atan2(entry.b(i), lam1-entry.c(i)) :
                            atan2(lam1-entry.a(i), entry.b(i)) );
            // to avoid numerical problems
            if (alpha<-piHalf) alpha+=pi; else if (alpha>piHalf) alpha-=pi;
            
            scale[i]= quantizeTo256( log2(det), scale_min, scale_max);
            ratio[i]= quantizeTo256( log2(rat), ratio_min, ratio_max);
            angle[i]= quantizeTo256(     alpha, angle_min, angle_max);
        }
        entry.clear_a();
        entry.clear_b();
        entry.clear_c();
        entry.set_qel_scale(reinterpret_cast<char*>(scale), size);
        delete []scale;
        entry.set_qel_ratio(reinterpret_cast<char*>(ratio), size);
        delete []ratio;
        entry.set_qel_angle(reinterpret_cast<char*>(angle), size);
        delete []angle;
    }
}



void
indexEntryUtil::unquantEllipse(rr::indexEntry &entry){
    
    if (entry.has_qel_scale() && entry.a_size()==0 ){
        ASSERT(entry.b_size()==0);
        ASSERT(entry.c_size()==0);
        std::string const &scaleStr= entry.qel_scale();
        std::string const &ratioStr= entry.qel_ratio();
        std::string const &angleStr= entry.qel_angle();
        unsigned char const *scale= reinterpret_cast<unsigned char const*>(scaleStr.c_str());
        unsigned char const *ratio= reinterpret_cast<unsigned char const*>(ratioStr.c_str());
        unsigned char const *angle= reinterpret_cast<unsigned char const*>(angleStr.c_str());
        
        double det, lam1, lam2, rat, alpha;
        double cosa, sina, cossqa, sinsqa;
        for (uint32_t i= 0; i < scaleStr.length(); ++i){
            det= pow(2, unquantizeFrom256( scale[i], scale_min, scale_max) );
            rat= pow(2, unquantizeFrom256( ratio[i], ratio_min, ratio_max) );
            alpha= unquantizeFrom256( angle[i], angle_min, angle_max);
            lam1= sqrt(rat*det);
            lam2= lam1/rat;
            // to undo rotation, need to rotate in the oposite direction
            cosa= cos(alpha); sina= sin(alpha); cossqa= cosa*cosa; sinsqa= sina*sina;
            entry.add_a( lam1*cossqa + lam2*sinsqa );
            entry.add_b( (lam1-lam2)*sina*cosa );
            entry.add_c( lam1*sinsqa + lam2*cossqa );
        }
        
        entry.clear_qel_scale();
        entry.clear_qel_ratio();
        entry.clear_qel_angle();
    }
}



void
indexEntryUtil::preUnquantEllipse(float *&aQuant, float *&bQuant, float *&cQuant){
    uint32_t size= 256*256*256;
    aQuant= new float[size];
    bQuant= new float[size];
    cQuant= new float[size];
    double det, lam1, lam2, rat, alpha;
    double cosa, sina, cossqa, sinsqa;
    
    for (int scale= 0; scale<256; ++scale)
        for (int ratio= 0; ratio<256; ++ratio) {
            det= pow(2, unquantizeFrom256( static_cast<unsigned char>(scale), scale_min, scale_max) );
            rat= pow(2, unquantizeFrom256( static_cast<unsigned char>(ratio), ratio_min, ratio_max) );
            lam1= sqrt(rat*det);
            lam2= lam1/rat;
            
            for (int angle= 0; angle<256; ++angle) {
                alpha= unquantizeFrom256( static_cast<unsigned char>(angle), angle_min, angle_max);
                // to undo rotation, need to rotate in the oposite direction
                cosa= cos(alpha); sina= sin(alpha); cossqa= cosa*cosa; sinsqa= sina*sina;
                uint32_t currInd= static_cast<uint32_t>(scale)*256*256 + static_cast<uint32_t>(ratio)*256 + static_cast<uint32_t>(static_cast<unsigned char>(angle));
                aQuant[currInd]= lam1*cossqa + lam2*sinsqa;
                bQuant[currInd]= (lam1-lam2)*sina*cosa;
                cQuant[currInd]= lam1*sinsqa + lam2*cossqa;
            }
        }
}



void
indexEntryUtil::predUnquantEllipse(
        float const *aQuant,
        float const *bQuant,
        float const *cQuant,
        rr::indexEntry &entry){
    
    if (entry.has_qel_scale()){
        ASSERT(entry.b_size()==0);
        ASSERT(entry.c_size()==0);
        std::string const &scaleStr= entry.qel_scale();
        std::string const &ratioStr= entry.qel_ratio();
        std::string const &angleStr= entry.qel_angle();
        unsigned char const *scale= reinterpret_cast<unsigned char const*>(scaleStr.c_str());
        unsigned char const *ratio= reinterpret_cast<unsigned char const*>(ratioStr.c_str());
        unsigned char const *angle= reinterpret_cast<unsigned char const*>(angleStr.c_str());
        
        uint32_t size= scaleStr.length();
        entry.mutable_a()->Reserve(size);
        entry.mutable_b()->Reserve(size);
        entry.mutable_c()->Reserve(size);
        uint32_t ind;
        
        for (uint32_t i= 0; i < size; ++i, ++scale, ++ratio, ++angle){
            ind= static_cast<uint32_t>(*scale)*256*256 +
                 static_cast<uint32_t>(*ratio)*256 +
                 static_cast<uint32_t>(*angle);
            entry.add_a( aQuant[ind] );
            entry.add_b( bQuant[ind] );
            entry.add_c( cQuant[ind] );
        }
        
        entry.clear_qel_scale();
        entry.clear_qel_ratio();
        entry.clear_qel_angle();
    }
    
}



uint32_t
indexEntryUtil::markInside(
            rr::indexEntry const &entry,
            query const &queryObj,
            std::vector< std::pair<uint32_t,uint32_t> > &inds,
            int *begin, int *end,
            uint32_t offset){
    
    uint32_t N= 0;
    int begin_= (begin==NULL ? 0 : *begin);
    int end_= (end==NULL ? entry.id_size() : *end);
    int j;
    
    for (int i= begin_; i < end_; i= j){
        
        for (j= i; j < end_ && (
                (entry.x_size()>0 &&
                    entry.x(j) >= queryObj.xl &&
                    entry.x(j) <= queryObj.xu &&
                    entry.y(j) >= queryObj.yl &&
                    entry.y(j) <= queryObj.yu) ||
                (entry.qx_size()>0 &&
                    entry.qx(j) >= queryObj.xl &&
                    entry.qx(j) <= queryObj.xu &&
                    entry.qy(j) >= queryObj.yl &&
                    entry.qy(j) <= queryObj.yu) );
            ++j);
        
        if (j>i){
            inds.push_back( std::make_pair(offset + i, offset + j) );
            N+= j-i;
        } else
            ++j;
        
    }
    
    return N;
}



uint32_t
indexEntryUtil::markInside(std::vector<rr::indexEntry> const &entries, query const &queryObj, std::vector< std::pair<uint32_t,uint32_t> > &inds){
    
    bool inRange= inds.size()>0;
    std::vector< std::pair<uint32_t,uint32_t> > ranges;
    uint32_t offset= 0;
    
    if (!inRange) {
        // set ranges
        
        ranges.reserve( entries.size() );
        
        for (uint32_t iEntry= 0; iEntry < entries.size(); offset= ranges.back().second, ++iEntry)
            ranges.push_back( std::make_pair(offset, offset+entries[iEntry].id_size()) );
        
    } else 
        ranges.swap(inds);
    
    
    // go only through the specified ranges
    
    uint32_t N= 0;
    offset= 0;
    
    uint32_t iEntry= 0;
    
    for (uint32_t iRange= 0; iRange<ranges.size(); ++iRange){
        std::pair<uint32_t, uint32_t> const &range= ranges[iRange];
        ASSERT(iRange==0 || ranges[iRange-1].second <= range.first);
        ASSERT(range.first <= range.second);
        
        // find iEntry
        for(; iEntry < entries.size() && range.first >= offset + entries[iEntry].id_size();
              ++iEntry, offset+= entries[iEntry].id_size());
        ASSERT( iEntry < entries.size() );
        rr::indexEntry const &entry= entries[iEntry];
        
        // get range inside entry
        int begin= static_cast<uint32_t>(range.first-offset);
        int end  = static_cast<uint32_t>(range.second-offset);
        ASSERT(end <= entry.id_size());
        
        // mark inside the range
        N+= markInside(entry, queryObj, inds, &begin, &end, offset);
        
    }
    
    return N;
}



void
indexEntryUtil::copyRange(
        rr::indexEntry const &from,
        int begin, int end,
        rr::indexEntry &to,
        uint32_t const *ID,
        bool copyXY,
        bool copyEllipse,
        embedderFactory const *embFactory){
    
    // copy the relevant data
    
    if (copyXY){
        // geometry
        
        if (from.x_size()>0)
            protobufUtil::addRangeToEnd<float>( from.x(), begin, end, *(to.mutable_x()) );
        
        if (from.y_size()>0)
            protobufUtil::addRangeToEnd<float>( from.y(), begin, end, *(to.mutable_y()) );
        
        if (from.qx_size()>0)
            protobufUtil::addRangeToEnd<uint32_t>( from.qx(), begin, end, *(to.mutable_qx()) );
        
        if (from.qy_size()>0)
            protobufUtil::addRangeToEnd<uint32_t>( from.qy(), begin, end, *(to.mutable_qy()) );
        
    }
    
    if (copyEllipse){
        
        if (from.a_size()>0)
            protobufUtil::addRangeToEnd<float>( from.a(), begin, end, *(to.mutable_a()) );
        
        if (from.b_size()>0)
            protobufUtil::addRangeToEnd<float>( from.b(), begin, end, *(to.mutable_b()) );
        
        if (from.c_size()>0)
            protobufUtil::addRangeToEnd<float>( from.c(), begin, end, *(to.mutable_c()) );
        
        if (from.has_qel_scale()>0)
            protobufUtil::addRangeToEnd( from.qel_scale(), begin, end, *(to.mutable_qel_scale()) );
        
        if (from.has_qel_ratio()>0)
            protobufUtil::addRangeToEnd( from.qel_ratio(), begin, end, *(to.mutable_qel_ratio()) );
        
        if (from.has_qel_angle()>0)
            protobufUtil::addRangeToEnd( from.qel_angle(), begin, end, *(to.mutable_qel_angle()) );
        
    }
    
    if (embFactory!=NULL){
        ASSERT(from.has_data());
        embedder *embFrom= embFactory->getEmbedder();
        embedder *embTo= embFactory->getEmbedder();
        
        embFrom->setDataCopy(from.data());
        embTo->setDataCopy(to.data());
        
        embTo->copyRangeFrom(*embFrom, begin, end);
        
        to.set_data( embTo->getEncoding() );
        
        delete embFrom;
        delete embTo;
    }
    
    if (ID!=NULL || embFactory!=NULL || copyXY || copyEllipse || from.weight_size()>0 || from.count_size()>0) {
        
        // id
        
        if (ID!=NULL)
            protobufUtil::addManyToEnd<uint32_t>( *ID, end - begin, *(to.mutable_id()) );
        else
            protobufUtil::addRangeToEnd<uint32_t>( from.id(), begin, end, *(to.mutable_id()) );
        
        // count / weight
        
        if (from.weight_size()>0)
            protobufUtil::addRangeToEnd<float>( from.weight(), begin, end, *(to.mutable_weight()) );
        
        if (from.count_size()>0)
            protobufUtil::addRangeToEnd<uint32_t>( from.count(), begin, end, *(to.mutable_count()) );
        
    } else {
        
        // just add once
        to.add_id( *ID );
        to.add_count( end-begin );
        
    }
    
}



void
indexEntryUtil::copyRanges(
        std::vector<rr::indexEntry> const &fromEntries,
        std::vector< std::pair<uint32_t, uint32_t> > const &ranges,
        rr::indexEntry &to,
        uint32_t const *ID,
        bool copyXY,
        bool copyEllipse,
        embedderFactory const *embFactory){
    
    uint32_t offset= 0;
    uint32_t iEntry= 0;
    
    for (uint32_t iRange= 0; iRange < ranges.size(); ++iRange){
        std::pair<uint32_t, uint32_t> const &range= ranges[iRange];
        ASSERT(iRange==0 || ranges[iRange-1].second <= range.first);
        ASSERT(range.first <= range.second);
        
        // find iEntry
        for(; iEntry < fromEntries.size() && range.first >= offset + fromEntries[iEntry].id_size();
                ++iEntry, offset+= fromEntries[iEntry].id_size());
        ASSERT( iEntry < fromEntries.size() );
        rr::indexEntry const &from= fromEntries[iEntry];
        
        // get range inside entry
        int begin= static_cast<int>(range.first-offset);
        int end  = static_cast<int>(range.second-offset);
        ASSERT(end <= from.id_size());
        
        copyRange(from, begin, end, to, ID, copyXY, copyEllipse, embFactory);
        
    }
    
}



indexEntryVector::indexEntryVector(std::vector<rr::indexEntry> const &entries) : entriesSize_(entries.size()), entries_(&entries) {
    
    nEntry_.reserve(entriesSize_);
    offset_.reserve(entriesSize_);
    num_= 0;
    uint32_t n;
    diffIDs_= false;
    
    for (uint32_t iEntry= 0; iEntry < entriesSize_; ++iEntry){
        rr::indexEntry const &entry= entries_->at(iEntry);
        n= indexEntryUtil::getNum(entry);
        diffIDs_= diffIDs_ || (entry.diffid_size()>0);
        nEntry_.push_back(n);
        offset_.push_back(num_);
        num_+= n;
    }
    
}



std::pair<uint32_t, int>
indexEntryVector::getInds(uint32_t ind) const {
    
    uint32_t iEntry= 0;
    for (; iEntry < entriesSize_ && ind < offset_[iEntry]; ++iEntry);
    ASSERT(iEntry<entriesSize_);
    
    return std::make_pair(iEntry, ind - offset_[iEntry]);
}



uint32_t
indexEntryVector::getID(uint32_t ind) const {
    
    ASSERT(!diffIDs_);
    
    uint32_t iEntry= 0;
    for (; iEntry < entriesSize_ && ind < offset_[iEntry]; ++iEntry);
    ASSERT(iEntry<entriesSize_);
    
    return entries_->at(iEntry).id(ind - offset_[iEntry]);
    
}



uint32_t
ievIterator::dereference() const {
    return iev_->getID(ind_);
}

bool
ievIterator::equal(ievIterator const &it) const {
    return it.ind_ == ind_;
}

void
ievIterator::increment() {
    ++ind_;
}

void
ievIterator::decrement() {
    --ind_;
}

void
ievIterator::advance(uint32_t n) {
    ind_+= n;
}

size_t
ievIterator::distance_to(ievIterator const &it) const {
    return it.ind_ - ind_;
}
