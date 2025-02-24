#
#  @file update_release.sh
#
#  @copyright 2016-2025 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @version 6.4.0
#  @author Mathieu Faverge
#  @date 2024-07-05
#
#!/usr/bin/env sh

# subset of commit to use to select the files tp work with
subset="diff --name-only HEAD~1"

release="no"

majorversion=6
minorversion=4
microversion=0

remotelogin=faverge
locallogin=mathieu

while [ $# -gt 0 ]
do
    case $1 in
        --major )
            shift
            majorversion="$1"
            ;;
        --minor )
            shift
            minorversion="$1"
            ;;
        --micro )
            shift
            microversion="$1"
            ;;
        --all )
            subset="ls-files"
            ;;
        --release )
            subset="ls-files"
            release="yes"
            ;;
        --rlogin )
            shift
            remotelogin="$1"
            ;;
        --llogin )
            shift
            locallogin="$1"
            ;;
        *)
            # Let's consider that anything else is a tag or a list of commit to study
            subset="diff --name-only $1"
            ;;
    esac
    shift
done
version="$majorversion.$minorversion.$microversion"

# Let's get the list of files to update
fileslist=$( git $subset )

#
# Steps to update header information before doing the release
#

#
# 1) Check header files with check_headers.sh
#
#./tools/check_headers.sh

#
# 2) Check that the fortran/python wrappers have been updated (see gen_wrappers.py)
#
#./tools/gen_wrappers.py

#
# 3) First update the date of the files with the following lines
#
for f in $fileslist
do
    if [ ! -f $f ]
    then
        continue;
    fi

    date=$( git log -1 --format=%cd --date=short $f )
    echo $date $f
    sed -i "s/date [-0-9]*\$/date $date/" $f
done

#
# 3b) Update the author list
#
if [ "$release" = "no" ]
then
    ./tools/check_authors.sh $fileslist
fi

#
# 4) Update the release number
#
for f in $fileslist
do
    if [ ! -f $f ]
    then
        continue;
    fi

    sed -i "s/@version [0-9]\.[0-9]\.[0-9][0-9]*/@version $version/" $f
done

#
# 5a) Update manually the version number in CMakeLists.txt
#
if [ $release = "yes" ]
then
    sed -i "s/set( PASTIX_VERSION_MAJOR [0-9] )/set( PASTIX_VERSION_MAJOR $majorversion )/" CMakeLists.txt
    sed -i "s/set( PASTIX_VERSION_MINOR [0-9] )/set( PASTIX_VERSION_MINOR $minorversion )/" CMakeLists.txt
    sed -i "s/set( PASTIX_VERSION_MICRO [0-9][0-9]* )/set( PASTIX_VERSION_MICRO $microversion )/" CMakeLists.txt

    #
    # 5b) Update manually the analysis.sh script
    #
    analysismicro=$(( microversion + 1 ))
    analysisversion="$majorversion.$minorversion.$analysismicro"
    sed -i "s/sonar.projectVersion=.*$/sonar.projectVersion=$analysisversion/" tools/analysis.sh
fi

#
# 6) If necessary, update the copyright information
#
for f in $fileslist
do
    if [ ! -f $f ]
    then
        continue;
    fi

    year=$( git log -1 --format=%cd --date=format:%Y $f )
    year=$( date +%Y )
    toto=$( grep -E " @copyright [0-9]{4}-$year Bordeaux INP" $f )

    if [ $? -ne 0 ]
    then
        sed -i "s/copyright \([0-9]*\)-[0-9]* Bordeaux/copyright \1-$year Bordeaux/" $f
    fi
done

#
# 7) Add the release to files.inria.fr/pastix/releases
#

if [ $release = "yes" ]
then
    # Cf https://doc-si.inria.fr/display/SU/Espace+web# to mount the remote filesystem
    # Here are the linux commands (Using VPN)
    #
    # sudo apt-get install davfs2
    mkdir -p /tmp/webdav-pastix
    sudo mount.davfs -o username=$remotelogin,user,noauto,uid=$locallogin https://files.inria.fr:8443/pastix /tmp/webdav-pastix
    cd /tmp/webdav-pastix/releases
    tree -rv -H https://files.inria.fr/pastix/releases >! index.html
fi

#
# 8) Update homebrew formula (only after release)
#
