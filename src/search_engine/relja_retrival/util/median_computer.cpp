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

#include "median_computer.h"

#include <algorithm>
#include <math.h>

#include <iostream>
#include "macros.h"



medianComputer::medianComputer() : totalNum_(0) {
}



void
medianComputer::add(float value){
    if (hist_.size()==0 && values_.size()<256){
        
        // still enough memory to remember them all
        values_.push_back(value);
        
    } else if (hist_.size()==0) {
        
        // should switch to approximate
        min_= *std::min_element(values_.begin(), values_.end());
        max_= *std::max_element(values_.begin(), values_.end());
        
        hist_.resize(256, 0);
        totalNum_= 0;
        
        for (uint32_t i= 0; i<values_.size(); ++i)
            addQuantized(values_[i]);
        values_.clear();
        
        addQuantized(value);
        
    } else
        addQuantized(value);
}



void
medianComputer::addQuantized(float value){
    ++totalNum_;
    value= std::max( min_, std::min( max_, value ) ); // force the value to be inside
    ++hist_[ round( (value-min_)/(max_-min_) * 255 ) ];
}



float
medianComputer::getMedian(){
    if (values_.size()>0){
        
        // exact is possible
        std::nth_element(values_.begin(), values_.begin() + values_.size()/2, values_.end());
        if (values_.size()%2==1)
            return values_[values_.size()/2];
        else {
            std::nth_element(values_.begin(), values_.begin() + values_.size()/2-1, values_.end()); // NOTE: this line for bug fix was added on 25 Sep 2014, so hamming-related stuff indexed before this date has potential errors (all results are still valid, but maybe they can be even better)
            return (values_[values_.size()/2-1] + values_[values_.size()/2]) / 2;
        }
        
    } else {
        if (totalNum_==0)
            return 0.0;
        
        // approximate
        uint32_t iBin= 0, found= 0;
        for (; found<=totalNum_/2; ++iBin)
            found+= hist_[iBin];
        --iBin;
        
        float prop= static_cast<float>( totalNum_/2 - (found - hist_[iBin]) ) / hist_[iBin];
        ASSERT( prop > -1e-5 && prop < 1.0+1e-5 );
        return min_ + (prop+iBin)*(max_-min_)/256;
        
    }
}
