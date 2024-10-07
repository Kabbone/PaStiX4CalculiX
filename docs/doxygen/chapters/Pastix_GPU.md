## Installing PaStiX with GPU support

Note that GPU support is only available through runtime system
environment. StarPU and/or PaRSEC is thus a mandatory dependency to
enable GPUs support.

** Requirements **

In addition to the requirements of the sequential or MPI version, you'll need:

    * A CUDA library
    * A runtime system among:
      * StarPU (https://starpu.gitlabpages.inria.fr)
      * PaRSEC (http://icl.utk.edu/parsec/)


### Download and install CUDA

We recommend a release >= 9.0 available on the [NVidia
website](https://developer.nvidia.com/CUDA-TOOLKIT-ARCHIVE).
Depending on the CUDA release you have chosen, your choice for the C
compiler will be limited to those supported by CUDA (
https://docs.nvidia.com/cuda/cuda-compiler-driver-nvcc/index.html#supported-host-compilers
) and more restrictively for [Linux
users](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html).

### Download and install StarPU

Please refer to [Installing PaStiX with runtime support](https://solverstack.gitlabpages.inria.fr/pastix/md_docs_doxygen_chapters_Pastix_Runtime.html)
for StarPU installation. The only step that will change for GPU support will be
during the configuration :

```sh
mkdir build_cuda
cd build_cuda
../configure --prefix=${STARPU_DIR} --enable-cuda
make
make install
```

### Download and install PaRSEC

Please refer to [Installing PaStiX with runtime support](https://solverstack.gitlabpages.inria.fr/pastix/md_docs_doxygen_chapters_Pastix_Runtime.html)
for PaRSEC installation. The only step that will change for GPU support will be
during the configuration :

```sh
mkdir build_cuda
cd build_cuda
cmake .. -DCMAKE_INSTALL_PREFIX=${PARSEC_DIR} -DPARSEC_GPU_WITH_CUDA=ON
make
make install
```

### Compile PaStiX with CUDA

Now that you have installed CUDA and at least one of the two runtime
systems as described above, you should be able to compile PaStiX with
GPU support:

Go in your PaStiX source directory and create a new build directory to
build the GPU version:
```sh
mkdir build_cuda
cd build_cuda
cmake .. -DCMAKE_INSTALL_PREFIX=${PASTIX_DIR} -DPASTIX_WITH_CUDA=ON \
    -DPASTIX_WITH_STARPU=[ON|OFF] -DPASTIX_WITH_PARSEC=[ON|OFF]
make
make install
```

Note that as before we defined a `PASTIX_DIR` variable to define the
install directory of PaStiX, and you just need to turn `ON`/`OFF` the
runtime systems based on the available one(s).

Once the compilation finished, you can setup your environment easily
by sourcing the provided file:
```sh
source $PASTIX_DIR/bin/pastix_env
```

And then, you can run your favorite example:
```sh
simple -9 10:10:10
```

You setup is ready to play with GPUs. Please refer to section
[How to use PaStiX with GPUs](https://solverstack.gitlabpages.inria.fr/pastix/md_docs_doxygen_chapters_How_PaStiX_GPU.html)
to get as best results as possible.
