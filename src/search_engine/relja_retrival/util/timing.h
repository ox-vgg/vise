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

#ifndef _TIMING_H_
#define _TIMING_H_

#include <iostream>
#include <string>
#include <stdio.h>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <fstream>

#include <ctime>
#include <chrono>

#include "macros.h"



namespace timing {
  // updated for cross platform compatibility
  // Abhishek Dutta <adutta@robots.ox.ac.uk>, 12 March 2020
  inline double now(){
    double now_ms = (double) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return now_ms;
    /*
      struct timeval t;
      gettimeofday(&t, NULL);
      return 1000 * t.tv_sec + t.tv_usec/1000.0;
    */
  }

  inline double tic(){ return now(); }

  inline double toc( double prevTime ){ return now() - prevTime; }

  static inline std::string getTimeString(){
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(gmtime(&itt), "%FT%T");
    // Note: std::put_time() not implemented in < gcc-5
    //ss << std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() << " sec. since epoch";
    return ss.str();
  }



  inline std::string hrminsec(double seconds){
    int hr= std::max( 0.0, floor(seconds / (60*60)) );
    int min= std::max( 0.0, floor(seconds/60 - hr*60) );
    int sec= std::max( 0.0, floor(seconds - hr*60*60 - min*60) );
    char t[9];
    sprintf(t, "%.2d:%.2d:%.2d", hr, min, sec);
    return t;
  }



  class progressPrint {

  public:

    progressPrint(uint64_t nJobs, std::string prefix, std::ofstream *logf = nullptr, uint64_t numPrint= 10) :
      nJobs_(nJobs),
      printStep_( std::max(nJobs/numPrint, static_cast<uint64_t>(1)) ),
      totalDone_(0),
      prefix_(prefix),
      d_logf(logf) {
    }

    void inc(std::string extraInfo= "") {
      if (totalDone_==0) t1_= timing::tic();
      ++totalDone_;
      if (totalDone_<4 ||
          !(totalDone_ & (totalDone_-1)) || // power of 2
          totalDone_%printStep_==0 || totalDone_==nJobs_){
        if (totalDone_==1) {
          if(d_logf) {
            (*d_logf) << prefix_ << ":: " << timing::getTimeString()<<" 1 / "
                      << nJobs_ << std::endl;
          } else {
            std::cout << prefix_ << ":: " << timing::getTimeString()<<" 1 / "
                      << nJobs_ << std::endl;
          }
        }
        else {
          double time= timing::toc(t1_)/1000;
          double avgtime= time / (totalDone_-1);
          if(d_logf) {
            (*d_logf) << prefix_ << ":: ";
          } else {
            std::cout << prefix_ << ":: ";
          }
          if (extraInfo.length()>0) {
            if(d_logf) {
              (*d_logf) << extraInfo<<"; ";
            } else {
              std::cout << extraInfo<<"; ";
            }
          }
          if(d_logf) {
            (*d_logf) << timing::getTimeString()<<" "<<totalDone_<<" / "<<nJobs_
                      <<"; elapsed "<< timing::hrminsec( time )
                      <<"; remaining "<< timing::hrminsec( avgtime*(nJobs_-1)-time )
                      <<"; avg "<< avgtime <<" sec" << std::endl;
          } else {
            std::cout << timing::getTimeString()<<" "<<totalDone_<<" / "<<nJobs_
                      <<"; time "<< timing::hrminsec( time )
                      <<"; left "<< timing::hrminsec( avgtime*(nJobs_-1)-time )
                      <<"; avg "<< avgtime <<" sec" << std::endl;
          }
        }
      }
    }

    inline void inc(int64_t num){
      for (; num!=0; --num) {
        inc();
      }
    }

    inline uint64_t totalDone(){ return totalDone_; }

  private:
    uint64_t nJobs_, printStep_, totalDone_;
    std::string prefix_;
    double t1_;
    std::ofstream *d_logf;
    DISALLOW_COPY_AND_ASSIGN(progressPrint)
  };

};

#endif
