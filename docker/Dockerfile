# Docker build file for VGG Image Search Engine (VISE)
#
# Author  : Abhishek Dutta <adutta@robots.ox.ac.uk>
# Date    : 2022-08-18
# Website : https://robots.ox.ac.uk/~vgg/software/vise/
ARG NPROC=2

FROM debian:stable AS base

LABEL maintainer="Abhishek Dutta" \
  email="adutta@robots.ox.ac.uk" \
  authors="Abhishek Dutta, Prasanna Sridhar" \
  website="https://robots.ox.ac.uk/~vgg/software/vise/"

ENV VISE_CODE=/opt/vise/code \
  VISE_DEP=/opt/vise/dep

## prepare base environment common for both build and runtime
RUN deps="wget \
  ca-certificates \
  pkg-config \
  gosu \
  libgomp1 \
  libjpeg-dev \
  libpng-dev" && \
  apt-get -yq update && \
  apt-get install --no-install-recommends -y --quiet ${deps} && \
  apt-get autoremove -y && \
  apt-get clean -y && \
  rm -rf /var/lib/apt/lists/*

FROM base AS build
ARG NPROC
ENV NPROC=${NPROC:-2}
# Add build dependencies
RUN deps="cmake \
  build-essential" && \
  apt-get -yq update && \
  apt-get install --no-install-recommends -y --quiet ${deps} && \
  apt-get autoremove -y && \
  apt-get clean -y && \
  rm -rf /var/lib/apt/lists/*


# Build and Install Boost
FROM build AS boost-build
WORKDIR /3rdparty
RUN wget -q https://sourceforge.net/projects/boost/files/boost/1.73.0/boost_1_73_0.tar.gz/download && \
  tar -zxf boost_1_73_0.tar.gz && \
  cd boost_1_73_0 && \
  ./bootstrap.sh --prefix=$VISE_DEP --with-toolset=gcc --with-libraries=filesystem,system,thread && \
  ./b2 --with-filesystem --with-system --with-thread variant=release threading=multi toolset=gcc install

# Build and Install ImageMagick
FROM build AS imagemagick-build
WORKDIR /3rdparty
RUN wget -q -O ImageMagick6-6.9.12-58.tar.gz https://github.com/ImageMagick/ImageMagick6/archive/6.9.12-58.tar.gz && \
  tar -zxf ImageMagick6-6.9.12-58.tar.gz && \
  cd ImageMagick6-6.9.12-58 && \
  ./configure --prefix=$VISE_DEP -enable-hdri=no --with-quantum-depth=8 --disable-dependency-tracking --with-x=no --without-perl && \
  make -j$NPROC && \
  make install

# Build google protobuf library
FROM build AS protobuf-build
WORKDIR /3rdparty
RUN wget -q https://github.com/protocolbuffers/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz && \
  tar -zxf protobuf-2.6.1.tar.gz && \
  cd protobuf-2.6.1 && \
  ./configure --prefix=$VISE_DEP && \
  make -j$NPROC && \
  make install

# Build eigen library
FROM build AS eigen-build
WORKDIR /3rdparty
RUN wget -q -O eigen-3.4.0.tar.gz https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz && \
  tar -zxf eigen-3.4.0.tar.gz && \
  cd eigen-3.4.0/ && \
  cmake -H. -Bcmake_build -DCMAKE_INSTALL_PREFIX=$VISE_DEP ../ && \
  make -C cmake_build -j$NPROC && \
  make -C cmake_build install

# Build vlfeat library
FROM build AS vlfeat-build
WORKDIR /3rdparty
RUN wget -q http://www.vlfeat.org/download/vlfeat-0.9.21-bin.tar.gz && \
  tar -zxf vlfeat-0.9.21-bin.tar.gz && \
  cd vlfeat-0.9.21 && \
  sed -i 's/default(none)//g' vl/kmeans.c && \
  make -j$NPROC && \
  mkdir -p $VISE_DEP/lib && \
  mkdir -p $VISE_DEP/include/vl && \
  cp bin/glnxa64/libvl.so $VISE_DEP/lib/libvl.so && \
  cp -fr vl/*.* $VISE_DEP/include/vl/

# Build sqlite library
FROM build as sqlite-build
WORKDIR /3rdparty
RUN wget -q https://www.sqlite.org/2020/sqlite-autoconf-3330000.tar.gz && \
  tar -zxf sqlite-autoconf-3330000.tar.gz && \
  cd sqlite-autoconf-3330000 && \
  ./configure --prefix=$VISE_DEP && \
  make -j$NPROC && \
  make install

# Create dev environment
FROM build as dev

COPY --from=boost-build /opt/vise/dep/ /opt/vise/dep/
COPY --from=imagemagick-build /opt/vise/dep/ /opt/vise/dep/
COPY --from=protobuf-build /opt/vise/dep/ /opt/vise/dep/
COPY --from=eigen-build /opt/vise/dep/ /opt/vise/dep/
COPY --from=vlfeat-build /opt/vise/dep/ /opt/vise/dep/
COPY --from=sqlite-build /opt/vise/dep/ /opt/vise/dep/

COPY docker/entrypoint.sh /
ENTRYPOINT [ "/bin/bash", "/entrypoint.sh" ]

RUN deps="gdb \
  gdbserver" && \
  apt-get -yq update && \
  apt-get install --no-install-recommends -y --quiet ${deps} && \
  apt-get autoremove -y && \
  apt-get clean -y && \
  rm -rf /var/lib/apt/lists/*

WORKDIR $VISE_CODE

# Create build environment
FROM build as vise-build
COPY --from=dev /opt/vise/dep/ /opt/vise/dep/

# compile VISE
WORKDIR $VISE_CODE
COPY src src
RUN cmake -Hsrc -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$VISE_DEP && \
  make -C build -j$NPROC

FROM base as runtime

# Users can update these environment variables as required
#
# VISE_ASSET : generic visual vocabulary files, etc.
# VISE_WWW   : HTML/CSS/JS based minimal web based frontend for VISE
# VISE_PROJECT       : storage for all the VISE projects
# VISE_HTTP_PORT     : bind http server to this port
# VISE_HTTP_ADDRESS  : bind http server to this address
# VISE_HTTP_WORKER   : number of workers for http server
# VISE_NTHREAD_INDEX : number of threads for indexing
# VISE_NTHREAD_SEARCH  : number of threads for visual search

ENV VISE_ASSET=/opt/vise/data/asset \
  VISE_WWW=/opt/vise/data/www \
  VISE_PROJECT=/opt/vise/project \
  VISE_HTTP_PORT=9669 \
  VISE_HTTP_ADDRESS=0.0.0.0 \
  VISE_HTTP_NAMESPACE=/ \
  VISE_HTTP_WORKER=8 \
  VISE_NTHREAD_INDEX=8 \
  VISE_NTHREAD_SEARCH=1


COPY --from=vise-build /opt/vise/code/build /opt/vise/code/build
COPY --from=vise-build /opt/vise/dep/ /opt/vise/dep/

COPY src/www /opt/vise/data/www

## download generic visual vocabulary of VISE
WORKDIR $VISE_ASSET/relja_retrival/visual_vocabulary
RUN wget -q https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629-imsize400x400-voc10k-hamm64.tar.gz && \
  tar -zxf imcount53629-imsize400x400-voc10k-hamm64.tar.gz && \
  mv imcount53629-imsize400x400-voc10k-hamm64 latest && \
  cp latest/conf.txt latest/generic_visual_vocab_conf.txt && \
  rm imcount53629-imsize400x400-voc10k-hamm64.tar.gz && \
  mkdir -p $VISE_PROJECT

## execute VISE
EXPOSE 9669/tcp

WORKDIR $VISE_CODE
CMD build/vise/vise-cli \
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
