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

#ifndef _CHAR_STREAMS_H_
#define _CHAR_STREAMS_H_


#include <iostream>
#include <cstring> // for memset and memcpy
#include <math.h>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#include "macros.h"



// do not use for reading and writing at the same time
class charStream {
    
    public:
        
        charStream(){}
        
        virtual ~charStream() {}
        
        virtual uint32_t
            getByteSize() const =0;
        
        virtual uint32_t
            getNum() const =0;
        
        virtual void
            add(uint64_t value) =0;
        
        virtual void
            resetIter() =0;
        
        virtual void
            setIter(uint32_t index) =0;
        
        // only for the read mode (i.e. only valid if used after setDataCopy or resetIter)!
        virtual uint64_t
            getNextUnsafe() =0;
        
        virtual void
            reserve(uint32_t n) {}
        
        inline void
            reserveAdditional(uint32_t n) {
                reserve(getNum()+n);
            }
        
        virtual void
            clear() =0;
        
        virtual std::string
            getDataCopy() const =0;
        
        virtual void
            setDataCopy(std::string const &data) =0;
        
        virtual uint64_t getIth() const {
                throw std::runtime_error("not implemented");
            }
        
        static charStream*
            charStreamCreate(uint8_t bits);
        
    private:
        DISALLOW_COPY_AND_ASSIGN(charStream);
};



template <class T>
class charStreamNative : public charStream {
    
    public:
        
        charStreamNative() : sizeProp_(sizeof(T)/sizeof(char)), iter_(NULL) {}
        
        inline uint32_t
            getByteSize() const { return data_.size() * sizeProp_; }
        
        inline uint32_t
            getNum() const { return data_.size(); }
        
        inline void
            add(uint64_t value){ data_.push_back(static_cast<T>(value)); }
        
        inline void
            resetIter(){ iter_= &data_[0]; }
        
        inline void
            setIter(uint32_t index) { iter_= &data_[index]; }
        
        inline uint64_t
            getNextUnsafe(){
                return *(iter_++);
            }
        
        inline void
            reserve(uint32_t n){ data_.reserve(n); }
        
        inline void
            clear(){ data_.clear(); iter_= NULL; }
        
        inline std::string
            getDataCopy() const {
                return std::string(
                    reinterpret_cast<const char*>(&data_[0]),
                    data_.size()*sizeProp_ );
            }
        
        inline void
            setDataCopy(std::string const &data){
                ASSERT( data.length() % sizeProp_ == 0 );
                data_.resize( data.length() / sizeProp_ );
                std::memcpy(
                    reinterpret_cast<char *>(&data_[0]),
                    data.c_str(),
                    data.length() );
                resetIter();
            }
        
        static inline uint32_t
            numBytesForN(uint32_t n)
            { return n*sizeof(T); }
        
    private:
        uint32_t const sizeProp_;
        std::vector<T> data_;
        T* iter_;
        DISALLOW_COPY_AND_ASSIGN(charStreamNative);
};



class charStreamNonNative : public charStream {
    
    public:
        
        charStreamNonNative() : n_(0) {}
        
        virtual ~charStreamNonNative(){}
        
        inline uint32_t
            getByteSize() const { return data_.size(); }
        
        inline uint32_t
            getNum() const { return n_; }
        
        inline std::string
            getDataCopy() const {
                flush();
                return std::string(
                    reinterpret_cast<const char*>(&data_[0]),
                    data_.size() );
            }
        
        inline void
            setDataCopy(std::string const &data) {
                data_.resize( data.length() );
                std::memcpy(
                    reinterpret_cast<char *>(&data_[0]),
                    data.c_str(),
                    data.length() );
                computeNum();
                resetIter();
            }
        
    protected:
        
        // write the necessary stuff into data_ (e.g. buffered data or size info)
        virtual void
            flush() const =0;
        
        virtual void
            computeNum() =0;
        
        mutable std::vector<uint8_t> data_;
        uint32_t n_;
        
    private:
        DISALLOW_COPY_AND_ASSIGN(charStreamNonNative);
};



class charStream12 : public charStreamNonNative {
    
    // first 1 byte used to get n.. only 1 bit is needed but better to be byte aligned
    // using read and write at the same time has undefined behaviour
    public:
        charStream12() { clear(); }
        void add(uint64_t value);
        void resetIter();
        void setIter(uint32_t index);
        uint64_t getNextUnsafe();
        void reserve(uint32_t n);
        void clear();
        static uint32_t numBytesForN(uint32_t n);
        
    private:
        void flush() const;
        void computeNum();
        
        bool isFirst_;
        uint8_t *iter_;
        
        DISALLOW_COPY_AND_ASSIGN(charStream12);
        
};



class charStream10 : public charStreamNonNative {
    
    // first 1 byte used to get n.. only 2 bits are needed but better to be byte aligned
    // using read and write at the same time has undefined behaviour
    public:
        charStream10() { clear(); }
        void add(uint64_t value);
        void resetIter();
        void setIter(uint32_t index);
        uint64_t getNextUnsafe();
        void reserve(uint32_t n);
        void clear();
        static uint32_t numBytesForN(uint32_t n);
        
    private:
        void flush() const;
        void computeNum();
        
        uint8_t *iter_;
        uint8_t pos_;
        static uint8_t const mask1_[4], mask2_[4];
        static uint8_t const lshift1_[4], shift2_[4];
        
        DISALLOW_COPY_AND_ASSIGN(charStream10);
        
};



class charStream6 : public charStreamNonNative {
    
    // first 1 byte used to get n.. only 2 bits are needed but better to be byte aligned
    // using read and write at the same time has undefined behaviour
    public:
        charStream6() { clear(); }
        void add(uint64_t value);
        void resetIter();
        void setIter(uint32_t index);
        uint64_t getNextUnsafe();
        void reserve(uint32_t n);
        void clear();
        static uint32_t numBytesForN(uint32_t n);
        
    private:
        void flush() const;
        void computeNum();
        
        uint8_t *iter_;
        uint8_t pos_;
        static uint8_t const mask1_[4], mask2_[4];
        static  int8_t const shift1_[4];
        static uint8_t const shift2_[4];
        
        DISALLOW_COPY_AND_ASSIGN(charStream6);
        
};



class charStream4 : public charStreamNonNative {
    
    // first 1 byte used to get n since size= n*2 or n*2-1.. only 1 bit is needed but better to be byte aligned
    public:
        charStream4() { clear(); }
        void add(uint64_t value);
        void resetIter();
        void setIter(uint32_t index);
        uint64_t getNextUnsafe();
        void reserve(uint32_t n);
        void clear();
        static uint32_t numBytesForN(uint32_t n);
        
    private:
        void flush() const;
        void computeNum();
        
        uint8_t *iter_;
        bool isFirstHalf_;
        static uint8_t const mask_[2];
        static uint8_t const shift_[2];
        
        DISALLOW_COPY_AND_ASSIGN(charStream4);
        
};

#endif
