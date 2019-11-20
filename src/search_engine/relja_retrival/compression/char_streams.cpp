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

#include "char_streams.h"



#include <cstring> // for memset and memcpy
#include <stdexcept>

#include "macros.h"



charStream*
charStream::charStreamCreate(uint8_t bits){
    ASSERT(bits<=64);
    charStream* cs;
    if (bits<=4)
        cs= new charStream4;
    else if (bits<=6)
        cs= new charStream6;
    else if (bits<=8)
        cs= new charStreamNative<uint8_t>;
    else if (bits<=10)
        cs= new charStream10;
    else if (bits<=12)
        cs= new charStream12;
    else if (bits<=16)
        cs= new charStreamNative<uint16_t>;
    else if (bits<=32)
        cs= new charStreamNative<uint32_t>;
    else // if (bits<=64)
        cs= new charStreamNative<uint64_t>;
    
    if (!(bits==4 || bits==6 || bits==8 || bits==10 || bits==12 || bits==16 || bits==32 || bits==64))
        throw std::runtime_error("Wasted bits"); // can run in this mode but probably better to force the programmer to not waste memory
    return cs;
}



// ------------------------------------ charStream12



void
charStream12::add(uint64_t value) {
    ++n_;
    if (isFirst_){
        data_.push_back( value >> 4 );
        data_.push_back( (value & 0x0F) << 4 );
    } else {
        data_.back() |= value >> 8;
        data_.push_back( value & 0xFF );
    }
    isFirst_= !isFirst_;
    iter_+= 1+isFirst_;
}



void
charStream12::resetIter() {
    iter_= &data_[1];
    isFirst_= true;
}



void
charStream12::setIter(uint32_t index) {
    iter_= &data_[1+(index*3)/2];
    isFirst_= (index%2==0);
}



uint64_t
charStream12::getNextUnsafe() {
    uint64_t value= 0;
    if (isFirst_)
        value= (static_cast<uint16_t>(*iter_) << 4) | ( ( *(iter_+1) & 0xF0) >> 4 );
    else
        value= (static_cast<uint16_t>(*iter_ & 0x0F) << 8) | *(iter_+1);
    isFirst_= !isFirst_;
    iter_+= 1+isFirst_;
    return value;
}



uint32_t
charStream12::numBytesForN(uint32_t n){
    return (n*3+1)/2 +1;
}



void
charStream12::reserve(uint32_t n) {
    data_.reserve(numBytesForN(n));
}



void
charStream12::clear() {
    data_.resize(1);
    n_= 0;
    isFirst_= true;
}



void
charStream12::flush() const {
    data_[0]= static_cast<uint8_t>((n_*3)%2);
}



// see calculations in charStream6
void
charStream12::computeNum() {
    ASSERT(data_[0]<2);
    n_= (data_.size()-1 - (data_[0]!=0) )*2 + data_[0];
    ASSERT( n_%3==0 );
    n_/=3;
}



// ------------------------------------ charStream10

// organization: | val1(1:8) | val1(9:10) val2(1:6) | val2(7:10) val3(1:4) | val3(5:10) val4(1:2) | val4(3:10) |
uint8_t const charStream10::mask1_[4]=   { 0xFF, 0x3F, 0x0F, 0x03 };
uint8_t const charStream10::mask2_[4]=   { 0xC0, 0xF0, 0xFC, 0xFF };
uint8_t const charStream10::lshift1_[4]= {    2,    4,    6,    8 };
uint8_t const charStream10::shift2_[4] = {    6,    4,    2,    0 };



void
charStream10::add(uint64_t value) {
    ++n_;
    if (pos_==0)
        data_.push_back( value >> lshift1_[0] );
    else
        data_.back() |= (value >> lshift1_[pos_]);
    data_.push_back( value << shift2_[pos_] );
    pos_= (pos_+1)%4;
}



void
charStream10::resetIter() {
    iter_= &data_[1];
    pos_= 0;
}



void
charStream10::setIter(uint32_t index) {
    iter_= &data_[1+(index*5)/4];
    pos_= index%4;
}



uint64_t
charStream10::getNextUnsafe() {
    uint64_t value= static_cast<uint16_t>(*iter_ & mask1_[pos_]) << lshift1_[pos_];
    value|= ( (*(iter_+1)) & mask2_[pos_] ) >> shift2_[pos_];
    iter_+= 1+(pos_==3);
    pos_= (pos_+1)%4;
    return value;
}



uint32_t
charStream10::numBytesForN(uint32_t n){
    return (n*5)/4 +1 +1;
}



void
charStream10::reserve(uint32_t n) {
    data_.reserve(numBytesForN(n));
}



void
charStream10::clear() {
    data_.resize(1);
    n_= 0;
    pos_= 0;
}



void
charStream10::flush() const {
    data_[0]= static_cast<uint8_t>( (n_*5)%4 );
}



// see calculations in charStream6
void
charStream10::computeNum() {
    ASSERT(data_[0]<4);
    n_= ( (data_.size()-1 - (data_[0]!=0) )*4 + data_[0] );
    ASSERT( n_%5==0 );
    n_/=5;
}



// ------------------------------------ charStream6

// organization: byte1=[val1 val2(1:2)] byte2=[val2(3:6) val3(1:4)] byte3=[val4(5:6) val5]
uint8_t const charStream6::mask1_[4]=  { 0xFC, 0x03, 0x0F, 0x3F };
uint8_t const charStream6::mask2_[4]=  { 0x00, 0xF0, 0xC0, 0x00 };
 int8_t const charStream6::shift1_[4]= {    2,   -4,   -2,    0 };
