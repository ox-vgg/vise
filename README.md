# VGG Image Search Engine (VISE)

VGG Image Search Engine (VISE) is a free and [open source](https://gitlab.com/vgg/vise) software for visual search of large collection of images using image region as a search query. VISE is developed and maintained by Visual Geometry Group ([VGG](https://www.robots.ox.ac.uk/~vgg/)) in Department of Engineering Science of the Oxford University. VISE is released under a [license](https://gitlab.com/vgg/vise/-/blob/master/src/LICENSE.txt) that allows unrestricted use in academic research projects and commercial industrial applications.

VISE builds on the C++ codebase developed by [Relja Arandjelovic](http://www.robots.ox.ac.uk/~relja/) during his DPhil / Postdoc at the Visual Geometry Group in 2014. VISE is developed and maintained by [Abhishek Dutta](https://www.robots.ox.ac.uk/~adutta/) under the supervision of [Prof. Andrew Zisserman](https://www.robots.ox.ac.uk/~az/).

Development and maintenance of VISE software has been supported by the following two grants:
 * Visual AI: An Open World Interpretable Visual Transformer (UKRI Grant [EP/T028572/1](https://gtr.ukri.org/projects?ref=EP%2FT028572%2F1))
 * Seebibyte: Visual Search for the Era of Big Data (EPSRC Grant [EP/M013774/1](https://gow.epsrc.ukri.org/NGBOViewGrant.aspx?GrantRef=EP/M013774/1))

For more details, visit https://www.robots.ox.ac.uk/~vgg/software/vise/

## Compiling from Source in Linux
```
export VISE_DIR=$HOME/vise/code     # change this to suit your requirements
export VISE_DEP_DIR=$HOME/vise/dep  # change this to suit your requirements

mkdir -p $VISE_DIR
cd $VISE_DIR
git clone git@gitlab.com:vgg/vise.git
cd $VISE_DIR/vise/scripts/build/
./make_deps_debian.sh $VISE_DEP_DIR # compile and install VISE dependencies

# compile VISE
cd $VISE_DIR/vise/
mkdir cmake_build && cd cmake_build
$VISE_DEP_DIR/bin/cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$VISE_DEP_DIR \
  -DVLFEAT_LIB=$VISE_DEP_DIR/_tmp_libsrc/vlfeat-0.9.21/bin/glnxa64/libvl.so \
  -DVLFEAT_INCLUDE_DIR=$VISE_DEP_DIR/_tmp_libsrc/vlfeat-0.9.21/ \
  ../src
make -j8                            # compile VISE

cd $VISE_DIR/vise/cmake_build
./vise/vise                         # start VISE server, the VISE web interface
                                    # is available at http://localhost:9669
```

## Command Line Mode (for Advanced Users)
```
cd /data/vise/demo/oxford-buildings # create a folder to store all project files
mkdir data image image_src tmp      # these subfolders are used by VISE
touch data/conf.txt                 # manually create conf.txt as shown below
cat data/conf.txt
  bow_cluster_count=100000
  bow_descriptor_count=3288217
  cluster_num_iteration=10
  hamm_embedding_bits=64
  preset_conf_id=preset_conf_manual
  resize_dimension=800x800
  search_engine=relja_retrival
  sift_scale_3=true
  use_root_sift=true

# download oxford buildings images to "image_src" folder
cd image_src
wget https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/oxbuild_images.tgz
tar -zxvf oxbuild_images.tgz # extract all images to "image_src" folder
rm oxbuild_images.tgz        # no longer needed

# create visual search index
./vise/vise-cli --run-mode=create-project --nthread=2 \
  oxford-buildings:/data/vggdemos/vise/www/vise/oxford-buildings/data/conf.txt

# make visual search engine web interface 
# available at http://localhost:9670/vise/demo/
./vise/vise-cli --run-mode=serve-project \
  --port=9670 --nthread=4 --address=0.0.0.0 --http_uri_namespace=/vise/demo/ \
  oxford-buildings:/data/oxford-buildings/data/conf.txt
```

## HTTP API
See [[doc/vise_http_api.txt]]

## Contact
Contact [Abhishek Dutta](mailto:adutta _at_ robots.ox.ac.uk) for any queries or feedback related to this software application.
