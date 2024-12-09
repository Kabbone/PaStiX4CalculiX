#!/usr/bin/env bash
###
#
#  @file test.sh
#  @copyright 2023-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @version 6.4.0
#  @author Florent Pruvost
#  @author Mathieu Faverge
#  @date 2024-07-05
#
###
set -e
set -x

cd build
source ${CI_PROJECT_DIR}/.gitlab/env.sh ${VERSION}
ctest --output-on-failure --no-compress-output $TESTS_RESTRICTION --output-junit ../${LOGNAME}-junit.xml
if [[ "$SYSTEM" == "linux" ]]; then
  # clang is used on macosx and it is not compatible with MORSE_ENABLE_COVERAGE=ON
  # so that we can only make the coverage report on the linux runner with gcc
  cd ..
  lcov --directory build --capture --keep-going --output-file ${LOGNAME}.lcov
fi
