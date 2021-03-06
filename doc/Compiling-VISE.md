# Compiling VISE from Source

VISE source code can be download from its [gitlab code repository](https://gitlab.com/vgg/vise).
The VISE source is known to compile in GNU/Linux (gcc), Windows 10 (Visual C++) and MacOS (Clang).

## Compiling from Source in Linux
```
export VISE_DIR=$HOME/vise/code     # change this to suit your requirements
export VISE_DEP_DIR=$HOME/vise/dep  # change this to suit your requirements

mkdir -p $VISE_DIR
mkdir -p $VISE_DEP_DIR
cd $VISE_DIR
git clone https://gitlab.com/vgg/vise
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

# download generic visual vocabulary to VISE application folder
mkdir -p $HOME/.vise/asset/relja_retrival/visual_vocabulary/
cd $HOME/.vise/asset/relja_retrival/visual_vocabulary/
wget https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629-imsize400x400-voc10k-hamm32.zip
unzip imcount53629-imsize400x400-voc10k-hamm32.zip
mv imcount53629-imsize400x400-voc10k-hamm32 latest
rm imcount53629-imsize400x400-voc10k-hamm32.zip

# create a symbolic link for VISE web application
cd $HOME/.vise
ln -s $VISE_DIR/vise/src/www www

cd $VISE_DIR/vise/cmake_build
mkdir -p $HOME/.vise/asset/relja_retrival/visual_vocabulary/latest
./vise/vise-cli --cmd=web-ui        # start VISE server, the VISE web interface
                                    # is available at http://localhost:9669
```

## Compiling from Source in MacOS (Big Sur)
```
export VISE_DIR=$HOME/vise/code     # change this to suit your requirements
export VISE_DEP_DIR=$HOME/vise/dep  # change this to suit your requirements

mkdir -p $VISE_DIR
mkdir -p $VISE_DEP_DIR
cd $VISE_DIR
git clone https://gitlab.com/vgg/vise
cd $VISE_DIR/vise/scripts/build/
./make_deps_macos.sh $VISE_DEP_DIR  # compile and install VISE dependencies

# compile VISE
cd $VISE_DIR/vise/
mkdir cmake_build && cd cmake_build
$VISE_DEP_DIR/bin/cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$VISE_DEP_DIR \
  ../src
make -j8                            # compile VISE

# download generic visual vocabulary to VISE application folder
mkdir -p $HOME/.vise/asset/relja_retrival/visual_vocabulary/
cd $HOME/.vise/asset/relja_retrival/visual_vocabulary/
wget https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629-imsize400x400-voc10k-hamm32.zip
unzip imcount53629-imsize400x400-voc10k-hamm32.zip
mv imcount53629-imsize400x400-voc10k-hamm32 latest
rm imcount53629-imsize400x400-voc10k-hamm32.zip

# create a symbolic link for VISE web application
cd $HOME/.vise
ln -s $VISE_DIR/vise/src/www www

cd $VISE_DIR/vise/cmake_build
./vise/vise-cli --cmd=web-ui        # start VISE server, the VISE web interface
                                    # is available at http://localhost:9669
```
