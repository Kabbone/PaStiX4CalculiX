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
# system and compiler
export UNDEFINITIONS="-UWIN32 -U_WIN32 -UWIN64 -U_WIN64 -UMAC_OS_X -U__FUJITSU -U_MSC_EXTENSIONS -U_MSC_VER -U__SUNPRO_C -U__SUNPRO_CC -U__sun -Usun -Uc_plusplus -U__cplusplus -U__X86 -U__IA64 -U__aarch64__ -U __arm64__ -U__IBMC__ -U__INTEL_COMPILER -U__bgp__"
# have compiler features
export UNDEFINITIONS="$UNDEFINITIONS -UHAVE_ATOMIC_XLC_32_BUILTINS -UHAVE_ATOMIC_XLC_64_BUILTINS -UHAVE_ATOMIC_MIPOSPRO_32_BUILTINS -UHAVE_ATOMIC_MIPOSPRO_64_BUILTINS -UHAVE_ATOMIC_SUN_32 -UHAVE_ATOMIC_SUN_64 -UPASTIX_ATOMIC_HAS_ATOMIC_ADD_32B -UPASTIX_ATOMIC_HAS_ATOMIC_SUB_32B -UHAVE_UNDERSCORE_VA_COPY -U_POSIX_BARRIERS"
# blas
export UNDEFINITIONS="$UNDEFINITIONS -UHAVE_BLAS_SET_NUM_THREADS -UHAVE_BLI_THREAD_SET_NUM_THREADS -UHAVE_OPENBLAS_SET_NUM_THREADS -UHAVE_MKL_SET_NUM_THREADS -UWeirdNEC -UHAVE_LAPACK_CONFIG_H"
# parsec specific
export UNDEFINITIONS="$UNDEFINITIONS -UPARSEC_PROF_DRY_BODY -UPARSEC_PROF_DRY_RUN -UPARSEC_PROF_TRACE -UPARSEC_PROF_GRAPHER -UPARSEC_SIM -UBUILD_PARSEC"
export UNDEFINITIONS="$UNDEFINITIONS -UPARSEC_DEBUG_NOISIER -UPARSEC_DEBUG_PARANOID -UPARSEC_DEBUG_HISTORY -UPARSEC_C_PARSEC_HAVE_VISIBILITY"
# pastix/spm specific
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_ARCH_X86 -UPASTIX_ARCH_PPC -UPASTIX_OS_AIX -UPASTIX_OS_FREEBSD -UPASTIX_OS_MACOS -UPASTIX_OS_WINDOWS -UCOMMON_RANDOM_RAND"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_DISTRIBUTED -UPASTIX_WITH_CUDA -UPASTIX_WITH_EZTRACE -UPASTIX_WITH_PAPI -UPASTIX_AFFINITY_DISABLE -UPASTIX_HAVE_SCHED_SETAFFINITY -UPASTIX_HAVE_OLD_SCHED_SETAFFINITY"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_SYMBOL_DUMP_SYMBMTX -UPASTIX_ORDER_DRAW_LASTSEP"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_BLEND_GENTRACE -UPASTIX_BLEND_PROPMAP_2STEPS -UPASTIX_BLEND_FANIN_FR -UPASTIX_BLEND_COSTLEVEL"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_NUMFACT_DUMP_SOLVER -UPASTIX_SUPERNODE_STATS"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_STARPU_PROFILING -UPASTIX_STARPU_PROFILING_LOG -UPASTIX_STARPU_COST_PER_ARCH -UPASTIX_STARPU_SYNC -UPASTIX_STARPU_STATS -UPASTIX_STARPU_HETEROPRIO -UPASTIX_STARPU_SIMULATION -UPASTIX_STARPU_INTERFACE_DEBUG -UPASTIX_DYNSCHED -UPASTIX_WITH_CUBLAS_V2 -USTARPU_CUDA_ASYNC"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_GENERATE_MODEL -UPASTIX_CUDA_FERMI"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_DEBUG_VALGRIND -UPASTIX_DEBUG_GRAPH -UPASTIX_DEBUG_ORDERING -UPASTIX_DEBUG_SYMBOL -UPASTIX_DEBUG_BLEND -UPASTIX_DEBUG_DUMP_COEFTAB -UPASTIX_DEBUG_FACTO -UPASTIX_DEBUG_SOLVE -UPASTIX_DEBUG_PARSEC -UPASTIX_DEBUG_STARPU -UPASTIX_DEBUG_LR -UPASTIX_DEBUG_LR_NANCHECK -UPASTIX_DEBUG_GMRES -UPASTIX_DEBUG_MPI -UPASTIX_DEBUG_COMMON -UNDEBUG -UFAX_DEBUG"
export UNDEFINITIONS="$UNDEFINITIONS -UPASTIX_INT64 -UPASTIX_LONG -USPM_INT64 -USPM_LONG"
export UNDEFINITIONS="$UNDEFINITIONS -UFORGET_PARTITION -UCOMPACT_SMX -UWITH_SEM_BARRIER"
export UNDEFINITIONS="$UNDEFINITIONS -UNAPA_SOPALIN -UHAVE_MM_SET_CSR -UHAVE_MM_SETCSR"
# pastix define (see config.h)
export DEFINITIONS="-DPASTIX_WITH_FORTRAN -DPASTIX_WITH_MPI -DPASTIX_COMMUNICATION_MATRIX -DHAVE_SCHED_SETAFFINITY -DHAVE_CLOCK_GETTIME -DHAVE_ATOMIC_GCC_32_BUILTINS -DHAVE_ATOMIC_GCC_64_BUILTINS -DHAVE_ATOMIC_GCC_128_BUILTINS -DHAVE_STDARG_H -DHAVE_UNISTD_H -DHAVE_VA_COPY -DHAVE_GETOPT_LONG -DHAVE_GETRUSAGE -DHAVE_GETOPT_H -DHAVE_ERRNO_H -DHAVE_STDDEF_H -DHAVE_LIMITS_H -DHAVE_STRING_H -DHAVE_COMPLEX_H -DHAVE_FALLTHROUGH -DHAVE_BUILTIN_EXPECT -DHAVE_GETLINE -DHAVE_MKDTEMP -DHAVE_FTZ_MACROS -DHAVE_DAZ_MACROS -DPASTIX_ARCH_X86_64 -DHAVE_HWLOC -DPASTIX_ORDERING_SCOTCH -DPASTIX_ORDERING_METIS -DPASTIX_ORDERING_PTSCOTCH -DPASTIX_ORDERING_FIX_SEED -DPASTIX_ORDERING_SCOTCH_MT -DPASTIX_BLEND_DEEPEST_DISTRIB -DPASTIX_WITH_PARSEC -DPASTIX_WITH_STARPU -DHAVE_STARPU_SCHED_POLICY_CALLBACK -DPASTIX_INT32 -DPASTIX_OS_LINUX -DISCHED_IMPLEMENT_BARRIERS -DSPM_INT32 -DSPM_WITH_MPI -DPINS_ENABLE -D__GNUC__ -D__x86_64 -DSTARPU_USE_FXT"
# pastix path to headers
export INCLUDES="-I. -Iblend -Icommon -Iinclude -Ikernels -Itest -Ispm/include -Ispm/src -Ibuild -Ibuild/include -Ibuild/kernels/ -Ibuild/sopalin -Ibuild/spm/include -Ibuild/test/"

# run cppcheck analysis
CPPCHECK_OPT=" -v -f --language=c --platform=unix64 --enable=all --xml --xml-version=2 --suppress=missingIncludeSystem ${INCLUDES} ${DEFINITIONS} ${UNDEFINITIONS}"
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
