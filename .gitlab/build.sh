#!/usr/bin/env bash
###
#
#  @file build.sh
#  @copyright 2023-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @version 6.4.0
#  @author Mathieu Faverge
#  @author Florent Pruvost
#  @date 2024-07-05
#
###

fatal() {
    echo "$0: error occurred, exit"
    exit 1
}

set -x

#
# Build the project
#
if [[ "$SYSTEM" != "windows" ]]; then
  if [[ "$SYSTEM" == "macosx" ]]; then
    # ensure scotch and starpu are installed
    for dep in scotch starpu
    do
      DEP_INSTALLED=`brew ls --versions ${dep} | cut -d " " -f 2`
      if [[ -z "${DEP_INSTALLED}" ]]; then
        # dep is not installed, we have to install it
        brew install --build-from-source ./tools/homebrew/${dep}.rb
      else
        # dep is already installed, check the version with our requirement
        DEP_REQUIRED=`brew info --json ./tools/homebrew/${dep}.rb |jq -r '.[0].versions.stable'`
        if [[ "${DEP_INSTALLED}" != "${DEP_REQUIRED}" ]]; then
          # if the installed version is not the required one, re-install
          brew remove --force --ignore-dependencies ${dep}
          brew install --build-from-source ./tools/homebrew/${dep}.rb
        fi
      fi
    done
    # clang is used on macosx and it is not compatible with MORSE_ENABLE_COVERAGE=ON
    # to avoid the Accelerate framework and get Openblas we use BLA_PREFER_PKGCONFIG
    # we do not have parsec installed on the macosx machine
    cmake -B build -S . -DPASTIX_CI_VERSION=${VERSION} -DPASTIX_CI_BRANCH=${BRANCH} \
          -C .gitlab/ci-test-initial-cache.cmake \
          -DMORSE_ENABLE_COVERAGE=OFF -DBLA_PREFER_PKGCONFIG=ON -DPASTIX_WITH_PARSEC=OFF \
          || fatal
  else
    source ${CI_PROJECT_DIR}/.gitlab/env.sh ${VERSION}
    unset SPM_DIR # Make sure we don't use the installed spm
    if [ ! -z "$SPM_DIR" ]; then source $SPM_DIR/bin/spm_env; fi
    cmake -B build -S . -DPASTIX_CI_VERSION=${VERSION} -DPASTIX_CI_BRANCH=${BRANCH} \
          -C .gitlab/ci-test-initial-cache.cmake \
          || fatal
    cp compile_commands.json compile_commands-${VERSION}.json
  fi
else
  # on windows the mpi_f08 interface is missing, see https://www.scivision.dev/windows-mpi-msys2/
  # default scotch in windows msys2 is int32
  # do not use static libraries because executables are too large and the build
  # directory can reach more than 10Go
  cmake -GNinja -B build -S . -DCMAKE_INSTALL_PREFIX=$PWD/install-${VERSION} \
        -DPASTIX_WITH_MPI=OFF -DPASTIX_INT64=OFF -DBUILD_SHARED_LIBS=ON \
        || fatal
fi
cmake --build build -j 4 || fatal
cmake --install build || fatal

#
# Check link to pastix
#
cd .gitlab/check_link/

# Set the compiler
if [[ "$SYSTEM" == "macosx" ]]; then
  export CC=clang
  if brew ls --versions pastix > /dev/null; then brew remove --force --ignore-dependencies pastix; fi
  if brew ls --versions pastix64 > /dev/null; then brew remove --force --ignore-dependencies pastix64; fi
else
  export CC=gcc
fi
export FC=gfortran

# Set the path variables
if [[ "$SYSTEM" == "linux" ]]; then
  export LIBRARY_PATH=$PWD/../../install-${VERSION}/lib:$LIBRARY_PATH
  export LD_LIBRARY_PATH=$PWD/../../install-${VERSION}/lib:$LD_LIBRARY_PATH
elif [[ "$SYSTEM" == "macosx" ]]; then
  export LIBRARY_PATH=$PWD/../../install-${VERSION}/lib:$LIBRARY_PATH
  export DYLD_LIBRARY_PATH=$PWD/../../install-${VERSION}/lib:$DYLD_LIBRARY_PATH
elif [[ "$SYSTEM" == "windows" ]]; then
  export PATH="/c/Windows/WinSxS/x86_microsoft-windows-m..namespace-downlevel_31bf3856ad364e35_10.0.19041.1_none_21374cb0681a6320":$PATH
  export PATH=$PWD/../../install-${VERSION}/bin:$PATH
fi

# 1) using cmake:
./link_cmake.sh $PWD/../../install-${VERSION} || fatal
# 2) using pkg-config:
./link_pkgconfig.sh $PWD/../../install-${VERSION} || fatal

# Clean the check build
rm -r build || fatal
