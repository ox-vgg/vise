#!/bin/bash
##
## Test VISE using Oxford Buildings dataset and ground truth files
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## Date: 23 Oct. 2020
##
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 VOC_SIZE HAMMING_BITS ASSET_DIR STORE_DIR" >&2
    echo "(example: $0 100000 32)" >&2
    exit 1
fi

VOC=$1
HAMM=$2
ASSET_DIR=$3
STORE_DIR=$4

export PNAME="_test_ox5k_voc${VOC}_hamm${HAMM}"
export PROJECT_DIR="${STORE_DIR}/${PNAME}"

if ! [ -d "${PROJECT_DIR}" ]; then
    mkdir -p $PROJECT_DIR
    mkdir -p $PROJECT_DIR/data $PROJECT_DIR/tmp
fi

## Download ox5k test images
if ! [ -f "${ASSET_DIR}/oxbuild_images.tgz" ]; then
    wget -O $ASSET_DIR/oxbuild_images.tgz https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/oxbuild_images.tgz
    mkdir $ASSET_DIR/image_src
    tar -xf $ASSET_DIR/oxbuild_images.tgz -C $ASSET_DIR/image_src
fi

## Download ox5k ground truth data
if ! [ -f "${ASSET_DIR}/gt_files_170407.tgz" ]; then
    wget -O $ASSET_DIR/gt_files_170407.tgz http://www.robots.ox.ac.uk/~vgg/data/oxbuildings/gt_files_170407.tgz
    mkdir -p $ASSET_DIR/gt/datasets/Oxford
    mkdir -p $ASSET_DIR/gt/rr_format
    tar -xf $ASSET_DIR/gt_files_170407.tgz -C $ASSET_DIR/gt/datasets/Oxford
fi

## create VISE configuration file
echo "Generating configuration ..."
echo "search_engine=relja_retrival" > $PROJECT_DIR/data/conf.txt
echo "bow_cluster_count=${VOC}" >> $PROJECT_DIR/data/conf.txt
echo "bow_descriptor_count=18000000" >> $PROJECT_DIR/data/conf.txt
echo "use_root_sift=true" >> $PROJECT_DIR/data/conf.txt
echo "sift_scale_3=true" >> $PROJECT_DIR/data/conf.txt
echo "cluster_num_iteration=10" >> $PROJECT_DIR/data/conf.txt
echo "hamm_embedding_bits=${HAMM}" >> $PROJECT_DIR/data/conf.txt
echo "resize_dimension=-1" >> $PROJECT_DIR/data/conf.txt
echo "preset_conf_id=preset_conf_manual" >> $PROJECT_DIR/data/conf.txt
echo "image_src_dir=${ASSET_DIR}/image_src/" >> $PROJECT_DIR/data/conf.txt
echo "image_dir=${ASSET_DIR}/image_src/" >> $PROJECT_DIR/data/conf.txt
echo "data_dir=${PROJECT_DIR}/data/" >> $PROJECT_DIR/data/conf.txt

## perform indexing
if ! [ -f "${PROJECT_DIR}/data/index_dset.bin" ] || ! [ -f "${PROJECT_DIR}/data/index_fidx.bin" ] || ! [ -f "${PROJECT_DIR}/data/index_iidx.bin" ] ; then
    if [ -f "${PWD}/vise/vise-cli" ]; then
        tstart=`date +%s`
        make -j16 && ./vise/vise-cli create-project $PNAME $PROJECT_DIR/data/conf.txt
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
        if ! [ -f "${PROJECT_DIR}/data/ox5k_perf_result.txt" ]; then
            tstart=`date +%s`
            search_engine/relja_retrival/v2/retrieval/tests/test_oxford_buildings $ASSET_DIR/gt/ $PROJECT_DIR/data/conf.txt $PROJECT_DIR/data/ox5k_perf_result.txt
            tend=`date +%s`
            elapsed=$((tend-tstart))
            echo "ox5k performance assessment completed in ${elapsed} sec."
        fi
    else
        echo "You must execute this script from $VISE_SOURCE/cmake_build/ folder"
        exit 1
    fi
fi

if [ -f "${PROJECT_DIR}/data/ox5k_perf_result.txt" ]; then
    while IFS= read -r line
    do
        echo "ox5k,${VOC},${HAMM},${IMSIZE},$line"
    done < "${PROJECT_DIR}/data/ox5k_perf_result.txt"
fi
