#
#  @file Dockerfile
#
#  @copyright 2024-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @version 6.4.0
#  @author Florent Pruvost
#  @date 2024-11-29
#
# This docker image is used to test the package during gitlab-ci pipelines.
# It should be stored in the gitlab's project container registry:
# https://gitlab.inria.fr/solverstack/pastix/container_registry
#
FROM registry.gitlab.inria.fr/solverstack/spm

USER root

# Pastix dependencies: add metis and ptscotch which is not in spm image
RUN apt-get -y upgrade --no-install-recommends \
    libmetis-dev \
    libptscotch-dev

# Starpu dependencies
RUN apt-get -y upgrade --no-install-recommends \
    autoconf \
    automake \
    libtool-bin \
    libfxt-dev \
    libhwloc-dev \
    libopenmpi-dev

RUN apt-get autoremove -y
RUN apt-get autoclean -y
RUN apt-get purge -y

RUN update-ca-certificates

# Install PARSEC shm version
RUN git clone https://bitbucket.org/mfaverge/parsec.git && \
    cd parsec && \
    git checkout mymaster && \
    git checkout 6022a61dc96c25f11dd2aeabff2a5b3d7bce867d && \
    git submodule update && \
    mkdir -p build-shm && \
    cd build-shm && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/home/gitlab/install/parsec-shm -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DPARSEC_GPU_WITH_CUDA=OFF && \
    make -j5 && \
    make install

# Install PARSEC mpi version
RUN cd /home/gitlab/parsec && \
    mkdir -p build-mpi && \
    cd build-mpi && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/home/gitlab/install/parsec-mpi -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DPARSEC_GPU_WITH_CUDA=OFF -DPARSEC_DIST_WITH_MPI=ON && \
    make -j5 && \
    make install && \
    cd ../../ && \
    rm parsec/ -rf

# Install StarPU
RUN wget https://files.inria.fr/starpu/starpu-1.4.7/starpu-1.4.7.tar.gz > /dev/null 2>&1 && \
    tar xzf starpu-1.4.7.tar.gz && \
    cd starpu-1.4.7/ && \
    ./configure --prefix=/home/gitlab/install/starpu --enable-debug --disable-build-doc --with-fxt --disable-simgrid && \
    make -j5 install && \
    cd ../ && \
    rm starpu-1.4.7/ -rf && \
    rm starpu-1.4.7.tar.gz

USER gitlab

# Set some default environment variables
ENV STARPU_DIR /home/gitlab/install/starpu
ENV PATH $STARPU_DIR/bin:$PATH
ENV LD_LIBRARY_PATH $STARPU_DIR/lib:$LD_LIBRARY_PATH
ENV PKG_CONFIG_PATH $STARPU_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
