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

#ifndef _EMBEDDER_H_
#define _EMBEDDER_H_



#include <stdint.h>
#include <string>

#include "char_streams.h"
#include "macros.h"



class embedder {
    public:
        
        embedder() {}
        virtual ~embedder() {}
        
        virtual bool
            doesSomething() const { return true; }
        
        virtual void
            add(float vector[], uint32_t clusterID) =0;
        
        // only use the embedder type = object type
        virtual void
            copyFrom(embedder &emb, uint32_t index) =0;
        
        // only use the embedder type = object type
        // copy range [start, end)
        // NOTE default implementation is probably slow
        virtual void
            copyRangeFrom(embedder &emb, uint32_t start, uint32_t end) {
                for (; start<end; ++start)
                    copyFrom(emb, start);
            }
        
        virtual void
            reserve(uint32_t n) {}
        
        inline void
            reserveAdditional(uint32_t n){
                reserve( getNum() + n );
            }
        
        virtual void
            setDataCopy(std::string const &data) =0;
        
        virtual void
            clear() =0;
        
        virtual uint32_t
            getByteSize() const =0;
        
        virtual std::string
            getEncoding() const =0;
        
        virtual uint32_t
            getNum() const =0;
        
    private:
        DISALLOW_COPY_AND_ASSIGN(embedder)
};



class embedderFactory {
    public:
        
        embedderFactory() {}
        virtual ~embedderFactory() {};
        
        virtual embedder*
            getEmbedder() const =0;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(embedderFactory)
};



class noEmbedder : public embedder {
    public:
        noEmbedder(){}
        bool doesSomething() const { return false; }
        void add(float vector[], uint32_t clusterID) {}
        void copyFrom(embedder &emb, uint32_t index) {}
        void copyRangeFrom(embedder &emb, uint32_t start, uint32_t end) {}
        void setDataCopy(std::string const &data) {};
        void clear() {}
        uint32_t getByteSize() const { return 0; }
        std::string getEncoding() const { return ""; }
        uint32_t getNum() const { return 0; }
    private:
        DISALLOW_COPY_AND_ASSIGN(noEmbedder)
    
};



class noEmbedderFactory : public embedderFactory {
    public:
        noEmbedderFactory(){}
        noEmbedder* getEmbedder() const { return new noEmbedder(); }
    private:
        DISALLOW_COPY_AND_ASSIGN(noEmbedderFactory)
};




class rawEmbedder : public embedder {
    
    public:
        rawEmbedder(uint32_t dim);
        
        ~rawEmbedder() {
            if (charStream_!=NULL) delete charStream_;
        }
        
        void
            add(float vector[], uint32_t clusterID);
        
        void
            copyFrom(embedder &emb, uint32_t index);
        
        void
            copyRangeFrom(embedder &emb, uint32_t start, uint32_t end);
        
        void
            reserve(uint32_t n);
        
        void
            setDataCopy(std::string const &data);
        
        void
            clear();
        
        inline uint32_t
            getByteSize() const { return charStream_->getByteSize(); }
        
        inline std::string
            getEncoding() const {
                return charStream_->getDataCopy();
            }
        
        inline uint32_t
            getNum() const {
                return charStream_->getNum();
            }
    
    private:
        uint32_t dim_;
        charStream *charStream_;
        DISALLOW_COPY_AND_ASSIGN(rawEmbedder)
    
};



class rawEmbedderFactory : public embedderFactory {
    public:
        rawEmbedderFactory(uint32_t dim) : dim_(dim) {}
        
        rawEmbedder*
            getEmbedder() const { return new rawEmbedder(dim_); }
        
    private:
        uint32_t const dim_;
        DISALLOW_COPY_AND_ASSIGN(rawEmbedderFactory)
};

#endif
