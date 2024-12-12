#!/usr/bin/env bash
#
# @file pages.sh
#
# @copyright 2024-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                      Univ. Bordeaux. All rights reserved.
#
# @version 6.4.1
# @author Mathieu Faverge
# @author Florent Pruvost
# @date 2024-12-12
#
set -e
set -x
PASTIX_SRC_DIR=${CI_PROJECT_DIR:-$PWD}

source ${PASTIX_SRC_DIR}/.gitlab/env.sh ${VERSION}

cmake -B build-${VERSION} \
      -D PASTIX_CI_VERSION=doc \
      -C ${PASTIX_SRC_DIR}/.gitlab/ci-test-initial-cache.cmake \
      -S ${PASTIX_SRC_DIR}
cmake --build build-${VERSION} -j5 > /dev/null
cmake --build build-${VERSION} --target docs
mv build-${VERSION}/docs/out/html public/
