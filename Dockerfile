# Docker build file for VGG Image Search Engine (VISE)
#
# Author  : Abhishek Dutta <adutta@robots.ox.ac.uk>
# Date    : 2022-08-18
# Website : https://robots.ox.ac.uk/~vgg/software/vise/

FROM debian:stable

MAINTAINER Abhishek Dutta, adutta@robots.ox.ac.uk, https://robots.ox.ac.uk/~vgg/software/vise/

# Users can update these environment variables as required
#
# VISE_ASSET : generic visual vocabulary files, etc.
# VISE_WWW   : HTML/CSS/JS based minimal web based frontend for VISE
# VISE_CODE  : source code and build files
# VISE_DEP   : VISE dependency libraries
# VISE_PROJECT       : storage for all the VISE projects
# VISE_HTTP_PORT     : bind http server to this port
# VISE_HTTP_ADDRESS  : bind http server to this address
# VISE_HTTP_WORKER   : number of workers for http server
# VISE_NTHREAD_INDEX : number of threads for indexing
# VISE_NTHREAD_SEARCH  : number of threads for visual search

ENV VISE_ASSET=/opt/vise/data/asset \
    VISE_WWW=/opt/vise/data/www \
    VISE_PROJECT=/opt/vise/project \
    VISE_CODE=/opt/vise/code \
    VISE_DEP=/opt/vise/dep \
    VISE_HTTP_PORT=9669 \
    VISE_HTTP_ADDRESS=0.0.0.0 \
    VISE_HTTP_NAMESPACE=/ \
    VISE_HTTP_WORKER=8 \
    VISE_NTHREAD_INDEX=8 \
    VISE_NTHREAD_SEARCH=1

## prepare environment
RUN apt-get update \
  && apt-get install --no-install-recommends -y --quiet wget cmake g++ gcc make ca-certificates pkg-config libjpeg-dev libpng-dev \
  && mkdir -p $VISE_DEP/src \
  \
  ## build boost library
  && cd $VISE_DEP/src/ \
  && wget -q https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.gz \
  && tar -zxvf boost_1_73_0.tar.gz && cd boost_1_73_0 \
  && ./bootstrap.sh --prefix=$VISE_DEP --with-toolset=gcc --with-libraries=filesystem,system,thread \
  && ./b2 --with-filesystem --with-system --with-thread variant=release threading=multi toolset=gcc install \
  && cd $VISE_DEP/src/ \
  && wget -q -O ImageMagick6-6.9.12-58.tar.gz https://github.com/ImageMagick/ImageMagick6/archive/6.9.12-58.tar.gz \
  && tar -zxvf ImageMagick6-6.9.12-58.tar.gz && cd ImageMagick6-6.9.12-58 \
  && ./configure --prefix=$VISE_DEP -enable-hdri=no --with-quantum-depth=8 --disable-dependency-tracking --with-x=no --without-perl \
  && make -j && make install \
  \
  ## build google protobuf library
  && cd $VISE_DEP/src/ \
  && wget -q https://github.com/protocolbuffers/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz \
  && tar -zxvf protobuf-2.6.1.tar.gz && cd protobuf-2.6.1 \
  && ./configure --prefix=$VISE_DEP && make -j && make install \
  \
  ## build eigen library
  && cd $VISE_DEP/src/ \
  && wget -q -O eigen-3.4.0.tar.gz https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz \
  && tar -zxvf eigen-3.4.0.tar.gz && cd eigen-3.4.0/ && mkdir cmake_build && cd cmake_build \
  && cmake -DCMAKE_INSTALL_PREFIX=$VISE_DEP ../ && make -j && make install \
  \
  ## build vlfeat library
  && cd $VISE_DEP/src/ \
  && wget -q http://www.vlfeat.org/download/vlfeat-0.9.21-bin.tar.gz \
  && tar -zxvf vlfeat-0.9.21-bin.tar.gz && cd vlfeat-0.9.21 \
  && sed -i 's/default(none)//g' vl/kmeans.c \
  && make -j && cp bin/glnxa64/libvl.so $VISE_DEP/lib/libvl.so \
  && mkdir $VISE_DEP/include/vl && cp -fr vl/*.* $VISE_DEP/include/vl/ \
  \
  ## build sqlite library
  && cd $VISE_DEP/src/ \
  && wget -q https://www.sqlite.org/2020/sqlite-autoconf-3330000.tar.gz \
  && tar -zxvf sqlite-autoconf-3330000.tar.gz && cd sqlite-autoconf-3330000 \
  && ./configure --prefix=$VISE_DEP && make -j && make install \
  \
  ## compile VISE
  && mkdir -p $VISE_CODE && cd $VISE_CODE \
  && wget -q -O /tmp/vise-2.0.1.tar.gz https://gitlab.com/vgg/vise/-/archive/vise-2.0.1/vise-vise-2.0.1.tar.gz \
  && tar -zxf /tmp/vise-2.0.1.tar.gz && mv vise-vise-2.0.1/* $VISE_CODE/ \
  && rm -fr vise-vise-2.0.1 && rm /tmp/vise-2.0.1.tar.gz \
  && mkdir build && cd build \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$VISE_DEP ../src \
  && make -j \
  \
  ## download generic visual vocabulary of VISE
  && mkdir -p $VISE_WWW && cp -fr $VISE_CODE/src/www/* $VISE_WWW \
  && mkdir -p $VISE_ASSET/relja_retrival/visual_vocabulary/ \
  && cd $VISE_ASSET/relja_retrival/visual_vocabulary/ \
  && wget -q https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629-imsize400x400-voc10k-hamm64.tar.gz \
  && tar -zxf imcount53629-imsize400x400-voc10k-hamm64.tar.gz && mv imcount53629-imsize400x400-voc10k-hamm64 latest \
  && cp latest/conf.txt latest/generic_visual_vocab_conf.txt \
  && rm imcount53629-imsize400x400-voc10k-hamm64.tar.gz \
  && mkdir -p $VISE_PROJECT
  

## execute VISE
EXPOSE 9669/tcp

CMD /opt/vise/code/build/vise/vise-cli \
  --cmd=web-ui \
  --http-address=$VISE_HTTP_ADDRESS \
  --http-port=$VISE_HTTP_PORT \
  --http-worker=$VISE_HTTP_WORKER \
  --http-namespace=$VISE_HTTP_NAMESPACE \
  --http-www-dir=$VISE_WWW \
  --vise-home-dir=$VISE_HOME \
  --vise-project-dir=$VISE_PROJECT \
  --vise-asset-dir=$VISE_ASSET/ \
  --nthread-index=$VISE_NTHREAD_INDEX \
  --nthread-search=$VISE_NTHREAD_SEARCH
