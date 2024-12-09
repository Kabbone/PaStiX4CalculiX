#!/usr/bin/env bash
###
#
#  @file analysis.sh
#  @copyright 2013-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @version 6.4.0
#  @author Mathieu Faverge
#  @author Florent Pruvost
#  @date 2024-07-05
#
###
set -e
set -x

PROJECT=$CI_PROJECT_NAME

# Performs an analysis of the project source code:
# - we consider to be in the project's source code root
# - we consider having build log files $PROJECT-build*.log in the root directory
# - we consider having junit files $PROJECT-test*junit.xml in the root directory
# - we consider having coverage files $PROJECT-test*.lcov in the root directory
# - we consider having cppcheck, sonar-scanner programs available in the environment

if [ $# -gt 0 ]
then
    BUILDDIR=$1
fi
BUILDDIR=${BUILDDIR:-build}

TOOLSDIR=$(dirname $0)
$TOOLSDIR/find_sources.sh $BUILDDIR

# Undefine this because not relevant in our configuration
export UNDEFINITIONS="-UWIN32 -UWIN64 -U_MSC_EXTENSIONS -U_MSC_VER -U__SUNPRO_C -U__SUNPRO_CC -U__sun -Usun -U__cplusplus"
export UNDEFINITIONS="$UNDEFINITIONS -UPARSEC_PROF_DRY_BODY -UPARSEC_PROF_DRY_RUN -UPARSEC_PROF_TRACE -UPARSEC_PROF_GRAPHER -UPARSEC_SIM -DPINS_ENABLE -UBUILD_PARSEC"
export UNDEFINITIONS="$UNDEFINITIONS -UPARSEC_DEBUG_NOISIER -UPARSEC_DEBUG_PARANOID -UPARSEC_DEBUG_HISTORY -UPARSEC_C_PARSEC_HAVE_VISIBILITY"
export UNDEFINITIONS="$UNDEFINITIONS -UBUILDING_STARPU -UPASTIX_WITH_STARPU_DIST"
export UNDEFINITIONS="$UNDEFINITIONS -UNAPA_SOPALIN"

# run cppcheck analysis
CPPCHECK_OPT=" -v -f --language=c --platform=unix64 --enable=all --xml --xml-version=2 --suppress=missingIncludeSystem ${UNDEFINITIONS}"
cppcheck $CPPCHECK_OPT --file-list=./filelist_none.txt 2> ${PROJECT}-cppcheck.xml
cppcheck $CPPCHECK_OPT -DPRECISION_s -UPRECISION_d -UPRECISION_c -UPRECISION_z -UPRECISION_z --file-list=./filelist_s.txt 2>> ${PROJECT}-cppcheck.xml
cppcheck $CPPCHECK_OPT -UPRECISION_s -DPRECISION_d -UPRECISION_c -UPRECISION_z -UPRECISION_z --file-list=./filelist_d.txt 2>> ${PROJECT}-cppcheck.xml
cppcheck $CPPCHECK_OPT -UPRECISION_s -UPRECISION_d -DPRECISION_c -UPRECISION_z -UPRECISION_z --file-list=./filelist_c.txt 2>> ${PROJECT}-cppcheck.xml
cppcheck $CPPCHECK_OPT -UPRECISION_s -UPRECISION_d -UPRECISION_c -DPRECISION_z -UPRECISION_z --file-list=./filelist_z.txt 2>> ${PROJECT}-cppcheck.xml
cppcheck $CPPCHECK_OPT -UPRECISION_s -UPRECISION_d -UPRECISION_c -UPRECISION_z -DPRECISION_p --file-list=./filelist_p.txt 2>> ${PROJECT}-cppcheck.xml

# merge all different compile_commands.json files into one single file for sonarqube
ls $BUILDDIR/compile_commands-*.json
jq -s 'map(.[])' $BUILDDIR/compile_commands-*.json > ${PROJECT}-compile_commands.json
ls ${PROJECT}-compile_commands.json

# create the sonarqube config file
cat > sonar-project.properties << EOF
sonar.host.url=https://sonarqube.inria.fr/sonarqube
sonar.qualitygate.wait=true

sonar.links.homepage=$CI_PROJECT_URL
sonar.links.scm=$CI_REPOSITORY_URL
sonar.links.ci=$CI_PROJECT_URL/pipelines
sonar.links.issue=$CI_PROJECT_URL/issues

sonar.projectKey=pastix_pastix_AZJlwL_NsbMNg1jXgz1q
sonar.projectDescription=Parallel Sparse direct Solver
sonar.projectVersion=6.4.1

sonar.scm.disabled=false
sonar.scm.provider=git
sonar.scm.exclusions.disabled=true

sonar.sources=$BUILDDIR, bcsc, blend, common, example, graph, include, kernels, order, refinement, sopalin, symbol, test
sonar.inclusions=`cat filelist.txt | grep -v spm | xargs echo | sed 's/ /, /g'`
sonar.sourceEncoding=UTF-8
sonar.cxx.file.suffixes=.h,.c
sonar.cxx.errorRecoveryEnabled=true
sonar.cxx.gcc.encoding=UTF-8
sonar.cxx.gcc.regex=(?<file>.*):(?<line>[0-9]+):[0-9]+:\\\x20warning:\\\x20(?<message>.*)\\\x20\\\[(?<id>.*)\\\]
sonar.cxx.gcc.reportPaths=${PROJECT}-build*.log
sonar.cxx.xunit.reportPaths=${PROJECT}-test*junit.xml
sonar.cxx.cobertura.reportPaths=${PROJECT}-coverage.xml
sonar.cxx.cppcheck.reportPaths=${PROJECT}-cppcheck.xml
sonar.cxx.jsonCompilationDatabase=${PROJECT}-compile_commands.json
EOF
echo "====== sonar-project.properties ============"
cat sonar-project.properties
echo "============================================"

# run sonar analysis + publish on sonarqube
sonar-scanner -X > sonar.log
