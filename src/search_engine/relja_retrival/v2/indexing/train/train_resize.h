/** @file train_resize.h
 *  @brief resize images in parallel
 *  @author Abhishek Dutta
 *  @date 24 Aug. 2020
 */

#ifndef _TRAIN_RESIZE_H_
#define _TRAIN_RESIZE_H_

#include <stdint.h>
#include <string>
#include <omp.h>
#include <fstream>

#include <boost/filesystem.hpp>

#include <Magick++.h>

#include "vise/task_progress.h"

namespace buildIndex {

    void
    computeTrainResize(boost::filesystem::path const image_src_dir,
                       boost::filesystem::path const image_dir,
                       boost::filesystem::path const filelist_fn,
                       boost::filesystem::path const filestat_fn,
                       std::string resize_dimension,
                       std::ofstream& logf,
                       vise::task_progress *progress = nullptr);
}

#endif
