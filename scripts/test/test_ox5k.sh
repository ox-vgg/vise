#!/bin/bash
##
## Test VISE using Oxford Buildings dataset and ground truth files
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## Date: 18 Aug. 2020
##
## Test Results
## |-------------------+------+------+----------+-------------|
## | Visual Vocabulary | Size | Hamm | mAP:hamm | mAP:hamm+sp |
## |-------------------+------+------+----------+-------------|
## | ox5k              | 100k |   64 |  0.47344 |      0.5567 |
## | generic-40180     | 100k |   64 |   0.4198 |      0.5064 |
## | generic-40180     | 50k  |   64 |   0.4087 |      0.5067 |
## | generic-40180     | 50k  |   32 |   0.4381 |   ** 0.5275 |
## | generic-40180     | 10k  |   64 |   0.3771 |      0.4966 |
## |-------------------+------+------+----------+-------------|
##
## Note:
## generic-40180 : includes 40180 images from artuk, bl1m, mitplaces205, mscoc
## generic-?     : includes 1000000 images from artuk, bl1m, mitplaces205, mscoc, nlschapbooks-subset, ox5k, 15cI
##
export PNAME=_test_ox5k
export PROJECT_DIR=$HOME/.vise/store/$PNAME
export GENERIC_VVOC_DIR=/home/tlm/.vise/asset/visual_vocabulary/voc50k_hamm32/
if ! [ -d "${PROJECT_DIR}" ]; then
  mkdir -p $PROJECT_DIR
  mkdir -p $PROJECT_DIR/data $PROJECT_DIR/image_src $PROJECT_DIR/image $PROJECT_DIR/tmp
fi

## Download ox5k test images
if ! [ -f "${PROJECT_DIR}/tmp/oxbuild_images.tgz" ]; then
  wget -O $PROJECT_DIR/tmp/oxbuild_images.tgz https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/oxbuild_images.tgz
  tar -xf $PROJECT_DIR/tmp/oxbuild_images.tgz -C $PROJECT_DIR/image_src
fi

## Download ox5k ground truth data
if ! [ -f "${PROJECT_DIR}/tmp/gt_files_170407.tgz" ]; then
  wget -O $PROJECT_DIR/tmp/gt_files_170407.tgz http://www.robots.ox.ac.uk/~vgg/data/oxbuildings/gt_files_170407.tgz
  mkdir -p $PROJECT_DIR/tmp/gt/datasets/Oxford
  mkdir -p $PROJECT_DIR/tmp/gt/rr_format
  tar -xf $PROJECT_DIR/tmp/gt_files_170407.tgz -C $PROJECT_DIR/tmp/gt/datasets/Oxford
fi

## copy generic visual vocabulary 
if ! [ -f "${PROJECT_DIR}/data/bowcluster.bin" ] || ! [ -f "${PROJECT_DIR}/data/trainhamm.bin" ]; then
  wget -O $PROJECT_DIR/tmp/generic_visual_voc50k_hamm32.zip https://www.robots.ox.ac.uk/~vgg/software/vise/_internal/test/asset/generic_visual_voc50k_hamm32.zip
  unzip $PROJECT_DIR/tmp/generic_visual_voc50k_hamm32.zip -d $PROJECT_DIR/data/
fi

## create VISE configuration file
echo "search_engine=relja_retrival" > $PROJECT_DIR/data/conf.txt
echo "use_root_sift=true" >> $PROJECT_DIR/data/conf.txt
echo "sift_scale_3=true" >> $PROJECT_DIR/data/conf.txt
echo "bow_descriptor_count=18000000" >> $PROJECT_DIR/data/conf.txt
echo "bow_cluster_count=50000" >> $PROJECT_DIR/data/conf.txt
echo "cluster_num_iteration=30" >> $PROJECT_DIR/data/conf.txt
echo "hamm_embedding_bits=32" >> $PROJECT_DIR/data/conf.txt
echo "resize_dimension=-1" >> $PROJECT_DIR/data/conf.txt
echo "preset_conf_id=preset_conf_manual" >> $PROJECT_DIR/data/conf.txt

## perform indexing
if ! [ -f "${PROJECT_DIR}/data/index_dset.bin" ] || ! [ -f "${PROJECT_DIR}/data/index_fidx.bin" ] || ! [ -f "${PROJECT_DIR}/data/index_iidx.bin" ] ; then
  if [ -f "${PWD}/vise/vise" ]; then
    tstart=`date +%s`
    make -j16 && ./vise/vise create-project _test_ox5k $PROJECT_DIR/data/conf.txt
    tend=`date +%s`
    elapsed=$((tend-tstart))
    echo "ox5k indexing completed in ${elapsed} sec." > $PROJECT_DIR/tmp/ox5k_perf_log.txt
  else
    echo "You must execute this script from $VISE_SOURCE/cmake_build/ folder"
    exit 1    
  fi
else
  ## assess performance using ox5k ground truth
  if [ -f "${PWD}/search_engine/relja_retrival/v2/retrieval/tests/test_oxford_buildings" ]; then
    if ! [ -f "${PROJECT_DIR}/tmp/ox5k_perf_result.txt" ]; then
      tstart=`date +%s`
      search_engine/relja_retrival/v2/retrieval/tests/test_oxford_buildings $PROJECT_DIR/tmp/gt/ $PROJECT_DIR/data/conf.txt $PROJECT_DIR/tmp/ox5k_perf_result.txt
      tend=`date +%s`
      elapsed=$((tend-tstart))
      echo "ox5k performance assessment completed in ${elapsed} sec."
    fi
  else
    echo "You must execute this script from $VISE_SOURCE/cmake_build/ folder"
    exit 1    
  fi
fi

if [ -f "${PROJECT_DIR}/tmp/ox5k_perf_result.txt" ]; then
  while IFS= read -r line
  do
    echo "$line"
  done < "${PROJECT_DIR}/tmp/ox5k_perf_result.txt"
fi