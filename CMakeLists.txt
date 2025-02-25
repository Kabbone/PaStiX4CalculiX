###
#
#  @copyright 2013-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @version 6.4.0
#  @author Mathieu Faverge
#  @author Gregoire Pichon
#  @author Matias Hastaran
#  @author Pierre Ramet
#  @author Tony Delarue
#  @author Vincent Bridonneau
#  @author Xavier Lacoste
#  @author Nolan Bredel
#  @author Tom Moenne-Loccoz
#  @author Alban Bellot
#  @author Alycia Lisito
#  @author Brieuc Nicolas
#  @author Florent Pruvost
#  @author Mohamed Aymane Kherraz
#  @date 2024-07-09
#
###
cmake_minimum_required (VERSION 3.5)

if( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
  message(FATAL_ERROR "
    Because PaStiX generates many source files at compile time,
    Building in the source directory is forbidden.
    Please:
     1) remove entirely this directory
     2) extract or clone a fresh revision of pastix
     3) create a build directory and run cmake from this directory or
    run with (for exemple):
    mkdir build && cd build && cmake ..
    or
    cmake . -B./build
")
endif()

project (PASTIX C CXX Fortran)

include(CMakeDependentOption)
include(CheckFunctionExists)

# The current version number
set( PASTIX_VERSION_MAJOR 6 )
set( PASTIX_VERSION_MINOR 4 )
set( PASTIX_VERSION_MICRO 0 )

set( PASTIX_VERSION "${PASTIX_VERSION_MAJOR}.${PASTIX_VERSION_MINOR}.${PASTIX_VERSION_MICRO}" )

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if((NOT EXISTS "${PROJECT_SOURCE_DIR}/spm/CMakeLists.txt") OR (NOT EXISTS "${PROJECT_SOURCE_DIR}/cmake_modules/morse_cmake/modules/MorseInit.cmake"))
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# Add extra cmake module path and initialize morse cmake modules
# --------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)
include(AddSourceFiles)

if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/cmake_modules/morse_cmake/modules)
  set( MORSE_CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules/morse_cmake/modules )
  list(APPEND CMAKE_MODULE_PATH ${MORSE_CMAKE_MODULE_PATH})
  include(MorseInit)
else()
  message(FATAL_ERROR "Submodule cmake_morse not initialized - run `git submodule update --init`")
endif()

# Define precision supported by PaStiX
# ------------------------------------
set( RP_PASTIX_DICTIONNARY
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/local_subs.py )
set( RP_PASTIX_PRECISIONS  "p;s;d;c;z" )
include(RulesPrecisions)

### System parameter detection
include(CheckSystem)
include(CheckSubnormals)

# Set the RPATH config
# --------------------
set(CMAKE_MACOSX_RPATH 1)

# For fPIC when build static
set( CMAKE_POSITION_INDEPENDENT_CODE ON )

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

### Misc options
option(BUILD_SHARED_LIBS
  "Build shared libraries" OFF)

### Distributed engine parameters
option(PASTIX_WITH_MPI
    "Build PaStiX for distributed memory with MPI" OFF)

cmake_dependent_option(PASTIX_COMMUNICATION_MATRIX
        "Generates a matrix of communications between nodes" OFF "PASTIX_WITH_MPI" OFF)

cmake_dependent_option(PASTIX_BLEND_FANIN_FR
        "The fanin will be processed in full rank " OFF "PASTIX_WITH_MPI" OFF)

### GPU engine parameters
option(PASTIX_WITH_CUDA
    "Enable GPU support using CUDA kernels" OFF)

cmake_dependent_option(PASTIX_CUDA_FERMI
    "Enable the Fermi kernel instead of the batched kernel" OFF "PASTIX_WITH_CUDA" OFF)

### Tracing parameters
option(PASTIX_WITH_EZTRACE
  "Enable Eztrace for tracing CPU and GPU kernels" OFF)

option(PASTIX_WITH_PAPI
  "Enable PAPI for energy and power consumption" OFF)

### Runtime engines
option(PASTIX_WITH_STARPU
    "Build PaStiX with StarPU runtime support" OFF)

cmake_dependent_option(PASTIX_STARPU_PROFILING
    "Enable kernel profiling depending on worker" OFF "PASTIX_WITH_STARPU" OFF)

cmake_dependent_option(PASTIX_STARPU_PROFILING_LOG
    "Enable kernel profiling on each task individually" OFF "PASTIX_STARPU_PROFILING" OFF)

cmake_dependent_option(PASTIX_STARPU_COST_PER_ARCH
    "Enable kernel perfmodel to depend on cost function" OFF "PASTIX_WITH_STARPU" OFF)

cmake_dependent_option(PASTIX_STARPU_HETEROPRIO
    "Enable heteroprio scheduling with GPU instead of dmdas" OFF "PASTIX_WITH_STARPU" OFF)

cmake_dependent_option(PASTIX_STARPU_SYNC
        "Enable sync mode with StarPU" OFF "PASTIX_WITH_STARPU" OFF)

cmake_dependent_option(PASTIX_STARPU_STATS
        "Enable stats on StarPU tasks submission" OFF "PASTIX_WITH_STARPU" OFF)

option(PASTIX_WITH_PARSEC
  "Build PaStiX with PaRSEC runtime support" OFF)

# Internal options
option(PASTIX_GENERATE_MODEL
  "Enable performances profiling for model generation" OFF)

