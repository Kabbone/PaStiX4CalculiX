VERSION="$1"

export STARPU_SILENT=1
export STARPU_WORKERS_NOBIND=1
export STARPU_MPI_NOBIND=1
export PARSEC_MCA_runtime_bind_main_thread=0

if [[ "$SYSTEM" == "linux" ]]; then

  #OpenMPI (uncomment the following lines if OpenMPI is compiled from sources and set OPENMPI_DIR path)
  #export PATH=$PATH:$OPENMPI_DIR/bin
  #export LD_RUN_PATH=LD_RUN_PATH:$OPENMPI_DIR/lib
  #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENMPI_DIR/lib
  #export INCLUDE_PATH=$INCLUDE_PATH:$OPENMPI_DIR/include
  #export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$OPENMPI_DIR/lib/pkgconfig

  #PaRSEC mpi version (default is shm)
  if [ "$VERSION" == "mpi" ]
  then
    export PARSEC_DIR=/home/gitlab/install/parsec-mpi
  else
    export PARSEC_DIR=/home/gitlab/install/parsec-shm
  fi

  export PATH=$PARSEC_DIR/bin:$PATH
  export LD_LIBRARY_PATH=$PARSEC_DIR/lib:$LD_LIBRARY_PATH
  export PKG_CONFIG_PATH=$PARSEC_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

elif [[ "$SYSTEM" == "windows" ]]; then

  # this is required with BUILD_SHARED_LIBS=ON
  export PATH=$PATH:"/c/Windows/WinSxS/x86_microsoft-windows-m..namespace-downlevel_31bf3856ad364e35_10.0.19041.1_none_21374cb0681a6320"
  export PATH=$PATH:$PWD:$PWD/kernels:$PWD/test:$PWD/wrappers/fortran90:$PWD/spm/src:$PWD/spm/tests:$PWD/spm/wrappers/fortran90

fi
