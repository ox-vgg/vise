# User Guide

VISE software has a graphical user interface (web browser based) and a command line interface (using `vise-cli` command). This document describes usage of these two interfaces. 


## Using the `vise-cli` command line tool
Here, we show an example of creating a visual search engine using the 5062 images 
contained in the [Oxford Buildings dataset](https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/).


```
# Assumption: the following environment variables correctly defined, i.e.
# VISE_DIR  : folder where we store all our data (e.g. /home/tlm/vise)
# VISE_CODE : VISE source code is stored in $VISE_CODE/vise/ and compiled 
#             binaries are stored in $VISE_CODE/vise/cmake_build/
# VISE_DEP  : VISE dependencies are compiled and installed in this folder
#
# Tested in Ubuntu 18.04

export VISE_DIR=$HOME/vise
export VISE_CODE=$HOME/vise/code
export VISE_DEP=$HOME/vise/dep

## 1. Compile VISE dependencies
mkdir -p $VISE_CODE $VISE_DEP
cd $VISE_CODE
git clone https://gitlab.com/vgg/vise
cd $VISE_CODE/vise/scripts/build/
./make_deps_debian.sh $VISE_DEP     # for Debian and Ubuntu based systems
./make_deps_macos.sh $VISE_DEP      # for MacOS systems

## 2. Compile VISE software
cd $VISE_CODE/vise/
mkdir cmake_build && cd cmake_build
$VISE_DEP/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$VISE_DEP ../src
make -j
$VISE_CODE/vise/cmake_build/vise/vise-cli --help

## 3. Prepare VISE Project (using sample images from Oxford Buildings dataset)
mkdir -p $VISE_DIR/projects/oxford-buildings/
mkdir -p $VISE_DIR/projects/oxford-buildings/data
mkdir -p $VISE_DIR/projects/oxford-buildings/image_src

cd $VISE_DIR/projects/oxford-buildings/image_src
wget https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/oxbuild_images.tgz
tar -zxvf oxbuild_images.tgz # 5062 JPG files get extracted

## Create project configuration file "conf.txt" and
## define configuration as follows: 
cd $VISE_DIR/projects/oxford-buildings/data
nano conf.txt  ## create a file named "conf.txt" with the following content
bow_cluster_count=100000
bow_descriptor_count=20000000
cluster_num_iteration=10
hamm_embedding_bits=64
nthread-indexing=16
preset_conf_id=preset_conf_manual
resize_dimension=-1
search_engine=relja_retrival
sift_scale_3=true
use_root_sift=true

$VISE_CODE/vise/cmake_build/vise/vise-cli --cmd=create-project \
  oxford-buildings:$VISE_DIR/projects/oxford-buildings/data/conf.txt

# Note: the above command will run for a long time (e.g. 30 min.)
# you can review the progress in the following log file
# $ tail -f $VISE_DIR/projects/oxford-buildings/data/index.log

## 4. Serve the project's visual search engine over a web interface
$VISE_CODE/vise/cmake_build/vise/vise-cli --cmd=serve-project \
  --http-port=9669 --http-namespace=/ \
  --http-www-dir=$VISE_CODE/vise/src/www \
  oxford-buildings:$VISE_DIR/projects/oxford-buildings/data/conf.txt
  
# Note: Now open http://localhost:9669/oxford-buildings/ in a
# web browser to access the visual search engine.
```

## Graphical User Interface (web browser based)
<i>coming soon ...</i>


***

You may also want to look at the [Frequently Asked Questions](FAQ.md) page. Contact [Abhishek Dutta](mailto:adutta@robots.ox.ac.uk) for queries and feedback related to this page.
