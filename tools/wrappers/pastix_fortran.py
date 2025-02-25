#!/usr/bin/env python
"""
Wrapper Fortran 90
==================

 @file wrappers/pastix_fortran.py

 PaStiX Fortran wrapper variables

 @copyright 2017-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
                      Univ. Bordeaux. All rights reserved.

 @version 6.4.0
 @author Mathieu Faverge
 @author Tony Delarue
 @date 2024-07-05

"""
filename_prefix = "wrappers/fortran90/src/"

enums_fortran_footer='''
contains

  function pastix_getintsize()
    integer :: pastix_getintsize
    pastix_getintsize = kind(PASTIX_INT_KIND)
    return
  end function pastix_getintsize

'''

enums = {
    'filename'    : filename_prefix + 'pastixf_enums.F90',
    'description' : "PaStiX fortran 90 wrapper to define enums and datatypes",
    'header'      : """

  use, intrinsic :: iso_c_binding
#if defined(PASTIX_WITH_MPI)
  use :: mpi_f08, only : MPI_Comm, MPI_COMM_WORLD
#endif
  implicit none

#if defined(PASTIX_WITH_MPI)
  logical, parameter :: pastix_with_mpi = .TRUE.
#else
  logical, parameter :: pastix_with_mpi = .FALSE.

  type, bind(c) :: MPI_Comm
     integer(kind=c_int) :: MPI_VAL = 0
  end type MPI_Comm

  type(MPI_Comm), parameter :: MPI_COMM_WORLD = MPI_Comm(0)
#endif

  integer, parameter :: pastix_int_t = PASTIX_INT_KIND

  type, bind(c) :: pastix_data_t
     type(c_ptr) :: ptr
  end type pastix_data_t

  type, bind(c) :: pastix_rhs_t
     type(c_ptr) :: ptr
  end type pastix_rhs_t

  type, bind(c) :: pastix_graph_t
     type(c_ptr) :: ptr
  end type pastix_graph_t

""",
    'footer'      : enums_fortran_footer,
    'enums'       : { 'mtxtype'  : "    enumerator :: PastixSymPosDef = PastixConjTrans + 1\n    enumerator :: PastixHerPosDef    = PastixConjTrans + 2\n" }
}

interface = {
    'filename'    : filename_prefix + "pastixf_interfaces.f90",
    'description' : "PaStiX Fortran 90 wrapper",
    'header'      : "",
    'footer'      : """
  interface pastixOrderGetArray
     subroutine pastixOrderGetArray_f08( order, permtab, peritab, rangtab, treetab, sndetab )
       use :: pastixf_enums, only : pastix_order_t, pastix_int_t
       implicit none
       type(pastix_order_t),                intent(in),            target  :: order
       integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: permtab
       integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: peritab
       integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: rangtab
       integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: treetab
       integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: sndetab
     end subroutine pastixOrderGetArray_f08
  end interface pastixOrderGetArray

""",
    'enums'       : {}
}

