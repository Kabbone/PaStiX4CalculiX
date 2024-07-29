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
#  @author Alycia Lisito
#  @date 2024-07-05
#
###
include(GenPkgConfig)

add_subdirectory(old)

set (INTERNAL_EXAMPLES
  dump_rank.c
  ordering_grid.c
  pastix_benchmark.c
  )

foreach (_file ${INTERNAL_EXAMPLES})
  get_filename_component(_name_we ${_file} NAME_WE)
  add_executable(${_name_we} ${_file})
  add_binary_to_completion( ${_name_we} )

  target_include_directories( ${_name_we} PRIVATE
    $<BUILD_INTERFACE:${PASTIX_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PASTIX_BINARY_DIR}> )
  target_link_libraries( ${_name_we} PRIVATE
    pastix
    pastix_kernels
    MORSE::LAPACKE )

endforeach()

set (EXAMPLES
  analyze.c
  bench_facto.c
  compress.c
  isolate.c
  multidof.c
  personal.c
  reentrant.c
  refinement.c
  schur.c
  simple.c
  simple_solve_and_refine.c
  simple_trans.c
  step-by-step.c
)

if ( PASTIX_WITH_MPI )
  set( EXAMPLES
    ${EXAMPLES}
    mpi_subproblems.c
  )
endif()

foreach (_file ${EXAMPLES})
  get_filename_component(_name_we ${_file} NAME_WE)
  add_executable(${_name_we} ${_file})
  add_binary_to_completion(${_name_we})

  target_include_directories( ${_name_we} PRIVATE
    $<BUILD_INTERFACE:${PASTIX_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PASTIX_BINARY_DIR}> )
  target_link_libraries( ${_name_we} PRIVATE
    pastix )

  install(TARGETS ${_name_we} RUNTIME DESTINATION share/doc/pastix/examples )
  install(FILES   ${_file}            DESTINATION share/doc/pastix/examples )
endforeach()

target_link_libraries( schur     PRIVATE MORSE::LAPACKE )
target_link_libraries( reentrant PRIVATE Threads::Threads )

### Install a makefile for user compilation test
set( EXAMPLE_LIBS ${DEPS_LIBRARIES} )
if ( EXAMPLE_LIBS )
  gpc_convert_libstyle_to_pkgconfig( EXAMPLE_LIBS )
  string(REPLACE ";" " " EXAMPLE_LIBS "${EXAMPLE_LIBS}")
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Makefile.in"
  "${CMAKE_CURRENT_BINARY_DIR}/make/Makefile" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/make/Makefile"
  DESTINATION share/doc/pastix/examples)

include(CTestLists.txt)
