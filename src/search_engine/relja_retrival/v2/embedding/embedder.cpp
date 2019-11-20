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

#include "embedder.h"



rawEmbedder::rawEmbedder(uint32_t dim)
        : dim_(dim), charStream_(NULL) {
    charStream_= new charStreamNative<uint32_t>;
}



void
rawEmbedder::add(float vector[], uint32_t clusterID){
    uint32_t *v= reinterpret_cast<uint32_t*>(vector); // assumes sizeof(float)==32
    uint32_t *vEnd= v+dim_;
    reserveAdditional(1);
    for (; v!=vEnd; ++v)
        charStream_->add(*v);
}



void
rawEmbedder::copyFrom(embedder &emb, uint32_t index){
    reserveAdditional(1);
    rawEmbedder* thisEmb= dynamic_cast<rawEmbedder*>( &emb );
    thisEmb->charStream_->setIter(index*dim_);
    for (uint32_t i= 0; i<dim_; ++i)
        charStream_->add( thisEmb->charStream_->getNextUnsafe() );
}



void
rawEmbedder::copyRangeFrom(embedder &emb, uint32_t start, uint32_t end){
    reserveAdditional(end-start);
    rawEmbedder* thisEmb= dynamic_cast<rawEmbedder*>( &emb );
    thisEmb->charStream_->setIter(start*dim_);
    for (; start<end; ++start)
        for (uint32_t i= 0; i<dim_; ++i)
            charStream_->add( thisEmb->charStream_->getNextUnsafe() );
}



void
rawEmbedder::reserve(uint32_t n){
    charStream_->reserve(n*dim_);
}



void
rawEmbedder::setDataCopy(std::string const &data) {
    charStream_->setDataCopy(data);
}



void
rawEmbedder::clear(){
    charStream_->clear();
}
