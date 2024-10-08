## Using EZTrace in PaStiX

PaStiX allows you to trace your factorization to analyze your computations.

### Download and install EZTRACE

[EZTrace](https://eztrace.gitlab.io/eztrace/) is a tool that aims at
generating automatically execution trace from HPC (High Performance Computing)
programs. It generates execution trace files that can be interpreted
by visualization tools such as [ViTE](http://vite.gforge.inria.fr/).

We recommend to use a release >= 1.1-9 that can be found
[on the EZTrace website](https://eztrace.gitlab.io/eztrace/download.html).

Let's define the directory where you want to EZTrace everything as:
```sh
export EZTRACE_DIR=/your/path/to/install
```
Note that you can also keep the default install directory, and install
in the system using root privileges. We do not recommend this latest version.

Go into the downloaded directory and execute the following steps:
```sh
mkdir build
cd build
../bootstrap
../configure --prefix=${EZTRACE_DIR}
make
make install
```

If you want to use EZTrace with MPI, please change the previous configure
line with this one :
```sh
../configure --prefix=${EZTRACE_DIR} --with-mpi=path/to/your/mpi/dir
```

EZTrace is now installed on your system but may not be available in
your environment.

One way to check if it is set correctly is to verify that the following command
return the path to the freshly installed EZTrace:
```sh
which eztrace_loaded
```

If it does not find it, you can execute the following lines and try
again.

On Linux:
```sh
export PATH=$PATH:$EZTRACE_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$EZTRACE_DIR/lib/pkgconfig
export LD_RUN_PATH=$LD_RUN_PATH:$EZTRACE_DIR/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$EZTRACE_DIR/lib
export INCLUDE_PATH=$INCLUDE_PATH:$EZTRACE_DIR/include
```

On MacOS:
```sh
export PATH=$PATH:$EZTRACE_DIR/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$EZTRACE_DIR/lib/pkgconfig
export INCLUDE_PATH=$INCLUDE_PATH:$EZTRACE_DIR/include
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$EZTRACE_DIR/lib:
```

Note that these lines can (should) be added to your environment file
such as `.bashrc` to set it by default.

### Download ViTE

[ViTE](http://vite.gforge.inria.fr/) is a trace explorer. It is a tool to visualize execution traces in
Pajé or OTF format for debugging and profiling parallel or distributed applications.
It is an open source software licenced under CeCILL-A.

On Linux:
```sh
sudo apt-get install vite
```

You can also get the latest release of ViTE through the [git repository](https://gitlab.inria.fr/solverstack/vite).

### Compile PaStiX with EZTrace

Go in your PaStiX source directory and create a new build directory to
build the EZTrace version:
```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${PASTIX_DIR} \
         -DPASTIX_WITH_EZTRACE=[ON|OFF]
make
make install
```

Note that as before we defined a `PASTIX_DIR` variable to define the
install directory of PaStiX.

Once the compilation finished, you can setup your environment easily
by sourcing the provided file:
```sh
source $PASTIX_DIR/bin/pastix_env
```

### How to use PaStiX with EZTrace

During the compilation, we built an `eztrace-convert-kernels` library
to trace PaStiX kernels (POTRF, GETRF, SYTRF, GEMM, etc.). You have to include
it in your environment with :
```sh
export EZTRACE_LIBRARY_PATH=$PASTIX_BUILD_DIR/kernels
```

Or, if you have installed PaStiX :
```sh
export EZTRACE_LIBRARY_PATH=$PASTIX_DIR/lib
```

Then you have to include it in the module loaded for EZTrace. You can see
the already used modules with the `eztrace_loaded` command, and see the available ones
with `eztrace_avail`.

You can add the PaStiX kernel module with the following command :
```sh
export EZTRACE_TRACE="kernels [mpi] [omp] [others]"
```

**Warning** If you only define kernels for EZTRACE_TRACE, you will not
be able to trace other events such as MPI exchanges.

You are now ready to use PaStiX with EZTrace. For instance, if you run your favorite example:
```sh
simple --mm your_matrix.mtx
```

You will generate a trace. Each trace has a rank attached to it. It corresponds to its MPI rank.
If you want to analyse/visualize them, you have to put all of them in your command line.

You can now know the stats of your run thanks to :
```sh
eztrace_stats -o your_output_dirname your_trace_*
```
It will create a directory with a various set of datas in it.

You can now visualize the stats of your run thanks to :
```sh
eztrace_convert -o your_output_name your_trace_*
```

It will create, by default, a `your_output_name.vite` file that can be visualized thanks to :
```sh
vite your_output_name.vite
```

