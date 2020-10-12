#!/bin/bash
##
## Test VISE using Oxford Buildings dataset and ground truth files
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## Date: 18 Aug. 2020
##
if [ "$#" -ne 5 ]; then
    echo "Usage: $0 VOC_SIZE HAMMING_BITS IMAGE_SIZE ASSET_DIR STORE_DIR" >&2
    echo "(example: $0 100k 32 800x800)" >&2
    exit 1
fi

VOC=$1
HAMM=$2
IMSIZE=$3
ASSET_DIR=$4
STORE_DIR=$5

export PNAME="_test_ox5k_voc${VOC}_hamm${HAMM}_${IMSIZE}"
export PROJECT_DIR="${STORE_DIR}/${PNAME}"
export GENERIC_VISUAL_VOC_URL=http://www.robots.ox.ac.uk/~vgg/software/vise/_internal/test/generic_visual_vocabulary

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

## copy generic visual vocabulary
if ! [ -f "${PROJECT_DIR}/data/bowcluster.bin" ] || ! [ -f "${PROJECT_DIR}/data/trainhamm.bin" ]; then
    ZIP_FILENAME="voc${VOC}_hamm${HAMM}_${IMSIZE}.zip"
    wget -O $PROJECT_DIR/data/$ZIP_FILENAME $GENERIC_VISUAL_VOC_URL/$ZIP_FILENAME
    unzip $PROJECT_DIR/data/$ZIP_FILENAME -d $PROJECT_DIR/data/
fi

## create VISE configuration file
echo "Generating configuration ..."
echo "search_engine=relja_retrival" > $PROJECT_DIR/data/conf.txt
echo "use_root_sift=true" >> $PROJECT_DIR/data/conf.txt
echo "sift_scale_3=true" >> $PROJECT_DIR/data/conf.txt
echo "cluster_num_iteration=30" >> $PROJECT_DIR/data/conf.txt
echo "hamm_embedding_bits=${HAMM}" >> $PROJECT_DIR/data/conf.txt
echo "resize_dimension=-1" >> $PROJECT_DIR/data/conf.txt
echo "preset_conf_id=preset_conf_manual" >> $PROJECT_DIR/data/conf.txt
echo "image_src_dir=${ASSET_DIR}/image_src/" >> $PROJECT_DIR/data/conf.txt
echo "image_dir=${ASSET_DIR}/image_src/" >> $PROJECT_DIR/data/conf.txt
echo "data_dir=${PROJECT_DIR}/data/" >> $PROJECT_DIR/data/conf.txt
cat $PROJECT_DIR/data/generic_visual_vocab_conf.txt | grep "bow_cluster_count" >> $PROJECT_DIR/data/conf.txt
cat $PROJECT_DIR/data/generic_visual_vocab_conf.txt | grep "bow_descriptor_count" >> $PROJECT_DIR/data/conf.txt

## perform indexing
if ! [ -f "${PROJECT_DIR}/data/index_dset.bin" ] || ! [ -f "${PROJECT_DIR}/data/index_fidx.bin" ] || ! [ -f "${PROJECT_DIR}/data/index_iidx.bin" ] ; then
    if [ -f "${PWD}/vise/vise" ]; then
        tstart=`date +%s`
        make -j16 && ./vise/vise create-project $PNAME $PROJECT_DIR/data/conf.txt
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