# Internal options
option(PASTIX_WITH_FORTRAN
  "Enable Fortran files/interface/examples to be compiled" ON)

option(PASTIX_INT64
  "Choose between int32 and int64 for integer representation" ON)

# Precisions generated
if(NOT PASTIX_PRECISIONS)
  set(PASTIX_PRECISIONS "s;d;c;z" CACHE STRING "The precisions to compile in PaSTiX (accepts a colon separated list of s;d;c;z)" FORCE)
else()
  set(PASTIX_PRECISIONS "${PASTIX_PRECISIONS}" CACHE STRING "The precisions to compile in PaSTiX (accepts a colon separated list of s;d;c;z)" FORCE)
endif()

# Ordering step options
option(PASTIX_ORDERING_SCOTCH
  "Enable Scotch Ordering" ON)
option(PASTIX_ORDERING_METIS
  "Enable Metis ordering" OFF)
cmake_dependent_option(PASTIX_ORDERING_PTSCOTCH
  "Activate the PT-scotch ordering (requires PASTIX_ORDERING_SCOTCH)" OFF
  "PASTIX_WITH_MPI;PASTIX_ORDERING_SCOTCH" OFF)
option(PASTIX_ORDERING_FIX_SEED
  "Fix the value of the random seed for ordering algorithm (when possible, Debug purpose)." ON )

# Symbolic factorization options
option(PASTIX_SYMBOL_DUMP_SYMBMTX
  "Dump the generated symbol matrix in a postscript file" OFF)
option(PASTIX_ORDER_DRAW_LASTSEP
  "Dump the last separator in a ivview file" OFF)

# Analyze step options
option(PASTIX_BLEND_DEEPEST_DISTRIB
  "Enable the candidate distribution to go to the deepest node matching the criterion" ON)
option(PASTIX_BLEND_PROPMAP_2STEPS
  "Enable the two stages proportionnal mapping" OFF)

# Numerical factorization options
option(PASTIX_NUMFACT_DUMP_SOLVER
  "Dump the generated factorized matrix in a postscript file with information on block's compressibility" OFF)
option(PASTIX_SUPERNODE_STATS
  "Enable statistics per original supernodes" OFF)

# Options to check
option(FORGET_PARTITION
  "Force to forget the partition generated by Scotch" OFF)
option(COMPACT_SMX
  "Optimization for solve computations (TODO: check if not obsolete because results don't converge)" OFF)

# Options to increase verbosity while debuging
option(PASTIX_DEBUG_VALGRIND
  "Make the compilation valgrind compatible to avoid live-lock in infinite loops" OFF)
option(PASTIX_DEBUG_GRAPH
  "Debug the graph data structure" OFF)
option(PASTIX_DEBUG_ORDERING
  "Debug the ordering step" OFF)
option(PASTIX_DEBUG_SYMBOL
  "Debug the symbol matrix factorization" OFF)
option(PASTIX_DEBUG_BLEND
  "Debug the analyze step" OFF)
option(PASTIX_DEBUG_DUMP_COEFTAB
  "Dump all cblk on disk after modifications" OFF)
cmake_dependent_option(PASTIX_DEBUG_FACTO
  "Debug the factorization step" OFF
  "PASTIX_WITH_MPI" OFF)
option(PASTIX_DEBUG_SOLVE
  "Debug the solve step" OFF)
cmake_dependent_option(PASTIX_DEBUG_PARSEC
  "Debug the parsec implementation of the factorization step" OFF
  "PASTIX_WITH_PARSEC" OFF)
cmake_dependent_option(PASTIX_DEBUG_STARPU
  "Debug the starpu implementation of the factorization step" OFF
  "PASTIX_WITH_STARPU" OFF)
option(PASTIX_DEBUG_LR
  "Debug the lowrank kernels (Valgrind)" OFF)
option(PASTIX_DEBUG_LR_NANCHECK
  "Debug the lowrank kernels (nancheck)" OFF)
option(PASTIX_DEBUG_GMRES
  "Debug the gmres refinment by enabling the computation of the true resiudal at each iteration" OFF)
cmake_dependent_option(PASTIX_DEBUG_MPI
  "Dump all the MPI messages" OFF
  "PASTIX_WITH_MPI" OFF)

# Option to generate LR TESTINGS (convenient if issues with tmg)
option(PASTIX_LR_TESTINGS
  "Enable the generation of low-rank testings (Requires LAPACKE with TMG support)" OFF)

#########################
# Look for dependencies #
#########################

# PaStiX depends on the libm
#---------------------------
find_package(M REQUIRED)
morse_export_imported_target(MORSE::M m pastix)

# PaStiX depends on LAPACKE and CBLAS
#------------------------------------
find_package(CBLAS REQUIRED)
morse_export_imported_target(MORSE::CBLAS cblas pastix)

## Tests requires TMG lib to generate random matrices.
if (PASTIX_LR_TESTINGS)
  find_package(LAPACKE REQUIRED
    COMPONENTS TMG)
else()
  find_package(LAPACKE REQUIRED)
endif()
morse_export_imported_target(MORSE::LAPACKE lapacke pastix)

# PaStiX depends on HwLoc
#---------------------------
find_package(HWLOC)
if (NOT BUILD_SHARED_LIBS)
  morse_export_imported_target(MORSE::HWLOC hwloc pastix)
endif()
set(HAVE_HWLOC ${HWLOC_FOUND})

