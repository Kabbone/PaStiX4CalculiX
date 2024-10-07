## Installing PaStiX with MPI support

Here you have the features available in hybrid shared/distributed
memory with MPI between processes, and POSIX threads within a process.

|                         | Seq   | Static | Dyn   | StarPU | PaRSEC |
|-------------------------|-------|--------|-------|--------|--------|
| POTRF (Cholesky)        | FR    | FR     | FR    | FR     | FR     |
| PXTRF (LL^t for complex)| FR    | FR     | FR    | FR     | FR     |
| HETRF (LDL^h)           | FR    | FR     | FR    | FR     | FR     |
| SYTRF (LDL^t)           | FR    | FR     | FR    | FR     | FR     |
| GETRF (LU)              | FR    | FR     | FR    | FR     | FR     |
| TRSM                    | FR    | FR     | FR    | FR     | FR     |
| DIAG                    | FR    | FR     | FR    | FR     | FR     |

**Disclaimer**

Even if PaStiX works in distributed, it is still under development.
You can face either performance issues (without runtime) or memory
issues (with StarPU or PaRSEC).

**Requirements**

In addition to the requirements of the sequential PaStiX, you'll need:

  * An MPI implementation (MPICH, OpenMPI...)

### Download and install OpenMPI

The Open MPI Project is an open source Message Passing Interface
implementation that is developed and maintained by a consortium
of academic, research, and industry partners.

We recommend a release >= 3.0 available on the [OpenMPI
website](https://www.open-mpi.org/).
A Ubuntu package (version 3.x) is also available on Ubuntu,
and can be download with :

```sh
sudo apt-get install openmpi-bin openmpi-common
```

To verify your downloaded version, you can launch :
```sh
mpiexec --version
```

### Download and install MPICH

MPICH is a high performance and widely portable implementation
of the Message Passing Interface (MPI) standard. MPICH and its
derivatives form the most widely used implementations of MPI in the world

We recommend a release >= 3.0 available on the [MPICH
website](https://www.mpich.org/).
An Ubuntu package (version 3.x) is also available on Ubuntu,
and can be download with :

```sh
sudo apt-get install mpich
```

To verify your downloaded version, you can launch :
```sh
mpiexec --version
```
### Download and install StarPU

Please refer to [Installing PaStiX with runtime support](https://solverstack.gitlabpages.inria.fr/pastix/md_docs_doxygen_chapters_Pastix_Runtime.html)
for StarPU installation. The only step
that will change for MPI support is that the `--enable-mpi` option
becomes mandatory when configuring StarPU:

```sh
mkdir build_MPI
cd build_MPI
../configure --prefix=${STARPU_DIR}
make
make install
```

### Download and install PaRSEC

Please refer to [Installing PaStiX with runtime support](https://solverstack.gitlabpages.inria.fr/pastix/md_docs_doxygen_chapters_Pastix_Runtime.html)
for PaRSEC installation. The only step
that will change for MPI support is that the
`-DPARSEC_DIST_WITH_MPI=ON` option becomes mandatory when configuring
PaRSEC:


```sh
mkdir build_MPI
cd build_MPI
cmake .. -DCMAKE_INSTALL_PREFIX=${PARSEC_DIR} -DPARSEC_DIST_WITH_MPI=ON
make
make install
```

### Compile PaStiX with MPI

Now that you have installed MPI, you should be able to
compile PaStiX with MPI support:

Go in your PaStiX source directory and create a new build
directory to build the GPU version:
```sh
mkdir build_MPI
cd build_MPI
cmake .. -DCMAKE_INSTALL_PREFIX=${PASTIX_DIR} -DPASTIX_WITH_MPI=ON \
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
And then, you can run your favorite example:
```sh
mpiexec -n 4 $PASTIX_DIR/examples/simple -9 10:10:10
```

### How to use PaStiX with MPI

PaStiX works with all the schedulers without runtime in distributed
memory, but if you want the best performances possible with POSIX threads,
please use the dynamic scheduler (option **-s 4**):
```sh
mpiexec -n 4  $PASTIX_DIR/examples/simple -s 4 -9 10:10:10
```

In the same way, if you intent to run multiple MPI instances on one node,
make sure that the number of MPI instances per node times the number of
threads per MPI instance is less or equals the number of cores of the node.

If hwloc gives you trouble in the recognition of the topology, you can
specify the amount of threads per node with the option **-t**. It this
still does not work, you can contact us to describe your problem.