functions = {
    'filename'    : filename_prefix + 'pastixf_functions.f90',
    'description' : "PaStiX Fortran interface implementation",
    'header'      : """
function pastixGetCptrFromValue(input) result(output)
  use :: iso_c_binding, only : c_double_complex, c_float_complex, c_double, c_float, c_ptr, c_null_ptr, c_loc
  implicit none

  class(*),   target :: input
  type(c_ptr)        :: output

  select type(t=>input)
  type is (complex(c_double_complex))
     output = c_loc( t )
  type is (complex(c_float_complex))
     output = c_loc( t )
  type is (real(c_double))
     output = c_loc( t )
  type is (real(c_float))
     output = c_loc( t )
  class default
     write(0,*) "Input pointer is not pointing to one of double complex, single complex, double real or single real"
     output = c_null_ptr
  end select

end function pastixGetCptrFromValue

function pastixGetCptrFrom1dArray(input) result(output)
  use :: iso_c_binding, only : c_double_complex, c_float_complex, c_double, c_float, c_ptr, c_null_ptr, c_loc
  implicit none

  class(*), dimension(:), target :: input
  type(c_ptr)                    :: output

  select type(t=>input)
  type is (complex(c_double_complex))
     output = c_loc( t )
  type is (complex(c_float_complex))
     output = c_loc( t )
  type is (real(c_double))
     output = c_loc( t )
  type is (real(c_float))
     output = c_loc( t )
  class default
     write(0,*) "Input pointer is not pointing to one of double complex, single complex, double real or single real"
     output = c_null_ptr
  end select

end function pastixGetCptrFrom1dArray

function pastixGetCptrFrom2dArray(input) result(output)
  use :: iso_c_binding, only : c_double_complex, c_float_complex, c_double, c_float, c_ptr, c_null_ptr, c_loc
  implicit none

  class(*), dimension(:,:), target :: input
  type(c_ptr)                      :: output

  select type(t=>input)
  type is (complex(c_double_complex))
     output = c_loc( t )
  type is (complex(c_float_complex))
     output = c_loc( t )
  type is (real(c_double))
     output = c_loc( t )
  type is (real(c_float))
     output = c_loc( t )
  class default
     write(0,*) "Input pointer is not pointing to one of double complex, single complex, double real or single real"
     output = c_null_ptr
  end select

end function pastixGetCptrFrom2dArray
""",
    'footer'      : """
subroutine pastixOrderGetArray_f08( order, permtab, peritab, rangtab, treetab, sndetab )
  use :: pastixf_interfaces, only : pastixOrderGetArray
  use :: iso_c_binding,      only : c_f_pointer
  use :: pastixf_enums,      only : pastix_order_t, pastix_int_t
  implicit none

  type(pastix_order_t),                intent(in),            target  :: order
  integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: permtab
  integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: peritab
  integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: rangtab
  integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: treetab
  integer(pastix_int_t), dimension(:), intent(out), optional, pointer :: sndetab

  if (present(permtab)) call c_f_pointer( order%permtab, permtab, [order%vertnbr]   )
  if (present(peritab)) call c_f_pointer( order%peritab, peritab, [order%vertnbr]   )
  if (present(rangtab)) call c_f_pointer( order%rangtab, rangtab, [order%cblknbr+1] )
  if (present(treetab)) call c_f_pointer( order%treetab, treetab, [order%cblknbr+1] )
  if (present(sndetab)) call c_f_pointer( order%sndetab, sndetab, [order%sndenbr]   )

end subroutine pastixOrderGetArray_f08
""",
    'enums'       : {}
}

bindings = {
    'filename'    : filename_prefix + 'pastixf_bindings.f90',
    'description' : "PaStiX Fortran to C bindings module",
    'header'      : """
  interface
     function pastixGetCptrFromValue(input) result(output)
       use :: iso_c_binding, only : c_ptr
       implicit none
       class(*),   target :: input
       type(c_ptr)        :: output
     end function pastixGetCptrFromValue

     function pastixGetCptrFrom1dArray(input) result(output)
       use :: iso_c_binding, only : c_ptr
       implicit none
       class(*), dimension(:), target :: input
       type(c_ptr)                      :: output
     end function pastixGetCptrFrom1dArray

     function pastixGetCptrFrom2dArray(input) result(output)
       use :: iso_c_binding, only : c_ptr
       implicit none
       class(*), dimension(:,:), target :: input
       type(c_ptr)                      :: output
     end function pastixGetCptrFrom2dArray
""",
    'footer'      : "  end interface\n",
    'enums'       : {}
}

cbindings = {
    'filename'    : filename_prefix + 'pastix_f2c.c',
    'description' : "PaStiX Fortran to C bindings module",
    'header'      : """
#include "common.h"

static inline PASTIX_Comm
_pastix_comm_f2c( int pastix_comm )
{
    int flag = 0;
    MPI_Initialized(&flag);
    if ( !flag ) {
        return MPI_COMM_WORLD;
    }
    else {
        return MPI_Comm_f2c( pastix_comm );
    }
}
""",
    'footer'      : "",
    'enums'       : {}
}

# set indentation in the f90 file
tab = "  "
indent = "   "

itab=2
iindent=3