# PaStiX might depend on MPI
#---------------------------
if (PASTIX_WITH_MPI)
  find_package(MPI REQUIRED)
  morse_export_imported_target(MPI::MPI_C mpic pastix)
  if (PASTIX_WITH_FORTRAN)
    morse_export_imported_target(MPI::MPI_Fortran mpif pastix)
  endif()

  set(pastix_mpiexec ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 4 --bind-to none --host localhost:4 )

endif (PASTIX_WITH_MPI)

# PaStiX might depend on CUDA
#----------------------------
if( PASTIX_WITH_CUDA )

  # When our cmake_minimum version will be 3.17 we will be able to use this
  # far better module: FindCUDAToolkit
  # https://cmake.org/cmake/help/latest/module/FindCUDAToolkit.html#module:FindCUDAToolkit

  find_package(CUDA REQUIRED)

  set(HAVE_CUDA ${CUDA_FOUND})
  if (CUDA_FOUND)
    # create imported target because not provided with old cmake
    add_library(CUDA::CUDA INTERFACE IMPORTED)
    add_library(CUDA::CUBLAS INTERFACE IMPORTED)

    if(CUDA_VERSION VERSION_LESS "3.0")
      set(CUDA_HOST_COMPILATION_CPP OFF)
    endif(CUDA_VERSION VERSION_LESS "3.0")
    set(CUDA_BUILD_EMULATION OFF)

    if (CUDA_INCLUDE_DIRS)
      set_target_properties(CUDA::CUDA PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIRS}")
    else()
      message(WARNING "PASTIX_WITH_CUDA requires"
        "\n   CUDA_INCLUDE_DIRS to be found. Be sure you have"
        "\n   cuda headers with your distribution of CUDA.")
    endif()

    if (CUDA_LIBRARIES)
      set_target_properties(CUDA::CUDA PROPERTIES INTERFACE_LINK_LIBRARIES "${CUDA_LIBRARIES}")

      # Check cuDeviceCanAccessPeer
      set(CMAKE_REQUIRED_INCLUDES  "${CUDA_INCLUDE_DIRS}")
      set(CMAKE_REQUIRED_LIBRARIES "${CUDA_LIBRARIES}")
      if(CUDA_VERSION VERSION_LESS "4.0")
        set(CUDA_HAVE_PEER_DEVICE_MEMORY_ACCESS 0)
      else()
        check_function_exists(cuDeviceCanAccessPeer CUDA_HAVE_PEER_DEVICE_MEMORY_ACCESS)
      endif()
      unset(CMAKE_REQUIRED_INCLUDES)
      unset(CMAKE_REQUIRED_LIBRARIES)

      # Add cublas if found
      if (CUDA_CUBLAS_LIBRARIES)
        set_target_properties(CUDA::CUBLAS PROPERTIES INTERFACE_LINK_LIBRARIES "${CUDA_CUBLAS_LIBRARIES}")
        target_link_libraries(CUDA::CUBLAS INTERFACE CUDA::CUDA)
      else()
        message(FATAL_ERROR "PASTIX_WITH_CUDA requires"
          "\n   CUDA_CUBLAS_LIBRARIES to be found. Be sure you have"
          "\n   libcublas with your distribution of CUDA.")
      endif()
    else()
      message(FATAL_ERROR "PASTIX_WITH_CUDA_CUDA requires"
        "\n   CUDA_LIBRARIES to be found. Be sure you have"
        "\n   libcuda with your distribution of CUDA.")
    endif()
    morse_export_imported_target(CUDA::CUDA cuda pastix)
    morse_export_imported_target(CUDA::CUBLAS cublas pastix)
  endif (CUDA_FOUND)
endif()

# PaStiX might depend on StarPU
#------------------------------
if (PASTIX_WITH_STARPU)

  # Specify the minimum version
  set( PASTIX_STARPU_VERSION "1.4" CACHE STRING "oldest STARPU version desired" )

  find_package( STARPU ${PASTIX_STARPU_VERSION} REQUIRED )
  morse_export_imported_target(MORSE::STARPU starpu pastix)

  get_target_property(CMAKE_REQUIRED_LIBRARIES MORSE::STARPU INTERFACE_LINK_LIBRARIES)

  # Check for specific functions
  check_function_exists(starpu_sched_ctx_get_sched_policy_callback HAVE_STARPU_SCHED_POLICY_CALLBACK)

  unset(CMAKE_REQUIRED_LIBRARIES)

endif (PASTIX_WITH_STARPU)

# PaStiX might depend on Parsec
#------------------------------
if (PASTIX_WITH_PARSEC)

  find_package( PARSEC REQUIRED )
  morse_export_imported_target(MORSE::PARSEC parsec pastix)

  # Add the index-array dep management option to PTGCC compilation flags.
  include(RulesJDF)
  set(PARSEC_PTGPP_CFLAGS "--dep-management;index-array;${PARSEC_PTGPP_CFLAGS}")

endif (PASTIX_WITH_PARSEC)

### Add trace generation
if(PASTIX_WITH_EZTRACE)
  if(PASTIX_WITH_MPI)
    find_package(EZTRACE REQUIRED
                 COMPONENTS MPI)
  else()
    find_package(EZTRACE REQUIRED)
  endif()
  find_package(LITL REQUIRED)
endif(PASTIX_WITH_EZTRACE)

