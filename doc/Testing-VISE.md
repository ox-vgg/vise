# Testing VISE

VISE source code can be download from its [gitlab code repository](https://gitlab.com/vgg/vise)
and tested using some sample datasets based on the [Oxford Buildings](https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/) dataset.

## Quick Test using Oxford-Buildings-100 Subset
The [Oxford-Buildings-100]() contains 100 images taken from the original [Oxford Buildings](https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/) dataset which contains 5062. The VISE source code contains [`src/tests/test_oxford_buildings.cc`](https://gitlab.com/vgg/vise/-/blob/master/src/tests/test_oxford_buildings.cc) file which tests various parts of the VISE software using either the original Oxford Buildings dataset or the Oxford-Buildings-100 subset. Below, we show how to run these tests using the Oxford-Buildings-100 subset.

```
export VISE_DIR=$HOME/vise          # change this to suit your preferences
export VISE_CODE=$HOME/vise/code
export VISE_DEP=$HOME/vise/dep
export VISE_TEST=$HOME/vise/test

mkdir -p $VISE_CODE
mkdir -p $VISE_DEP
cd $VISE_CODE
git clone https://gitlab.com/vgg/vise
cd $VISE_CODE/vise/scripts/build/
# based on your platform, chose the command below
./make_deps_debian.sh $VISE_DEP     # compile VISE dependencies for Debian Linux
./make_deps_macos.sh $VISE_DEP      # compile VISE dependencies for MacOS Linux

# compile VISE
cd $VISE_CODE/vise/
mkdir cmake_build && cd cmake_build
$VISE_DEP/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$VISE_DEP ../src
make -j8                            # compile VISE

# download the Oxford-Buildings-100 subset
mkdir -p $VISE_TEST/data/
mkdir -p $VISE_TEST/temp/
cd $VISE_TEST/data/
wget https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/tests/oxford-buildings-100.zip
unzip oxford-buildings-100.zip

# run tests on oxford-buildings-100 subset
cd $VISE_CODE/vise/cmake_build
./tests/test_oxford_buildings \
  oxford-buildings-100 \  # test-id {oxford-buildings-100 or oxford-buildings-5k}
  $VISE_TEST/data/oxford-buildings-100/images \  # folder containing test images
  $VISE_TEST/data/oxford-buildings-100/gt \      # ground truth
  0.03 \                                         # mAP tolerance
  $VISE_TEST/temp/                               # temporary folder
```

We have run this test on Windows, Linux and MacOS machines and obtained the 
following image retrieval performance metrics.

```
+---------------+-----------------------------+---------+----------------------+
| Test Id.      | Machine                     | Time(s) | mAP (multiple runs)  |
+---------------+-----------------------------+---------+----------------------+
| Oxford-100    | Debian 10 Linux, 16 cores   | 248     | 0.8273, 0.8273       |
| Oxford-100    | Debian 10 Linux, 8 cores    | 442     | 0.8319, 0.8319,      |
| Oxford-100    | Windows 10, 8 cores         | 346     | 0.8448,0.8310,0.8339,|
| Oxford-100    | Mac mini 2020, 12 cores     | 628     | 0.8285               |
| Oxford-100    | MacBook Pro 2016, 4 cores   | 1015    | 0.8285               |
+---------------+-----------------------------+---------+----------------------+
```


***

Contact [Abhishek Dutta](mailto:adutta@robots.ox.ac.uk) for queries and feedback related to this page.