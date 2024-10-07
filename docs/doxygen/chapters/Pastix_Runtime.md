## Installing PaStiX with runtime support

** Requirements **

In addition to the requirements of the sequential or MPI version, you'll need:

  * A runtime system among:
    * StarPU (https://starpu.gitlabpages.inria.fr)
    * PaRSEC (http://icl.utk.edu/parsec/)

### Download and install StarPU

[StarPU](https://starpu.gitlabpages.inria.fr) is a runtime system
developed by the research team STORM at Inria Bordeaux -
Sud-Ouest. It provides a sequential task flow programming model similar
to OpenMP tasks with an additional support of the GPUs and distributed
environment.

The latest release is available
[here](https://files.inria.fr/starpu/), and we recommend a release >=
1.3.7.
A detailed explanation on how to install StarPU is directly available
on [their
website](https://files.inria.fr/starpu/testing/master/doc/html/BuildingAndInstallingStarPU.html),
however we give here a basic set of commands to install the release
from the latest release.

Go into your own working directory for StarPU.
Download the latest version and extract the archive.

```sh
wget https://files.inria.fr/starpu/starpu-1.3.7/starpu-1.3.7.tar.gz
tar xvzf starpu-1.3.7.tar.gz
```

Let's define the directory where you want to StarPU everything as:
```sh
export STARPU_DIR=/your/path/to/install
```
Note that you can also keep the default install directory, and install
in the system using root privileges. We do not recommend this latest version.

Let's create a build directory outside the source directory to
configure and build StarPU with minimal flags for PaStiX:
```sh
mkdir build
cd build
../configure --prefix=${STARPU_DIR}
make
make install
```

StarPU is now installed on your system but may not be available in
your environment.

One way to check if it is set correctly is to verify that the following command
return the path to the freshly installed StarPU:
```sh
which starpu_calibrate_bus
```

If it does not find it, you can execute the following lines and try
again.

On Linux:
```sh
export PATH=$PATH:$STARPU_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$STARPU_DIR/lib/pkgconfig
export LD_RUN_PATH=$LD_RUN_PATH:$STARPU_DIR/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$STARPU_DIR/lib
export INCLUDE_PATH=$INCLUDE_PATH:$STARPU_DIR/include
```

On MacOS:
```sh
export PATH=$PATH:$STARPU_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$STARPU_DIR/lib/pkgconfig
export INCLUDE_PATH=$INCLUDE_PATH:$STARPU_DIR/include
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$STARPU_DIR/lib:
```

Note that these lines can (should) be added to your environment file
such as `.bashrc` to set it by default.

### Download and install PaRSEC

[PaRSEC](http://icl.utk.edu/parsec/) is a runtime system developed by the Innovative Computing
Laboratory at the University of Tennessee. It provides multiple
programming models to interact with such as a the _Dynamic Task
Discovery (DTD)_ model (identical to the _Sequential Task Flow (STF)_
from StarPU), as well as a _Parameterized Task Graph (PTG)_ model
which is here exploited by PaStiX to implement an
heterogeneous implementation (Distributed implementation of PaStiX on
top of PaRSEC is not yet tested and validated).

The latest release available is the one from the [BitBucket
repository](https://bitbucket.org/icldistcomp/parsec). However, due to specific requirements in the context of
PaStiX, we **SOLELY** rely on [this dedicated
fork](https://bitbucket.org/mfaverge/parsec) with the specific
`pastix-6.0.2` tag. Be careful to compile PaStiX **only** with the
specific revision to enable PaRSEC runtime.

Go into your own working directory for PaRSEC.
Clone the repository, and get the specific tag.

```sh
git clone --recursive https://bitbucket.org/mfaverge/parsec
cd parsec
git checkout pastix-6.0.2
```

Let's define the directory where you want to install PaRSEC as:
```sh
export PARSEC_DIR=/your/path/to/install
```

Note that you can also keep the default install directory, and install
in the system using root privileges. We do not recommend this latest version.

Let's create a build directory outside the source directory to
configure and build PaRSEC with minimal flags for PaStiX:
```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${PARSEC_DIR}
make
make install
```

PaRSEC is now installed on your system but may not be available in
your environment.

One way to check if it is set correctly is to verify that the following command
return the path to the freshly installed PaRSEC compiler:
```sh
which parsec_ptgpp
```

If it does not find it, you can execute the following lines and try
again.

On Linux:
```sh
export PATH=$PATH:$PARSEC_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PARSEC_DIR/lib/pkgconfig
export LD_RUN_PATH=$LD_RUN_PATH:$PARSEC_DIR/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PARSEC_DIR/lib
export INCLUDE_PATH=$INCLUDE_PATH:$PARSEC_DIR/include
```

On MacOS:
```sh
export PATH=$PATH:$PARSEC_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PARSEC_DIR/lib/pkgconfig
export INCLUDE_PATH=$INCLUDE_PATH:$PARSEC_DIR/include
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$PARSEC_DIR/lib:
```

Note that these lines can (should) be added to your environment file
such as `.bashrc` to set it by default.

### Compile PaStiX with runtime support

Now that you have at least one of the two runtime systems as
described above, you should be able to compile PaStiX with
runtime support:

Go in your PaStiX source directory and create a new build directory to
build the runtime version:
```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${PASTIX_DIR} \
    -DPASTIX_WITH_STARPU=[ON|OFF] -DPASTIX_WITH_PARSEC=[ON|OFF]
make
make install
```

Note that as before we defined a `PASTIX_DIR` variable to define the
install directory of PaStiX, and you just need to turn ON/OFF the
runtime systems based on the available one(s).

Once the compilation finished, you can setup your environment easily
by sourcing the provided file:
```sh
source $PASTIX_DIR/bin/pastix_env
```

### How to use PaStiX with runtime systems

The options **-s** defines the scheduler:
* **-s 2** uses the PaRSEC scheduler.
* **-s 3** uses the StarPU scheduler.

```sh
simple -s [2|3] --mm your_matrix.mtx
```