# PaStiX depends on PAPI
#---------------------------
if ( PASTIX_WITH_PAPI )
  find_package(PAPI REQUIRED)
endif ()

# PaStiX might depend on Scotch/PT-Scotch
#----------------------------------------
if (PASTIX_ORDERING_PTSCOTCH)
  find_package(PTSCOTCH)
  if (NOT BUILD_SHARED_LIBS)
    morse_export_imported_target(MORSE::PTSCOTCH ptscotch pastix)
  endif()

  if (PTSCOTCH_FOUND)
    # Check coherency for integer size
    if(PASTIX_INT64 AND NOT PTSCOTCH_Num_8)
      message(FATAL_ERROR "PASTIX_INT64 is enabled and provided PT-Scotch is not compiled with int64 support, please build with -DPASTIX_INT64=OFF or install a 64 bits version of PT-Scotch.")
    endif()
    if(NOT PASTIX_INT64 AND NOT PTSCOTCH_Num_4)
      message(FATAL_ERROR "PASTIX_INT64 is disabled and provided PT-Scotch is not compiled with int32 support, please build with -DPASTIX_INT64=ON or install a 32 bits version of PT-Scotch.")
    endif()
  endif()
endif()

if (PASTIX_ORDERING_SCOTCH)
  find_package(SCOTCH REQUIRED)
  if (NOT BUILD_SHARED_LIBS)
    morse_export_imported_target(MORSE::SCOTCH scotch pastix)
  endif()

  if (SCOTCH_FOUND)
    cmake_dependent_option(PASTIX_ORDERING_SCOTCH_MT
    "Enable multi-threaded ordering step With Scotch library (require Scotch v6.1 or above)" ON
    "HAVE_SCOTCH_CONTEXT_INIT" OFF)

    # Check coherency for integer size
    if(PASTIX_INT64 AND NOT SCOTCH_Num_8)
      message(FATAL_ERROR "PASTIX_INT64 is enabled and provided Scotch is not compiled with int64 support, please build with -DPASTIX_INT64=OFF or install a 64 bits version of Scotch.")
    endif()
    if(NOT PASTIX_INT64 AND NOT SCOTCH_Num_4)
      message(FATAL_ERROR "PASTIX_INT64 is disabled and provided Scotch is not compiled with int32 support, please build with -DPASTIX_INT64=ON or install a 32 bits version of Scotch.")
    endif()
  endif()
endif()

# PaStiX might depend on Metis/ParMetis
#--------------------------------------
if (PASTIX_ORDERING_METIS)
  find_package(METIS REQUIRED)
  if (NOT BUILD_SHARED_LIBS)
    morse_export_imported_target(MORSE::METIS metis pastix)
  endif()

  if (METIS_FOUND)
    # Check coherency for integer size
    if(PASTIX_INT64 AND NOT METIS_Idx_8)
      message(FATAL_ERROR "PASTIX_INT64 is enabled and provided Metis is not compiled with int64 support.")
    endif()
    if(NOT PASTIX_INT64 AND NOT METIS_Idx_4)
      message(FATAL_ERROR "PASTIX_INT64 is disabled and provided Metis is not compiled with int32 support.")
    endif()
  endif()
endif()

# PaStiX might depend on GTG
#---------------------------
find_package(GTG QUIET)

if (GTG_FOUND)
  # Symbolic factorization options
  option(PASTIX_BLEND_GENTRACE
    "Allow trace generation in Blend simulation" OFF)
else()
  set( PASTIX_BLEND_GENTRACE OFF )
endif()

