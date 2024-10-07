## Build and Install PaStiX with CMake

** Requirements **

PaStiX needs some libraries to be built on your system :

  * A **sequential** BLAS library (MKL, OpenBlas, ...), as well as the C interface CBLAS that comes with it.
  * A **sequential** LAPACK library (MKL, OpenBLAS, ...) as well as the C interface LAPACKE that comes with it, and only for testing purpose the TMG library to generate matrices.
  * The [hwloc](https://www.open-mpi.org/projects/hwloc/) library is not mandatory but **highly recommended** to enable better thread mapping.
  * An optional ordering library among:
    * The [Scotch](https://gitlab.inria.fr/scotch/scotch) library
    * The [METIS](http://glaros.dtc.umn.edu/gkhome/metis/metis/overview) library
    * Note that the user can provide its own ordering if he does not want to use any of these libraries.

### Install BLAS and LAPACK

**Install BLAS and CBLAS**

BLAS (Basic Linear Algebra Subprograms) is a specification that
prescribes a set of low-level routines for performing common
linear algebra operations. These routines were originally written in
Fortran, so we need the CBLAS interface to compile it in C.

On Linux :
```sh
sudo apt-get install libopenblas-dev
```

On Mac:
```sh
brew install openblas
```

**Install LAPACK and LAPACKE**

LAPACK and LAPACKE are libraries that provide routines for
solving systems of linear equations. They depend on the BLAS library.

On Linux :
```sh
sudo apt-get install liblapacke-dev
```

On Mac:
```sh
brew install lapack
```

Warning : you may need to add the following lines to help cmake with LAPACKE
on MacOS for the configuration of PaStiX during the build step:
```sh
-DBLAS_DIR=${OPENBLAS_INSTALL_DIR} -DBLA_VENDOR=Open
```

### Install hwloc

The [Portable Hardware Locality (hwloc)](https://www.open-mpi.org/projects/hwloc/)
software package provides a portable abstraction of the hierarchical topology
of modern architectures, including NUMA memory nodes, sockets, shared
caches, cores and simultaneous multi-threading. It also gathers various
system attributes such as cache and memory information as well as the
locality of I/O devices such as network interfaces, Infiniband, HCAs or GPUs.

On Linux :
```sh
sudo apt-get install libhwloc-dev
```

On Mac:
```sh
brew install hwloc
```

### Install an ordering library

At last, we need to install a library for the ordering part of the factorization.
PaStiX supports both [Scotch](https://www.labri.fr/perso/pelegrin/scotch/)
and [Metis](http://glaros.dtc.umn.edu/gkhome/metis/metis/overview).

#### Install Metis

You can install it simply with the following command :

On Linux :
```sh
sudo apt-get install libsmetis-dev
```

On Mac:
```sh
brew install metis
```

#### Install Scotch

You can install it simply with the following command on Linux:

```sh
sudo apt-get install libscotch-dev
```

on Mac : (use the homebrew formula available in the PaStiX repository)

```sh
brew install $PASTIX_DIR/tools/homebrew/scotch5.rb
```

Or, for the 6.0 version :

```sh
sudo apt-get install libscotch-dev libscotch-6.0
```

However, you may want compile your own Scotch library. For doing so,
download the version that you want to use and go in your Scotch directory.

Let's define the directory where you want to install Scotch as:
```sh
export SCOTCH_DIR=/your/path/to/install
```

Note that you can also keep the default install directory, and install
in the system using root privileges. We do not recommend this latest version.

To build scotch, you have to copy the Make.inc corresponding to your architecture :
```sh
cd scotch_x.x.x/src
cp Make.inc/Makefile.inc.xxxx_xxx_xxx Makefile.in
make
make prefix=$SCOTCH_DIR install
```
Note that either you choose INTSIZE32 or INTSIZE64 in the Makefile.in file, it will
define the value of -DPASTIX_INT64=[ON|OFF] for PaStiX.

Scotch is now installed on your system but may not be available in
your environment.

One way to check if it is set correctly is to verify that the following command
return the path to the freshly installed Scotch:
```sh
which gmap
```

If it does not find it, you can execute the following lines and try
again.

On Linux:
```sh
export PATH=$PATH:$SCOTCH_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$SCOTCH_DIR/lib/pkgconfig
export LD_RUN_PATH=$LD_RUN_PATH:$SCOTCH_DIR/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCOTCH_DIR/lib
export INCLUDE_PATH=$INCLUDE_PATH:$SCOTCH_DIR/include
```

On MacOS:
```sh
export PATH=$PATH:$SCOTCH_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$SCOTCH_DIR/lib/pkgconfig
export INCLUDE_PATH=$INCLUDE_PATH:$SCOTCH_DIR/include
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$SCOTCH_DIR/lib:
```

Note that these lines can (should) be added to your environment file
such as `.bashrc` to set it by default.

### Get PaStiX

To use the latest stable development branch of PaStiX, please clone the master branch.
Note that PaStiX contains two git submodules for spm and morse_cmake.
To get source codes please use these commands:
```sh
# if git version >= 1.9
git clone --recursive git@gitlab.inria.fr:solverstack/pastix.git
cd pastix
# else
git clone https://gitlab.inria.fr/solverstack/pastix.git
cd pastix
git submodule init
git submodule update
```

### Build and Install PaStiX

Go in your PaStiX source directory and create a new build directory to
build the default shared memory (without MPI) version:
```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${PASTIX_DIR} -DPASTIX_INT64=[ON|OFF] \
         -DPASTIX_ORDERING_SCOTCH=[ON|OFF] \
         -DPASTIX_ORDERING_METIS=[ON|OFF]
make
```

Then you can install it simply with the command :
```sh
make install
```

Note that as before we defined a `PASTIX_DIR` variable to define the
install directory of PaStiX.

On Mac, if you decide to not build PaStiX and only install it with all its
dependencies, you can simply run :
```sh
brew install ${PASTIX_SRC_DIR}/tools/homebrew/pastix6.rb
```

Once the compilation finished, you can setup your environment easily
by sourcing the provided file:
```sh
source ${PASTIX_DIR}/bin/pastix_env
```

And then, you can run your favorite example:
```sh
 ${PASTIX_DIR}/examples/simple -9 10:10:10
```

Your setup is ready to play with PaStiX. Please refer to section [How to
to use PaStiX](https://solverstack.gitlabpages.inria.fr/pastix/md_docs_doxygen_chapters_Pastix_Python.html) to get as best results as possible.

### How to link PaStiX within your code

As said previously, you can setup your environment easily by sourcing the provided file:
```sh
source ${PASTIX_DIR}/bin/pastix_env
```

But, you can also have a look on examples installed to see how to link PaStiX within your code :
```sh
cd ${PASTIX_DIR}/examples
make clean
VERBOSE=1 make
```

You will get the following output with a `C` driver using `OpenBLAS` :
```sh
cc -o simple simple.c -I$PASTIX_DIR/include -Wall -O2 -I$PASTIX_DIR/include/pastix \
-L$PASTIX_DIR/lib -lpastix -lpastix_kernels -lspm -lhwloc -L/usr/lib/x86_64-linux-gnu \
-llapacke -L/usr/lib/x86_64-linux-gnu/openblas-pthread -lopenblas -L/usr/lib/x86_64-linux-gnu \
-lscotch -L/usr/lib/x86_64-linux-gnu -lscotcherrexit -L/usr/lib/x86_64-linux-gnu \
-lpthread -L/usr/lib/x86_64-linux-gnu -lz -L/usr/lib/x86_64-linux-gnu -lm \
-L/usr/lib/x86_64-linux-gnu -lrt
```

You will get the following output with a `Fortran` driver using `openblas` :
```sh
f77 -o fsimple fsimple.f90 -I$PASTIX_DIR/include -I$PASTIX_DIR/include/pastix -Wall -O2 \
-L$PASTIX_DIR/lib -lpastixf -lpastix -lpastix_kernels -lpastix -lpastix_kernels -lspmf \
-lspm -lhwloc -L/usr/lib/x86_64-linux-gnu -llapacke -L/usr/lib/x86_64-linux-gnu/openblas-pthread \
-lopenblas -L/usr/lib/x86_64-linux-gnu -lscotch -L/usr/lib/x86_64-linux-gnu -lscotcherrexit \
-L/usr/lib/x86_64-linux-gnu -lpthread -L/usr/lib/x86_64-linux-gnu -lz -L/usr/lib/x86_64-linux-gnu \
-lm -L/usr/lib/x86_64-linux-gnu -lrt
```

It will work if you change the examples Makefile accordingly :
```sh
PASTIXINCS=$(shell echo `PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --cflags pastix`)
PASTIXLIBS=$(shell echo `PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --libs pastix`)

CFLAGS= ${PASTIXINCS} -Wall -O2 -I${PASTIX_DIR}/include/pastix
LDFLAGS= ${PASTIXLIBS} ${EXTRALIBS}

PASTIXFINCS=$(shell echo `PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --cflags pastixf`)
PASTIXFLIBS=$(shell echo `PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --libs pastixf`)

FFLAGS=${PASTIXFINCS} -Wall -O2
LDFFLAGS=${PASTIXFLIBS} ${EXTRALIBS}
```
