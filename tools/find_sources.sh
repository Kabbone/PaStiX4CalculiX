#!/usr/bin/env sh
###
#
#  @file find_sources.sh
#  @copyright 2013-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @brief Generate the filelist for the static analysis
#
# filter sources:
# - consider generated files in ${BUILDDIR}
# - exclude base *z* files to avoid duplication
# - exclude cblas.h and lapacke-.h because not really part of pastix and make cppcheck analysis too long
#
#  @version 6.4.0
#  @author Mathieu Faverge
#  @date 2024-07-05
#
###
if [ $# -gt 0 ]
then
    BUILDDIR=$1
fi
BUILDDIR=${BUILDDIR:-build}

echo $PWD
rm -f filelist.txt

git ls-files | grep "\.[ch]"   >  filelist.txt
find $BUILDDIR -name '*\.[ch]' >> filelist.txt
echo "${BUILDDIR}/include/pastix/config.h" >> filelist.txt

git ls-files | grep "\.py"     > filelist_py.txt
echo "${BUILDDIR}/wrappers/python/examples/pypastix/enum.py" >> filelist_py.txt

# Remove files in kernel/gpus that are C++ and not our own files.
sed -i "/kernels\/gpus\/.*/d" filelist.txt

# Remove all CMakeFiles generated files
sed -i '/CMakeFiles/d' filelist.txt

# Remove all .cmake files
sed -i '/.cmake/d' filelist.txt

# Remove all .in files
sed -i '/.in$/d' filelist.txt

# Remove all clang files
sed -i '/^\.clang/d' filelist.txt

# Remove files compiled from jdf files
for jdf in `find -name "*\.jdf"`
do
    name=`basename $jdf .jdf`
    echo Remove $name
    sed -i "/$name\.[ch]/d" filelist.txt
done

# Remove Julia wrapper files
sed -i '/build\/wrappers\/julia\/depot\/.*/d' filelist.txt

# Remove installed files
sed -i '/^install.*/d' filelist.txt

# Remove original files used for precision generation
for file in `git grep "@precisions" | awk -F ":" '{ print $1 }'`
do
    sed -i "\:^$file.*:d" filelist.txt
done

# Remove external header files
for file in include/cblas.h include/lapacke.h
do
    sed -i "\:^$file.*:d" filelist.txt
done

# Remove submodules
for file in spm cmake_modules/morse_cmake
do
    sed -i "\:^$file:d" filelist.txt
done

# Remove specific systems header files (our config: linux, x86_64, gcc)
sed -i '/^common\/sys\/atomic-macosx.h/d' filelist.txt
sed -i '/^common\/sys\/atomic-ppc-bgp.h/d' filelist.txt
sed -i '/^common\/sys\/atomic-ppc.h/d' filelist.txt
sed -i '/^common\/sys\/atomic-x86_32.h/d' filelist.txt
sed -i '/^common\/sys\/atomic-xlc.h/d' filelist.txt

# Generate separated list of files for generated files and non generated files
rm -f filelist_[sdcz].txt
for name in $(cat filelist.txt)
do
    test=$(grep "@generated" $name | wc -l)
    if [ $test -gt 0 ]
    then
        prec=$(grep "@generated" $name | sed 's/^.*[scdzp] -> \([sdczp]\).*$/\1/')
        echo $name >> filelist_${prec}.txt
    else
        echo $name >> filelist_none.txt
    fi
done