# valgrind detection
#-------------------
find_program( MEMORYCHECK_COMMAND valgrind )
set( MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full" )

#
##
###
# Finished detecting the system, lets do our own things now
###
##
#

# Compile the spm library
option( PASTIX_WITH_EXTERNAL_SPM "Force the use of an external SpM library that should be correctly set in the environment variables" OFF )
set( PASTIX_SPM_VERSION "1.2.4" CACHE STRING "Minimal SPM API" )

if ( PASTIX_WITH_EXTERNAL_SPM )
  find_package( SPM REQUIRED )
  if ( SPM_FOUND )
    if( (PASTIX_INT64 AND (NOT SPM_INTSIZE EQUAL 8)) OR (PASTIX_INT32 AND (NOT SPM_INTSIZE EQUAL 4)) )
      message( WARNING "PASTIX_INT64 is enabled and provided SPM does not have the same support. Internal SPM will be compiled.")
      unset( SPM_FOUND )
    endif()
  endif()

  # Final check
  if ( NOT SPM_FOUND )
    message( FATAL_ERROR "No external SpM library found. Please disable PASTIX_WITH_EXTERNAL_SPM or correctly set your environment" )
  endif()
else()
  set( SPM_FOUND FALSE )
endif()

if ( SPM_FOUND )
  message( STATUS "Use installed SPM" )
else()
  message( STATUS "Use internal SPM" )
  set( SPM_INT64        ${PASTIX_INT64}           CACHE BOOL "" FORCE )
  set( SPM_WITH_FORTRAN ${PASTIX_WITH_FORTRAN}    CACHE BOOL "" FORCE )
  set( SPM_WITH_MPI     ${PASTIX_WITH_MPI}        CACHE BOOL "" FORCE )
  set( SPM_WITH_SCOTCH  ${PASTIX_ORDERING_SCOTCH} CACHE BOOL "" FORCE )
  mark_as_advanced( SPM_INT64        )
  mark_as_advanced( SPM_WITH_FORTRAN )
  mark_as_advanced( SPM_WITH_MPI     )
  mark_as_advanced( SPM_WITH_SCOTCH  )
  add_subdirectory(spm)
  add_library(SPM::spm ALIAS spm)
  if ( SPM_WITH_FORTRAN )
    add_library(SPM::spmf ALIAS spmf)
  endif()
endif()

# Disable restrict (temporary)
if(CMAKE_C_FLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Drestrict=")
else()
  set(CMAKE_C_FLAGS "-Drestrict=")
endif()

#Configuration header
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/include/pastix/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/pastix/config.h")
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/include/pastix/config.h"
  DESTINATION include/pastix )

# Sub modules
set(modules kernels refinement) #csc kernels sopalin common)
foreach (module ${modules})
   add_subdirectory(${module})
endforeach()

if (PASTIX_WITH_PARSEC)
    add_subdirectory(sopalin/parsec)
endif()

if (PASTIX_WITH_STARPU)
    add_subdirectory(sopalin/starpu)
endif()

include(RulesPrecisions)

##
# Headers from bcsc directory
##
set(generated_bcsc_headers "")
set(HEADERS
  bcsc/bcsc_z.h
)

precisions_rules_py(generated_bcsc_headers
  "${HEADERS}"
  TARGETDIR  "bcsc"
  PRECISIONS "p;s;d;c;z")

add_custom_target(bcsc_headers_tgt
  DEPENDS ${generated_bcsc_headers} )

##
# Headers from sopalin directory
##
set(generated_sopalin_headers "")
set(HEADERS
  sopalin/coeftab_z.h
)

precisions_rules_py(generated_sopalin_headers
  "${HEADERS}"
  TARGETDIR  "sopalin"
  PRECISIONS "p;s;d;c;z")

add_custom_target( sopalin_headers
  DEPENDS ${generated_sopalin_headers} )

###############
# Source files

##
# Source files from common directory
##
set(generated_common_sources "")
set(SOURCES
  common/z_integer.c
  )
precisions_rules_py(generated_common_sources
  "${SOURCES}"
  TARGETDIR  "common"
  PRECISIONS "p;s;d;c;z"
  )

##
# Source files from bcsc directory
##
set(generated_bcsc_sources "")
set(SOURCES
  bcsc/bcsc_zinit.c
  bcsc/bcsc_znorm.c
  bcsc/bcsc_zspmv.c
  bcsc/bvec_zcompute.c
  bcsc/bvec_zlapmr.c
  bcsc/bvec_zmpi_comm.c
  )
precisions_rules_py(generated_bcsc_sources
  "${SOURCES}"
  TARGETDIR  "bcsc"
  PRECISIONS "p;s;d;c;z"
  )

##
# Source files from sopalin directory
##
set(generated_sopalin_sources "")
set(SOURCES
  sopalin/coeftab_z.c
  sopalin/coeftab_zinit.c
  sopalin/sequential_zdiag.c
  sopalin/sequential_zgetrf.c
  sopalin/sequential_zhetrf.c
  sopalin/sequential_zpotrf.c
  sopalin/sequential_zpxtrf.c
  sopalin/sequential_zsytrf.c
  sopalin/sequential_ztrsm.c
  )
precisions_rules_py(generated_sopalin_sources
  "${SOURCES}"
  TARGETDIR  "sopalin"
  PRECISIONS "p;s;d;c;z"
  )

set(generated_sopalin_mixed_sources "")
set(SOURCES
  sopalin/coeftab_zcinit.c
  )
precisions_rules_py(generated_sopalin_mixed_sources
  "${SOURCES}"
  TARGETDIR "sopalin"
  PRECISIONS "ds;zc")

##
# Source files from refinement directory
##
set(generated_refinement_sources "")
set(SOURCES
  refinement/z_refine_bicgstab.c
  refinement/z_refine_functions.c
  refinement/z_refine_gmres.c
  refinement/z_refine_grad.c
  refinement/z_refine_pivot.c
  )
precisions_rules_py(generated_refinement_sources
  "${SOURCES}"
  TARGETDIR  "refinement"
  PRECISIONS "p;s;d;c;z"
  )

#
# Build the pastix library
#
set(PASTIX_LIB_SRCS
  # Files that are fully converted
  #
  graph/graph.c
  graph/graph_compute_projection.c
  graph/graph_connected_components.c
  graph/graph_io.c
  graph/graph_isolate.c
  graph/graph_prepare.c
  #
  order/order.c
  order/order_add_isolate.c
  order/order_amalgamate.c
  order/order_apply_level_order.c
  order/order_check.c
  order/order_compute_personal.c
  order/order_find_supernodes.c
  order/order_grids.c
  order/order_io.c
  order/pastix_subtask_order.c
  #
  symbol/fax_csr.c
  symbol/fax_csr_amalgamate.c
  symbol/fax_csr_direct.c
  symbol/fax_csr_iluk.c
  symbol/symbol.c
  symbol/symbol_base.c
  symbol/symbol_check.c
  symbol/symbol_cost.c
  symbol/symbol_cost_flops.c
  symbol/symbol_cost_perfs.c
  symbol/symbol_draw.c
  symbol/symbol_draw_map.c
  symbol/symbol_expand.c
  symbol/symbol_fax_direct.c
  symbol/symbol_fax_iluk.c
  symbol/symbol_io.c
  symbol/symbol_reorder.c
  symbol/symbol_reordering.c
  symbol/pastix_subtask_reordering.c
  symbol/pastix_subtask_symbfact.c

  # Files that still require some changes
  blend/pastix_subtask_blend.c
  blend/pastix_task_analyze.c

  blend/blendctrl.c
  blend/cand.c
  blend/cand_gendot.c
  blend/cost.c
  blend/elimintree.c
  blend/extendVector.c
  blend/extracblk.c
  blend/propmap.c
  blend/simu.c
  blend/simu_run.c
  blend/simu_task.c
  blend/solver.c
  blend/solver_backup.c
  blend/solver_check.c
  blend/solver_copy.c
  blend/solver_draw.c
  blend/solver_io.c
  blend/solver_matrix_gen.c
  blend/solver_matrix_gen_utils.c
  blend/solver_recv.c
  blend/splitsymbol.c

  common/api.c
  common/integer.c
  common/isched.c
  common/models.c
  common/get_options.c
  common/parse_options.c
  common/check_options.c
  common/getline.c
  #
  bcsc/bcsc.c
  bcsc/bvec.c
  #
  sopalin/coeftab.c
  sopalin/schur.c
  sopalin/diag.c
  sopalin/pastix_task_sopalin.c
  sopalin/pastix_task_solve.c
  sopalin/pastix.c
  sopalin/pastix_rhs.c
  #
  refinement/pastix_task_refine.c
  #
  ${generated_common_sources}
  ${generated_bcsc_sources}
  ${generated_sopalin_sources}
  ${generated_sopalin_mixed_sources}
  ${generated_refinement_sources}
  )

set_source_files_properties(
  sopalin/coeftab.h
  PROPERTIES DEPENDS sopalin_headers
  )

if(PASTIX_ORDERING_SCOTCH)
  set(PASTIX_LIB_SRCS
    ${PASTIX_LIB_SRCS}
    graph/graph_compute_kway.c
    order/order_compute_scotch.c
    order/order_draw.c
    order/order_supernodes.c
    order/order_scotch_common.c
    )
endif()
if(PASTIX_ORDERING_PTSCOTCH)
  set(PASTIX_LIB_SRCS
    ${PASTIX_LIB_SRCS}
    order/order_compute_ptscotch.c
    )
endif()
if(PASTIX_ORDERING_METIS)
  list(APPEND PASTIX_LIB_SRCS
    order/order_compute_metis.c
    )
endif()
if(HWLOC_FOUND)
  list(APPEND PASTIX_LIB_SRCS
    common/isched_hwloc.c
    )
else()
  list(APPEND PASTIX_LIB_SRCS
    common/isched_nohwloc.c
    )
endif()

if(PASTIX_COMMUNICATION_MATRIX)
  list(APPEND PASTIX_LIB_SRCS
    blend/solver_comm_matrix.c
    )
endif()

if(PASTIX_WITH_PAPI)
  list(APPEND PASTIX_LIB_SRCS
    common/pastix_papi.c
  )
endif()


add_library( pastix
  ${PASTIX_LIB_SRCS}
  )

set_target_properties( pastix PROPERTIES
  VERSION ${PASTIX_VERSION}
  SOVERSION ${PASTIX_VERSION_MAJOR}.${PASTIX_VERSION_MINOR} )

add_dependencies(pastix
  kernels_headers_tgt
  bcsc_headers_tgt
  refinement_headers_tgt
  sopalin_headers
)

target_include_directories( pastix PRIVATE
  $<BUILD_INTERFACE:${PASTIX_SOURCE_DIR}>
  $<BUILD_INTERFACE:${PASTIX_BINARY_DIR}>
  $<BUILD_INTERFACE:${PASTIX_SOURCE_DIR}/common> )

target_include_directories( pastix PUBLIC
  $<BUILD_INTERFACE:${PASTIX_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PASTIX_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include> )

if (HAVE_BLAS_SET_NUM_THREADS AND CBLAS_INCLUDE_DIRS)
  target_include_directories( pastix PRIVATE
    $<BUILD_INTERFACE:${CBLAS_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:${CBLAS_INCLUDE_DIRS}> )
endif()

target_link_libraries( pastix PUBLIC  SPM::spm )
target_link_libraries( pastix PRIVATE pastix_kernels )
target_link_libraries( pastix PRIVATE Threads::Threads )

if(PASTIX_WITH_MPI)
  target_link_libraries( pastix PUBLIC MPI::MPI_C )
endif()

if(PASTIX_ORDERING_PTSCOTCH)
  target_link_libraries( pastix PRIVATE MORSE::PTSCOTCH )
endif()
if(PASTIX_ORDERING_SCOTCH)
  target_link_libraries( pastix PRIVATE MORSE::SCOTCH )
endif()
if(PASTIX_ORDERING_METIS)
  target_link_libraries( pastix PRIVATE MORSE::METIS )
endif()
if(PASTIX_WITH_PAPI)
  target_link_libraries( pastix PRIVATE MORSE::PAPI )
endif()

if(PASTIX_WITH_PARSEC)
  target_link_libraries( pastix PRIVATE
    pastix_parsec
    )

  add_dependencies(pastix
    parsec_headers_tgt
    )
endif()

if(PASTIX_WITH_STARPU)
  target_link_libraries(pastix PRIVATE
    pastix_starpu
    )

  add_dependencies(pastix
    starpu_headers_tgt
    )
endif()

if (PASTIX_BLEND_GENTRACE)
  target_link_libraries(pastix PRIVATE
    MORSE::GTG
    )
endif()

if(HWLOC_FOUND)
  target_link_libraries( pastix PRIVATE
    MORSE::HWLOC )
endif()
# foreach (module2 ${modules})
#   add_dependencies(pastix ${module2}_headers)
# endforeach()

# Install header files
set(PASTIX_HDRS
  include/pastix/api.h
  include/pastix/old_api.h
  include/pastix/datatypes.h
  include/pastix/order.h
  graph/graph.h
  include/cblas.h
  include/lapacke.h
)
install(FILES
  include/pastix.h
  DESTINATION include )
install(FILES ${PASTIX_HDRS}
  DESTINATION include/pastix )

# export target pastix
install(EXPORT pastixTargets
        NAMESPACE PASTIX::
        DESTINATION lib/cmake/pastix
        )

# Installation
# ------------
install( TARGETS pastix
  EXPORT pastixTargets
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include )

## Executable and tests
enable_testing()
include(CTest)
# Examples executables
add_subdirectory(example)
# Testing executables
add_subdirectory(test)

### Wrappers
add_subdirectory(wrappers)

### Build pkg-config and environment file
include(GenPkgConfig)

set(PASTIX_PKGCONFIG_LIBS pastix pastix_kernels)

# StarPU
if(PASTIX_WITH_STARPU)
  list(APPEND PASTIX_PKGCONFIG_LIBS pastix_starpu)
  if ( STARPU_FOUND_WITH_PKGCONFIG )
    if ( PASTIX_WITH_MPI )
      list(APPEND PASTIX_PKGCONFIG_REQUIRED starpumpi-${PASTIX_STARPU_VERSION})
    else()
      list(APPEND PASTIX_PKGCONFIG_REQUIRED starpu-${PASTIX_STARPU_VERSION})
    endif()
  else()
    list(APPEND PASTIX_PKGCONFIG_LIBS_PRIVATE ${STARPU_LIBRARIES_DEP})
  endif()
endif()

# PaRSEC
if(PASTIX_WITH_PARSEC)
  list(APPEND PASTIX_PKGCONFIG_LIBS pastix_parsec)
  if ( PARSEC_FOUND_WITH_PKGCONFIG )
    list(APPEND PASTIX_PKGCONFIG_REQUIRED parsec)
  else()
    list(APPEND PASTIX_PKGCONFIG_LIBS_PRIVATE ${PARSEC_LIBRARIES_DEP})
  endif()
endif()

if(PASTIX_WITH_CUDA)
  list(APPEND PASTIX_PKGCONFIG_LIBS pastix_kernels_cuda)
endif()

if(PASTIX_WITH_EZTRACE)
  list(APPEND PASTIX_PKGCONFIG_REQUIRED eztrace litl)
endif()

list(APPEND PASTIX_PKGCONFIG_INCS
  )
list(APPEND PASTIX_PKGCONFIG_LIBS_PRIVATE
  ${LAPACKE_LIBRARIES_DEP}
  ${CBLAS_LIBRARIES_DEP}
  )
list(APPEND PASTIX_PKGCONFIG_LIBS_PRIVATE
  ${EXTRA_LIBRARIES}
  )

# HwLoc
if ( HWLOC_FOUND_WITH_PKGCONFIG )
  list(APPEND PASTIX_PKGCONFIG_REQUIRED hwloc)
else()
  list(APPEND PASTIX_PKGCONFIG_LIBS_PRIVATE ${HWLOC_LIBRARIES})
endif()

generate_pkgconfig_files(
  "${CMAKE_CURRENT_SOURCE_DIR}/pastix.pc.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/pastixf.pc.in"
  PROJECTNAME PASTIX )

generate_newenv_file( PACKAGE PaStiX )

# PaStiX auto-completion file
# (see https://github.com/scop/bash-completion)
# ---------------------------------------------
get_property( PASTIX_COMP_BINARIES GLOBAL PROPERTY PASTIX_COMP_BINARIES )
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/tools/pastix_completion.sh.in"
  "${CMAKE_CURRENT_BINARY_DIR}/bash-completion/pastix" @ONLY)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/bash-completion/pastix"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bash-completion/completions/pastix")

#
# Add uninstall target to remove installed files
#
if(NOT TARGET uninstall)
  configure_file(
    "${MORSE_CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

#############################################################
#
#      Build documentation
#
#############################################################
add_documented_files(
  README.md
  docs/doxygen/chapters/thread_bindings.doxy
  docs/doxygen/chapters/Pastix_Install.md
  docs/doxygen/chapters/Pastix_MPI.md
  docs/doxygen/chapters/Pastix_EZTRACE.md
  docs/doxygen/chapters/Pastix_Runtime.md
  docs/doxygen/chapters/Pastix_GPU.md
  docs/doxygen/chapters/How_PaStiX_GPU.md
  docs/doxygen/chapters/Pastix_Guide.md
  docs/doxygen/chapters/Pastix_benchmarking.md
  docs/doxygen/chapters/Pastix_Python.md
  #
  tools/binding_for_multimpi.c
  #
  include/pastix.h
  include/pastix/api.h
  include/pastix/datatypes.h
  include/pastix/order.h
  common/api.c
  common/get_options.c
  common/models.c
  common/pastixdata.h
  common/isched_hwloc.c
  #
  graph/graph.h
  graph/graph.c
  graph/graph_compute_kway.c
  graph/graph_compute_projection.c
  graph/graph_connected_components.c
  graph/graph_io.c
  graph/graph_isolate.c
  graph/graph_prepare.c
  #
  include/pastix/order.h
  #order/order_scotch_strats.h TODO: include in developer version
  order/order.c
  order/order_add_isolate.c
  order/order_amalgamate.c
  order/order_apply_level_order.c
  order/order_check.c
  order/order_draw.c
  order/order_find_supernodes.c
  order/order_grids.c
  order/order_io.c
  order/order_compute_metis.c
  order/order_compute_ptscotch.c
  order/order_compute_scotch.c
  order/order_compute_personal.c
  order/order_scotch_common.c
  order/order_supernodes.c
  order/pastix_subtask_order.c
  #
  symbol/fax_csr.h
  symbol/symbol.h
  symbol/symbol_cost.h  #TODO: include in developer version
  symbol/symbol_fax.h   #TODO: include in developer version
  symbol/fax_csr.c
  symbol/fax_csr_amalgamate.c
  symbol/fax_csr_direct.c
  symbol/fax_csr_iluk.c
  symbol/symbol.c
  symbol/symbol_base.c
  symbol/symbol_check.c
  symbol/symbol_cost.c
  symbol/symbol_cost_flops.c
  symbol/symbol_cost_perfs.c
  symbol/symbol_draw.c
  symbol/symbol_draw_map.c
  symbol/symbol_expand.c
  #symbol/symbol_fax.c Not documented: template file
  symbol/symbol_fax_direct.c
  symbol/symbol_fax_iluk.c
  symbol/symbol_io.c
  symbol/symbol_reorder.c
  symbol/symbol_reordering.c
  symbol/pastix_subtask_reordering.c
  symbol/pastix_subtask_symbfact.c
  #
  blend/blend.h
  blend/blendctrl.h
  blend/blendctrl.c
  blend/cand.h
  blend/cand.c
  blend/cand_gendot.c
  blend/cost.h
  blend/cost.c
  blend/elimintree.h
  blend/elimintree.c
  blend/extendVector.h
  blend/extendVector.c
  blend/extracblk.h
  blend/extracblk.c
  blend/perf.h
  #
  blend/simu.h
  blend/simu_timer.h
  blend/simu.c
  blend/simu_run.c
  blend/simu_task.c
  #
  blend/solver.h
  blend/solver.c
  blend/solver_backup.c
  blend/solver_check.c
  blend/solver_copy.c
  blend/solver_draw.c
  blend/solver_io.c
  blend/solver_matrix_gen.c
  blend/solver_comm_matrix.h
  blend/solver_comm_matrix.c
  blend/solver_matrix_gen_utils.h
  blend/solver_matrix_gen_utils.c
  blend/solver_recv.c
  #
  blend/propmap.c
  blend/splitsymbol.c
  blend/pastix_subtask_blend.c
  blend/pastix_task_analyze.c
  #
  bcsc/bcsc.c
  bcsc/bcsc.h
  bcsc/bvec.c
  bcsc/bvec.h
  #
  sopalin/coeftab.h
  sopalin/coeftab.c
  sopalin/diag.c
  sopalin/schur.c
  sopalin/pastix.c
  sopalin/pastix_rhs.c
  sopalin/pastix_task_sopalin.c
  sopalin/pastix_task_solve.c
  #
  refinement/pastix_task_refine.c
  #
  example/analyze.c
  example/compress.c
  example/personal.c
  example/reentrant.c
  example/refinement.c
  example/schur.c
  example/simple.c
  example/step-by-step.c
  )

add_documented_files(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  ${generated_common_sources}
  ${generated_bcsc_headers}
  ${generated_bcsc_sources}
  ${generated_sopalin_headers}
  ${generated_sopalin_sources}
  ${generated_sopalin_mixed_sources}
  ${generated_refinement_sources}
)

#
# Build documentation
#
add_subdirectory(docs)

#-- Add a custom target to run Doxygen when ever the project is built
add_custom_target (tags
  COMMAND etags ${PASTIX_LIB_SRCS}
  DEPENDS ${PASTIX_LIB_SRCS} )

###############################################################################
# Export targets #
##################

# see https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)

set(BIN_INSTALL_DIR "bin/"     CACHE STRING "where to install executables relative to prefix" )
set(INC_INSTALL_DIR "include/" CACHE STRING "where to install headers relative to prefix"     )
set(LIB_INSTALL_DIR "lib/"     CACHE STRING "where to install libraries relative to prefix"   )

configure_package_config_file( cmake_modules/PASTIXConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/PASTIXConfig.cmake
  INSTALL_DESTINATION lib/cmake/pastix
  PATH_VARS BIN_INSTALL_DIR INC_INSTALL_DIR LIB_INSTALL_DIR)
write_basic_package_version_file(PASTIXConfigVersion.cmake
  VERSION ${PASTIX_VERSION}
  COMPATIBILITY AnyNewerVersion)

# Install config files
install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/PASTIXConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/PASTIXConfigVersion.cmake
  DESTINATION lib/cmake/pastix)

#############################################################
# Print Options
#############################################################
include(PrintOpts)
