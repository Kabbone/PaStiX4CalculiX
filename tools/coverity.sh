#!/usr/bin/env bash
#
# @file coverity.sh
#
# @copyright 2024-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                      Univ. Bordeaux. All rights reserved.
#
# @version 6.4.1
# @author Mathieu Faverge
# @author Florent Pruvost
# @date 2024-12-10
#
set -e
set -x
PASTIX_SRC_DIR=${CI_PROJECT_DIR:-$PWD}

source ${PASTIX_SRC_DIR}/.gitlab/env.sh ${VERSION}

cmake -B build-${VERSION} \
      -D PASTIX_CI_VERSION=${VERSION} \
      -C ${PASTIX_SRC_DIR}/.gitlab/ci-test-initial-cache.cmake \
      -S ${PASTIX_SRC_DIR}

cov-build --dir cov-int/ cmake --build build-${VERSION} -j4

tar czvf ${CI_PROJECT_NAME}.tgz cov-int/

curl --form token=$COVERITY_TOKEN \
     --form email=mathieu.faverge@inria.fr \
     --form file=@${CI_PROJECT_NAME}.tgz \
     --form version="`git rev-parse --short HEAD`" \
     --form description="" \
     https://scan.coverity.com/builds?project=PaStiX
