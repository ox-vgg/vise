# Compiling VISE

VISE source code can be download from its [gitlab code repository](https://gitlab.com/vgg/vise).
The VISE source is known to compile in GNU/Linux (gcc), Windows 10 (Visual C++) and MacOS (Clang).

## Compiling from Source in Linux and MacOS
These compilation instructions have been tested on the following platforms:
 * Ubuntu 18.04
 * MacOS Big Sur

```
export VISE_DIR=$HOME/vise          # change this to suit your preferences
export VISE_CODE=$HOME/vise/code
export VISE_DEP=$HOME/vise/dep

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

# download generic visual vocabulary to VISE application folder
mkdir -p $HOME/.vise/asset/relja_retrival/visual_vocabulary/
cd $HOME/.vise/asset/relja_retrival/visual_vocabulary/
wget https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629-imsize400x400-voc10k-hamm64.zip
unzip imcount53629-imsize400x400-voc10k-hamm64.zip
mv imcount53629-imsize400x400-voc10k-hamm64 latest
rm imcount53629-imsize400x400-voc10k-hamm64.zip

# create a symbolic link for VISE web application
ln -s $VISE_CODE/vise/src/www $HOME/.vise/www  # you can copy "www" files as well

cd $VISE_CODE/vise/cmake_build
./vise/vise-cli --cmd=web-ui        # start VISE server, the VISE web interface
                                    # is available at http://localhost:9669

./vise/vise-cli --help
```


## Building VISE using Docker
VISE can be built using docker as follows.

```
export VISE_DIR=$HOME/vise          # change this to suit your preferences
export VISE_CODE=$HOME/vise/code
export VISE_DATA=$HOME/vise/data

mkdir -p $VISE_CODE $VISE_DATA
cd $VISE_CODE
git clone https://gitlab.com/vgg/vise
cd $VISE_CODE/vise/

# build docker image
sudo docker build -t vise .

# show list of available docker images
sudo docker images
REPOSITORY   TAG       IMAGE ID       CREATED          SIZE
vise         latest    b6f63eb2dd0f   5 seconds ago    3.28GB
...

# run vise:latest docker image
sudo docker run \
  --volume $VISE_DATA/:/opt/vise/data/ \
  -p 9669:9669 \
  -i -t \
  vise:latest
```
Now you can access VISE graphical user interface in a web browser at http://localhost:9669.

***

Contact [Abhishek Dutta](mailto:adutta@robots.ox.ac.uk) for queries and feedback related to this page.