uint8_t const charStream6::shift2_[4]= {    0,    4,    6,    0 };


void
charStream6::add(uint64_t value) {
    ++n_;
    if (pos_==0)
        data_.push_back( value << shift1_[pos_] );
    else
        data_.back() |= (value >> -shift1_[pos_]);
    if (mask2_[pos_]!=0)
        data_.push_back( value << shift2_[pos_] );
    pos_= (pos_+1)%4;
}



void
charStream6::resetIter() {
    iter_= &data_[1];
    pos_= 0;
}



void
charStream6::setIter(uint32_t index) {
    iter_= &data_[1+(index*3)/4];
    pos_= index%4;
}



uint64_t
charStream6::getNextUnsafe() {
    uint64_t value= 0;
    uint8_t val= *iter_;
    if (shift1_[pos_]>0)
        value= (val & mask1_[pos_]) >> shift1_[pos_];
    else
        value= (val & mask1_[pos_]) << -shift1_[pos_];
    if (mask2_[pos_]!=0)
        value|= ( (*(iter_+1)) & mask2_[pos_] ) >> shift2_[pos_];
    iter_+= (pos_!=0);
    pos_= (pos_+1)%4;
    return value;
}



uint32_t
charStream6::numBytesForN(uint32_t n){
    return (n*3)/4 +1 +1;
}



void
charStream6::reserve(uint32_t n) {
    data_.reserve(numBytesForN(n));
}



void
charStream6::clear() {
    data_.resize(1);
    n_= 0;
    pos_= 0;
}



// apart from the 1 byte to store 3N%4, this is the computation:
// bytes= ceil(3N/4)= floor(3N/4)+ 3N%4!=0 = (3N - 3N%4)/4 + 3N%4!=0
void
charStream6::flush() const {
    data_[0]= static_cast<uint8_t>( (n_*3)%4 );
}



// so inverse calculation (apart form the 1 byte to store 3N%4):
// N= ( ( bytes - 3N%4!=0 ) * 4 + 3N%4 ) / 3
void
charStream6::computeNum() {
    ASSERT(data_[0]<4);
    n_= ( (data_.size()-1 - (data_[0]!=0) )*4 + data_[0] );
    ASSERT( n_%3==0 );
    n_/=3;
}



// ------------------------------------ charStream4

// organization: byte1=[val1, val2]
uint8_t const charStream4::mask_[2]=  { 0x0F, 0xF0 };
uint8_t const charStream4::shift_[2]= {    0,    4 };



void
charStream4::add(uint64_t value) {
    ++n_;
    if (isFirstHalf_)
        data_.push_back( value<<shift_[isFirstHalf_] );
    else
        data_.back() |= ( value<<shift_[isFirstHalf_] );
    isFirstHalf_= !isFirstHalf_;
}



void
charStream4::resetIter() {
    iter_= &data_[1];
    isFirstHalf_= true;
}



void
charStream4::setIter(uint32_t index) {
    iter_= &data_[1+index/2];
    isFirstHalf_= (index%2==0);
}



uint64_t
charStream4::getNextUnsafe() {
    uint64_t value= *iter_;
    isFirstHalf_= !isFirstHalf_;
    iter_+= isFirstHalf_;
    return (value & mask_[!isFirstHalf_]) >> shift_[!isFirstHalf_];
}




uint32_t
charStream4::numBytesForN(uint32_t n){
    return n*2 +1;
}



void
charStream4::reserve(uint32_t n) {
    data_.reserve(numBytesForN(n));
}



void
charStream4::clear() {
    data_.resize(1);
    n_= 0;
    isFirstHalf_= true;
}



void
charStream4::flush() const {
    data_[0]= static_cast<uint8_t>( n_%2 );
}



void
charStream4::computeNum() {
    ASSERT(data_[0]<2);
    n_= (data_.size()-1)*2-data_[0];
}






/*

unsigned
charStream12::getIth(uint32_t i) const {
    unsigned char *currData_= data+1 + i + i/2;
    if (i%2==0)
        return (static_cast<unsigned>(*currData_) << 4) | ( ( *(currData_+1) & 0xF0) >> 4 );
    else
        return (static_cast<unsigned>(*currData_ & 0x0F) << 8) | *(currData_+1);
}

unsigned
charStream10::getIth(uint32_t i) const {
    unsigned char pos= i%4;
    unsigned char *currData_= data+1 + i + i/4;
    unsigned value= static_cast<unsigned>(*currData_ & mask1[pos]) << lshift1[pos];
    value|= ( (*(currData_+1)) & mask2[pos] ) >> shift2[pos];
    currData_+= 1+(pos==3);
    return value;
}

unsigned
charStream6::getIth(uint32_t i) const {
    unsigned value= 0;
    unsigned char pos= i%4;
    unsigned char *currData_= data+1 + (i/4)*3 + i%4 - (i%4!=0);
    unsigned char val= *currData_;
    if (shift1[pos]>0)
        value= (val & mask1[pos]) >> shift1[pos];
    else
        value= (val & mask1[pos]) << -shift1[pos];
    if (mask2[pos]!=0)
        value|= ( (*(currData_+1)) & mask2[pos] ) >> shift2[pos];
    return value;
}

unsigned
charStream4::getIth(uint32_t i) const {
    return (data[1+i/2] & mask[1-i%2]) >> shift[1-i%2];
}

*/