# translation_table of types
types_dict = {
    "int":    { 'use' : "iso_c_binding", 'only' : "c_int",    'ftype' : "integer(kind=c_int)"    },
    "int8_t": { 'use' : "iso_c_binding", 'only' : "c_int8_t", 'ftype' : "integer(kind=c_int8_t)" },
    "size_t": { 'use' : "iso_c_binding", 'only' : "c_size_t", 'ftype' : "integer(kind=c_size_t)" },
    "char":   { 'use' : "iso_c_binding", 'only' : "c_char",   'ftype' : "character(kind=c_char)" },
    "double": { 'use' : "iso_c_binding", 'only' : "c_double", 'ftype' : "real(kind=c_double)"    },
    "float":  { 'use' : "iso_c_binding", 'only' : "c_float",  'ftype' : "real(kind=c_float)"     },
    "void":   { 'use' : "iso_c_binding", 'only' : "c_ptr",    'ftype' : "class(*)"               },
    "FILE":   { 'use' : "iso_c_binding", 'only' : "c_ptr",    'ftype' : "type(c_ptr)"            },

    "unsigned long long int": { 'use' : "iso_c_binding", 'only' : "c_long_long",
                                'ftype' : "integer(kind=c_long_long)"  },

    "seed_t": { 'use' : "iso_c_binding", 'only' : "c_long_long",
                'ftype' : "integer(kind=c_long_long)" },

    "spm_coeftype_t":  { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_dir_t":       { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_trans_t":     { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_uplo_t":      { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_diag_t":      { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_side_t":      { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_driver_t":    { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_fmttype_t":   { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_layout_t":    { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_normtype_t":  { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_rhstype_t":   { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_mtxtype_t":   { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "spm_complex64_t": { 'use' : "iso_c_binding", 'only' : "c_double_complex",
                         'ftype' : "complex(kind=c_double_complex)" },
    "spm_complex32_t": { 'use' : "iso_c_binding", 'only' : "c_float_complex",
                         'ftype' : "complex(kind=c_float_complex)"  },

    "spmatrix_t": { 'use' : "spmf_enums", 'only' : "spmatrix_t", 'ftype' : "type(spmatrix_t)"        },
    "spm_int_t":  { 'use' : "spmf_enums", 'only' : "spm_int_t",  'ftype' : "integer(kind=spm_int_t)" },
    "SPM_Comm":   { 'use' : "spmf_enums", 'only' : "MPI_Comm",   'ftype' : "type(MPI_Comm)"          },
    "MPI_Comm":   { 'use' : "spmf_enums", 'only' : "MPI_Comm",   'ftype' : "type(MPI_Comm)"          },

    "pastix_coeftype_t":  { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_dir_t":       { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_trans_t":     { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_uplo_t":      { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_diag_t":      { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_side_t":      { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_fmttype_t":   { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_layout_t":    { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_normtype_t":  { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_rhstype_t":   { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_mtxtype_t":   { 'use' : "iso_c_binding", 'only' : "c_int", 'ftype' : "integer(c_int)" },
    "pastix_complex64_t": { 'use' : "iso_c_binding", 'only' : "c_double_complex",
                            'ftype' : "complex(kind=c_double_complex)" },
    "pastix_complex32_t": { 'use' : "iso_c_binding", 'only' : "c_float_complex",
                            'ftype' : "complex(kind=c_float_complex)"  },

    "pastix_data_t":  { 'use' : "pastixf_enums", 'only' : "pastix_data_t",  'ftype' : "type(pastix_data_t)"        },
    "pastix_rhs_t":   { 'use' : "pastixf_enums", 'only' : "pastix_rhs_t",   'ftype' : "type(pastix_rhs_t)"         },
    "pastix_int_t":   { 'use' : "pastixf_enums", 'only' : "pastix_int_t",   'ftype' : "integer(kind=pastix_int_t)" },
    "pastix_order_t": { 'use' : "pastixf_enums", 'only' : "pastix_order_t", 'ftype' : "type(pastix_order_t)"       },
    "pastix_graph_t": { 'use' : "pastixf_enums", 'only' : "pastix_graph_t", 'ftype' : "type(pastix_graph_t)"       },
    "PASTIX_Comm":    { 'use' : "pastixf_enums", 'only' : "MPI_Comm",       'ftype' : "type(MPI_Comm)"             },
}
