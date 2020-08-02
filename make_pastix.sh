#!/bin/bash
if ! [[ -d build ]]; then
    mkdir build
fi
cd build

INSTALLPATH="~/CalculiX/PaStiX/pastix"
#CUDADIR="/usr/lib/cuda"
PARSECDIR="~/CalculiX/PaStiX/parsec"
SCOTCHDIR="~/CalculiX/PaStiX/scotch"
HWLOCDIR="~/CalculiX/PaStiX/hwloc"
BLASDIR="~/CalculiX/PaStiX/openblas"

cmake \
    -DBLAS_DIR=${BLASDIR} \
    -DHWLOC_DIR=${HWLOCDIR} \
	-DCMAKE_INSTALL_PREFIX=${INSTALLPATH} \
	-DCMAKE_BUILD_TYPE=Debug \
	-DPASTIX_WITH_PARSEC=ON \
	-DPARSEC_DIR=${PARSECDIR} \
	-DSCOTCH_DIR=${SCOTCHDIR} \
	-DPASTIX_WITH_CUDA=OFF \
	-DPASTIX_ORDERING_SCOTCH=ON \
	-DCMAKE_C_COMPILER=gcc \
	-DCMAKE_CXX_COMPILER=g++ \
	-DCMAKE_Fortran_COMPILER=gfortran \
	-DCMAKE_C_FLAGS="-fopenmp" \
    ..

make -j8
make install
