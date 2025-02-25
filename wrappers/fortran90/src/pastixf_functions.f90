!>
!> @file pastixf_functions.f90
!>
!> PaStiX Fortran interface implementation
!>
!> @copyright 2017-2025 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
!>                      Univ. Bordeaux. All rights reserved.
!>
!> @version 6.4.0
!> @author Mathieu Faverge
!> @author Tony Delarue
!> @author Selmane Lebdaoui
!> @date 2025-02-24
!>
!> This file has been automatically generated with gen_wrappers.py
!>
!> @ingroup wrap_fortran
!>
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

subroutine pastixOrderInit_f08(ordeptr, baseval, vertnbr, cblknbr, perm, invp, &
     rang, tree, info)
  use :: pastixf_interfaces, only : pastixOrderInit
  use :: pastixf_bindings,   only : pastixOrderInit_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_int_t, pastix_order_t
  implicit none
  type(pastix_order_t),       intent(inout), target   :: ordeptr
  integer(kind=pastix_int_t), intent(in)              :: baseval
  integer(kind=pastix_int_t), intent(in)              :: vertnbr
  integer(kind=pastix_int_t), intent(in)              :: cblknbr
  integer(kind=pastix_int_t), intent(inout), target   :: perm(:)
  integer(kind=pastix_int_t), intent(inout), target   :: invp(:)
  integer(kind=pastix_int_t), intent(inout), target   :: rang(:)
  integer(kind=pastix_int_t), intent(inout), target   :: tree(:)
  integer(kind=c_int),        intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixOrderInit_f2c(c_loc(ordeptr), baseval, vertnbr, cblknbr, &
       c_loc(perm), c_loc(invp), c_loc(rang), c_loc(tree))
  if ( present(info) ) info = x_info

end subroutine pastixOrderInit_f08

subroutine pastixOrderAlloc_f08(ordeptr, vertnbr, cblknbr, info)
  use :: pastixf_interfaces, only : pastixOrderAlloc
  use :: pastixf_bindings,   only : pastixOrderAlloc_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_int_t, pastix_order_t
  implicit none
  type(pastix_order_t),       intent(inout), target   :: ordeptr
  integer(kind=pastix_int_t), intent(in)              :: vertnbr
  integer(kind=pastix_int_t), intent(in)              :: cblknbr
  integer(kind=c_int),        intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixOrderAlloc_f2c(c_loc(ordeptr), vertnbr, cblknbr)
  if ( present(info) ) info = x_info

end subroutine pastixOrderAlloc_f08

subroutine pastixOrderAllocId_f08(ordeptr, vertnbr, info)
  use :: pastixf_interfaces, only : pastixOrderAllocId
  use :: pastixf_bindings,   only : pastixOrderAllocId_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_int_t, pastix_order_t
  implicit none
  type(pastix_order_t),       intent(inout), target   :: ordeptr
  integer(kind=pastix_int_t), intent(in)              :: vertnbr
  integer(kind=c_int),        intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixOrderAllocId_f2c(c_loc(ordeptr), vertnbr)
  if ( present(info) ) info = x_info

end subroutine pastixOrderAllocId_f08

subroutine pastixOrderExit_f08(ordeptr)
  use :: pastixf_interfaces, only : pastixOrderExit
  use :: pastixf_bindings,   only : pastixOrderExit_f2c
  use :: iso_c_binding,      only : c_loc
  use :: pastixf_enums,      only : pastix_order_t
  implicit none
  type(pastix_order_t), intent(inout), target :: ordeptr

  call pastixOrderExit_f2c(c_loc(ordeptr))
end subroutine pastixOrderExit_f08

subroutine pastixOrderBase_f08(ordeptr, baseval)
  use :: pastixf_interfaces, only : pastixOrderBase
  use :: pastixf_bindings,   only : pastixOrderBase_f2c
  use :: iso_c_binding,      only : c_loc
  use :: pastixf_enums,      only : pastix_int_t, pastix_order_t
  implicit none
  type(pastix_order_t),       intent(inout), target :: ordeptr
  integer(kind=pastix_int_t), intent(in)            :: baseval

  call pastixOrderBase_f2c(c_loc(ordeptr), baseval)
end subroutine pastixOrderBase_f08

subroutine pastixOrderCheck_f08(ordeptr, info)
  use :: pastixf_interfaces, only : pastixOrderCheck
  use :: pastixf_bindings,   only : pastixOrderCheck_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_order_t
  implicit none
  type(pastix_order_t), intent(in),  target   :: ordeptr
  integer(kind=c_int),  intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixOrderCheck_f2c(c_loc(ordeptr))
  if ( present(info) ) info = x_info

end subroutine pastixOrderCheck_f08

subroutine pastixOrderCopy_f08(ordedst, ordesrc, info)
  use :: pastixf_interfaces, only : pastixOrderCopy
  use :: pastixf_bindings,   only : pastixOrderCopy_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_order_t
  implicit none
  type(pastix_order_t), intent(inout), target   :: ordedst
  type(pastix_order_t), intent(in),    target   :: ordesrc
  integer(kind=c_int),  intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixOrderCopy_f2c(c_loc(ordedst), c_loc(ordesrc))
  if ( present(info) ) info = x_info

end subroutine pastixOrderCopy_f08

subroutine pastixOrderGet_f08(pastix_data, order)
  use :: pastixf_interfaces, only : pastixOrderGet
  use :: pastixf_bindings,   only : pastixOrderGet_f2c
  use :: iso_c_binding,      only : c_f_pointer, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_order_t
  implicit none
  type(pastix_data_t),  intent(in),  target  :: pastix_data
  type(pastix_order_t), intent(out), pointer :: order

  call c_f_pointer(pastixOrderGet_f2c(c_loc(pastix_data)), order)
end subroutine pastixOrderGet_f08

subroutine pastixOrderBcast_f08(ordemesh, root, pastix_comm)
  use :: pastixf_interfaces, only : pastixOrderBcast
  use :: pastixf_bindings,   only : pastixOrderBcast_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_order_t
  use :: spmf_enums,         only : MPI_Comm
  implicit none
  type(pastix_order_t), intent(inout), target :: ordemesh
  integer(kind=c_int),  intent(in)            :: root
  type(MPI_Comm),       intent(in)            :: pastix_comm

  call pastixOrderBcast_f2c(c_loc(ordemesh), root, pastix_comm%MPI_VAL)
end subroutine pastixOrderBcast_f08

subroutine pastixOrderGrid_f08(myorder, nx, ny, nz, info)
  use :: pastixf_interfaces, only : pastixOrderGrid
  use :: pastixf_bindings,   only : pastixOrderGrid_f2c
  use :: iso_c_binding,      only : c_f_pointer, c_int, c_loc, c_ptr
  use :: pastixf_enums,      only : pastix_int_t, pastix_order_t
  implicit none
  type(pastix_order_t),       intent(inout), pointer  :: myorder
  integer(kind=pastix_int_t), intent(in)              :: nx
  integer(kind=pastix_int_t), intent(in)              :: ny
  integer(kind=pastix_int_t), intent(in)              :: nz
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_myorder
  integer(kind=c_int) :: x_info

  x_myorder = c_loc(myorder)

  x_info = pastixOrderGrid_f2c(x_myorder, nx, ny, nz)
  call c_f_pointer(x_myorder, myorder)
  if ( present(info) ) info = x_info

end subroutine pastixOrderGrid_f08

subroutine pastixOrderLoad_f08(pastix_data, ordeptr, info)
  use :: pastixf_interfaces, only : pastixOrderLoad
  use :: pastixf_bindings,   only : pastixOrderLoad_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_order_t
  implicit none
  type(pastix_data_t),  intent(in),    target   :: pastix_data
  type(pastix_order_t), intent(inout), target   :: ordeptr
  integer(kind=c_int),  intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixOrderLoad_f2c(c_loc(pastix_data), c_loc(ordeptr))
  if ( present(info) ) info = x_info

end subroutine pastixOrderLoad_f08

subroutine pastixOrderSave_f08(pastix_data, ordeptr, info)
  use :: pastixf_interfaces, only : pastixOrderSave
  use :: pastixf_bindings,   only : pastixOrderSave_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_order_t
  implicit none
  type(pastix_data_t),  intent(inout), target   :: pastix_data
  type(pastix_order_t), intent(in),    target   :: ordeptr
  integer(kind=c_int),  intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixOrderSave_f2c(c_loc(pastix_data), c_loc(ordeptr))
  if ( present(info) ) info = x_info

end subroutine pastixOrderSave_f08

subroutine pastix_f08(pastix_data, pastix_comm, n, colptr, rowptr, values, &
     perm, invp, B, nrhs, iparm, dparm, info)
  use :: pastixf_interfaces, only : pastix
  use :: pastixf_bindings,   only : pastix_f2c
  use :: iso_c_binding,      only : c_double, c_f_pointer, c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom1dArray, pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  use :: spmf_enums,         only : MPI_Comm
  implicit none
  type(pastix_data_t),        intent(inout), pointer  :: pastix_data
  type(MPI_Comm),             intent(in)              :: pastix_comm
  integer(kind=pastix_int_t), intent(in)              :: n
  integer(kind=pastix_int_t), intent(inout), target   :: colptr(:)
  integer(kind=pastix_int_t), intent(inout), target   :: rowptr(:)
  class(*),                   intent(inout), target   :: values(:)
  integer(kind=pastix_int_t), intent(inout), target   :: perm(:)
  integer(kind=pastix_int_t), intent(inout), target   :: invp(:)
  class(*),                   intent(inout), target   :: B(:,:)
  integer(kind=pastix_int_t), intent(in)              :: nrhs
  integer(kind=pastix_int_t), intent(inout), target   :: iparm(:)
  real(kind=c_double),        intent(inout), target   :: dparm(:)
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_pastix_data
  type(c_ptr)         :: x_values
  type(c_ptr)         :: x_B
  integer(kind=c_int) :: x_info

  x_pastix_data = c_loc(pastix_data)
  x_values      = pastixGetCptrFrom1dArray(values)
  x_B           = pastixGetCptrFrom2dArray(B)

  x_info = pastix_f2c(x_pastix_data, pastix_comm%MPI_VAL, n, c_loc(colptr), &
       c_loc(rowptr), x_values, c_loc(perm), c_loc(invp), x_B, nrhs, &
       c_loc(iparm), c_loc(dparm))
  call c_f_pointer(x_pastix_data, pastix_data)
  if ( present(info) ) info = x_info

end subroutine pastix_f08

subroutine pastixInitParam_f08(iparm, dparm)
  use :: pastixf_interfaces, only : pastixInitParam
  use :: pastixf_bindings,   only : pastixInitParam_f2c
  use :: iso_c_binding,      only : c_double, c_loc
  use :: pastixf_enums,      only : pastix_int_t
  implicit none
  integer(kind=pastix_int_t), intent(inout), target :: iparm(:)
  real(kind=c_double),        intent(inout), target :: dparm(:)

  call pastixInitParam_f2c(c_loc(iparm), c_loc(dparm))
end subroutine pastixInitParam_f08

subroutine pastixInit_f08(pastix_data, pastix_comm, iparm, dparm)
  use :: pastixf_interfaces, only : pastixInit
  use :: pastixf_bindings,   only : pastixInit_f2c
  use :: iso_c_binding,      only : c_double, c_f_pointer, c_loc, c_ptr
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  use :: spmf_enums,         only : MPI_Comm
  implicit none
  type(pastix_data_t),        intent(inout), pointer :: pastix_data
  type(MPI_Comm),             intent(in)             :: pastix_comm
  integer(kind=pastix_int_t), intent(inout), target  :: iparm(:)
  real(kind=c_double),        intent(inout), target  :: dparm(:)

  type(c_ptr) :: x_pastix_data

  x_pastix_data = c_loc(pastix_data)

  call pastixInit_f2c(x_pastix_data, pastix_comm%MPI_VAL, c_loc(iparm), &
       c_loc(dparm))
  call c_f_pointer(x_pastix_data, pastix_data)

end subroutine pastixInit_f08

subroutine pastixInitWithAffinity_f08(pastix_data, pastix_comm, iparm, dparm, &
     bindtab)
  use :: pastixf_interfaces, only : pastixInitWithAffinity
  use :: pastixf_bindings,   only : pastixInitWithAffinity_f2c
  use :: iso_c_binding,      only : c_double, c_f_pointer, c_int, c_loc, c_ptr
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  use :: spmf_enums,         only : MPI_Comm
  implicit none
  type(pastix_data_t),        intent(inout), pointer :: pastix_data
  type(MPI_Comm),             intent(in)             :: pastix_comm
  integer(kind=pastix_int_t), intent(inout), target  :: iparm(:)
  real(kind=c_double),        intent(inout), target  :: dparm(:)
  integer(kind=c_int),        intent(in),    target  :: bindtab(:)

  type(c_ptr) :: x_pastix_data

  x_pastix_data = c_loc(pastix_data)

  call pastixInitWithAffinity_f2c(x_pastix_data, pastix_comm%MPI_VAL, &
       c_loc(iparm), c_loc(dparm), c_loc(bindtab))
  call c_f_pointer(x_pastix_data, pastix_data)

end subroutine pastixInitWithAffinity_f08

subroutine pastixFinalize_f08(pastix_data)
  use :: pastixf_interfaces, only : pastixFinalize
  use :: pastixf_bindings,   only : pastixFinalize_f2c
  use :: iso_c_binding,      only : c_f_pointer, c_loc, c_ptr
  use :: pastixf_enums,      only : pastix_data_t
  implicit none
  type(pastix_data_t), intent(inout), pointer :: pastix_data

  type(c_ptr) :: x_pastix_data

  x_pastix_data = c_loc(pastix_data)

  call pastixFinalize_f2c(x_pastix_data)
  call c_f_pointer(x_pastix_data, pastix_data)

end subroutine pastixFinalize_f08

subroutine pastix_task_analyze_f08(pastix_data, spm, info)
  use :: pastixf_interfaces, only : pastix_task_analyze
  use :: pastixf_bindings,   only : pastix_task_analyze_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  use :: spmf_enums,         only : spmatrix_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  type(spmatrix_t),    intent(in),    target   :: spm
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_task_analyze_f2c(c_loc(pastix_data), c_loc(spm))
  if ( present(info) ) info = x_info

end subroutine pastix_task_analyze_f08

subroutine pastix_task_numfact_f08(pastix_data, spm, info)
  use :: pastixf_interfaces, only : pastix_task_numfact
  use :: pastixf_bindings,   only : pastix_task_numfact_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  use :: spmf_enums,         only : spmatrix_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  type(spmatrix_t),    intent(inout), target   :: spm
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_task_numfact_f2c(c_loc(pastix_data), c_loc(spm))
  if ( present(info) ) info = x_info

end subroutine pastix_task_numfact_f08

subroutine pastix_task_solve_f08(pastix_data, m, nrhs, B, ldb, info)
  use :: pastixf_interfaces, only : pastix_task_solve
  use :: pastixf_bindings,   only : pastix_task_solve_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  implicit none
  type(pastix_data_t),        intent(inout), target   :: pastix_data
  integer(kind=pastix_int_t), intent(in)              :: m
  integer(kind=pastix_int_t), intent(in)              :: nrhs
  class(*),                   intent(inout), target   :: B(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldb
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_B
  integer(kind=c_int) :: x_info

  x_B = pastixGetCptrFrom2dArray(B)

  x_info = pastix_task_solve_f2c(c_loc(pastix_data), m, nrhs, x_B, ldb)
  if ( present(info) ) info = x_info

end subroutine pastix_task_solve_f08

subroutine pastix_task_refine_f08(pastix_data, n, nrhs, B, ldb, X, ldx, info)
  use :: pastixf_interfaces, only : pastix_task_refine
  use :: pastixf_bindings,   only : pastix_task_refine_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  implicit none
  type(pastix_data_t),        intent(inout), target   :: pastix_data
  integer(kind=pastix_int_t), intent(in)              :: n
  integer(kind=pastix_int_t), intent(in)              :: nrhs
  class(*),                   intent(inout), target   :: B(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldb
  class(*),                   intent(inout), target   :: X(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldx
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_B
  type(c_ptr)         :: x_X
  integer(kind=c_int) :: x_info

  x_B = pastixGetCptrFrom2dArray(B)
  x_X = pastixGetCptrFrom2dArray(X)

  x_info = pastix_task_refine_f2c(c_loc(pastix_data), n, nrhs, x_B, ldb, x_X, &
       ldx)
  if ( present(info) ) info = x_info

end subroutine pastix_task_refine_f08

subroutine pastix_task_solve_and_refine_f08(pastix_data, n, nrhs, B, ldb, X, &
     ldx, info)
  use :: pastixf_interfaces, only : pastix_task_solve_and_refine
  use :: pastixf_bindings,   only : pastix_task_solve_and_refine_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  implicit none
  type(pastix_data_t),        intent(inout), target   :: pastix_data
  integer(kind=pastix_int_t), intent(in)              :: n
  integer(kind=pastix_int_t), intent(in)              :: nrhs
  class(*),                   intent(inout), target   :: B(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldb
  class(*),                   intent(inout), target   :: X(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldx
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_B
  type(c_ptr)         :: x_X
  integer(kind=c_int) :: x_info

  x_B = pastixGetCptrFrom2dArray(B)
  x_X = pastixGetCptrFrom2dArray(X)

  x_info = pastix_task_solve_and_refine_f2c(c_loc(pastix_data), n, nrhs, x_B, &
       ldb, x_X, ldx)
  if ( present(info) ) info = x_info

end subroutine pastix_task_solve_and_refine_f08

subroutine pastix_subtask_order_f08(pastix_data, spm, myorder, info)
  use :: pastixf_interfaces, only : pastix_subtask_order
  use :: pastixf_bindings,   only : pastix_subtask_order_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_order_t
  use :: spmf_enums,         only : spmatrix_t
  implicit none
  type(pastix_data_t),  intent(inout), target   :: pastix_data
  type(spmatrix_t),     intent(in),    target   :: spm
  type(pastix_order_t), intent(inout), target   :: myorder
  integer(kind=c_int),  intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_order_f2c(c_loc(pastix_data), c_loc(spm), &
       c_loc(myorder))
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_order_f08

subroutine pastix_subtask_symbfact_f08(pastix_data, info)
  use :: pastixf_interfaces, only : pastix_subtask_symbfact
  use :: pastixf_bindings,   only : pastix_subtask_symbfact_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_symbfact_f2c(c_loc(pastix_data))
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_symbfact_f08

subroutine pastix_subtask_reordering_f08(pastix_data, info)
  use :: pastixf_interfaces, only : pastix_subtask_reordering
  use :: pastixf_bindings,   only : pastix_subtask_reordering_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_reordering_f2c(c_loc(pastix_data))
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_reordering_f08

subroutine pastix_subtask_blend_f08(pastix_data, info)
  use :: pastixf_interfaces, only : pastix_subtask_blend
  use :: pastixf_bindings,   only : pastix_subtask_blend_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_blend_f2c(c_loc(pastix_data))
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_blend_f08

subroutine pastix_subtask_spm2bcsc_f08(pastix_data, spm, info)
  use :: pastixf_interfaces, only : pastix_subtask_spm2bcsc
  use :: pastixf_bindings,   only : pastix_subtask_spm2bcsc_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  use :: spmf_enums,         only : spmatrix_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  type(spmatrix_t),    intent(inout), target   :: spm
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_spm2bcsc_f2c(c_loc(pastix_data), c_loc(spm))
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_spm2bcsc_f08

subroutine pastix_subtask_bcsc2ctab_f08(pastix_data, info)
  use :: pastixf_interfaces, only : pastix_subtask_bcsc2ctab
  use :: pastixf_bindings,   only : pastix_subtask_bcsc2ctab_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_bcsc2ctab_f2c(c_loc(pastix_data))
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_bcsc2ctab_f08

subroutine pastix_subtask_sopalin_f08(pastix_data, info)
  use :: pastixf_interfaces, only : pastix_subtask_sopalin
  use :: pastixf_bindings,   only : pastix_subtask_sopalin_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_sopalin_f2c(c_loc(pastix_data))
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_sopalin_f08

subroutine pastix_subtask_applyorder_f08(pastix_data, dir, m, n, B, ldb, Bp, &
     info)
  use :: pastixf_interfaces, only : pastix_subtask_applyorder
  use :: pastixf_bindings,   only : pastix_subtask_applyorder_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t, pastix_rhs_t
  implicit none
  type(pastix_data_t),        intent(inout), target   :: pastix_data
  integer(c_int),             intent(in)              :: dir
  integer(kind=pastix_int_t), intent(in)              :: m
  integer(kind=pastix_int_t), intent(in)              :: n
  class(*),                   intent(inout), target   :: B(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldb
  type(pastix_rhs_t),         intent(in)              :: Bp
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_B
  integer(kind=c_int) :: x_info

  x_B = pastixGetCptrFrom2dArray(B)

  x_info = pastix_subtask_applyorder_f2c(c_loc(pastix_data), dir, m, n, x_B, &
       ldb, Bp)
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_applyorder_f08

subroutine pastix_subtask_trsm_f08(pastix_data, side, uplo, trans, diag, b, &
     info)
  use :: pastixf_interfaces, only : pastix_subtask_trsm
  use :: pastixf_bindings,   only : pastix_subtask_trsm_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_rhs_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  integer(c_int),      intent(in)              :: side
  integer(c_int),      intent(in)              :: uplo
  integer(c_int),      intent(in)              :: trans
  integer(c_int),      intent(in)              :: diag
  type(pastix_rhs_t),  intent(in)              :: b
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_trsm_f2c(c_loc(pastix_data), side, uplo, trans, &
       diag, b)
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_trsm_f08

subroutine pastix_subtask_diag_f08(pastix_data, b, info)
  use :: pastixf_interfaces, only : pastix_subtask_diag
  use :: pastixf_bindings,   only : pastix_subtask_diag_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_rhs_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  type(pastix_rhs_t),  intent(in)              :: b
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_diag_f2c(c_loc(pastix_data), b)
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_diag_f08

subroutine pastix_subtask_solve_f08(pastix_data, b, info)
  use :: pastixf_interfaces, only : pastix_subtask_solve
  use :: pastixf_bindings,   only : pastix_subtask_solve_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_rhs_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  type(pastix_rhs_t),  intent(in)              :: b
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_solve_f2c(c_loc(pastix_data), b)
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_solve_f08

subroutine pastix_subtask_refine_f08(pastix_data, b, x, info)
  use :: pastixf_interfaces, only : pastix_subtask_refine
  use :: pastixf_bindings,   only : pastix_subtask_refine_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_rhs_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  type(pastix_rhs_t),  intent(in)              :: b
  type(pastix_rhs_t),  intent(in)              :: x
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_refine_f2c(c_loc(pastix_data), b, x)
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_refine_f08

subroutine pastix_subtask_solve_adv_f08(pastix_data, transA, b, info)
  use :: pastixf_interfaces, only : pastix_subtask_solve_adv
  use :: pastixf_bindings,   only : pastix_subtask_solve_adv_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_rhs_t
  implicit none
  type(pastix_data_t), intent(inout), target   :: pastix_data
  integer(c_int),      intent(in)              :: transA
  type(pastix_rhs_t),  intent(in)              :: b
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastix_subtask_solve_adv_f2c(c_loc(pastix_data), transA, b)
  if ( present(info) ) info = x_info

end subroutine pastix_subtask_solve_adv_f08

subroutine pastixIsolateUnknowns_f08(pastix_data, n, list)
  use :: pastixf_interfaces, only : pastixIsolateUnknowns
  use :: pastixf_bindings,   only : pastixIsolateUnknowns_f2c
  use :: iso_c_binding,      only : c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  implicit none
  type(pastix_data_t),        intent(inout), target :: pastix_data
  integer(kind=pastix_int_t), intent(in)            :: n
  integer(kind=pastix_int_t), intent(in),    target :: list(:)

  call pastixIsolateUnknowns_f2c(c_loc(pastix_data), n, c_loc(list))
end subroutine pastixIsolateUnknowns_f08

subroutine pastixSetSchurUnknownList_f08(pastix_data, n, list)
  use :: pastixf_interfaces, only : pastixSetSchurUnknownList
  use :: pastixf_bindings,   only : pastixSetSchurUnknownList_f2c
  use :: iso_c_binding,      only : c_loc
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  implicit none
  type(pastix_data_t),        intent(inout), target :: pastix_data
  integer(kind=pastix_int_t), intent(in)            :: n
  integer(kind=pastix_int_t), intent(in),    target :: list(:)

  call pastixSetSchurUnknownList_f2c(c_loc(pastix_data), n, c_loc(list))
end subroutine pastixSetSchurUnknownList_f08

subroutine pastixGetSchur_f08(pastix_data, S, lds, info)
  use :: pastixf_interfaces, only : pastixGetSchur
  use :: pastixf_bindings,   only : pastixGetSchur_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  implicit none
  type(pastix_data_t),        intent(in),    target   :: pastix_data
  class(*),                   intent(inout), target   :: S(:,:)
  integer(kind=pastix_int_t), intent(in)              :: lds
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_S
  integer(kind=c_int) :: x_info

  x_S = pastixGetCptrFrom2dArray(S)

  x_info = pastixGetSchur_f2c(c_loc(pastix_data), x_S, lds)
  if ( present(info) ) info = x_info

end subroutine pastixGetSchur_f08

subroutine pastixRhsInit_f08(rhs, info)
  use :: pastixf_interfaces, only : pastixRhsInit
  use :: pastixf_bindings,   only : pastixRhsInit_f2c
  use :: iso_c_binding,      only : c_int, c_loc
  use :: pastixf_enums,      only : pastix_rhs_t
  implicit none
  type(pastix_rhs_t),  intent(inout), target   :: rhs
  integer(kind=c_int), intent(out),   optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixRhsInit_f2c(c_loc(rhs))
  if ( present(info) ) info = x_info

end subroutine pastixRhsInit_f08

subroutine pastixRhsFinalize_f08(rhs, info)
  use :: pastixf_interfaces, only : pastixRhsFinalize
  use :: pastixf_bindings,   only : pastixRhsFinalize_f2c
  use :: iso_c_binding,      only : c_int
  use :: pastixf_enums,      only : pastix_rhs_t
  implicit none
  type(pastix_rhs_t),  intent(in)            :: rhs
  integer(kind=c_int), intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixRhsFinalize_f2c(rhs)
  if ( present(info) ) info = x_info

end subroutine pastixRhsFinalize_f08

subroutine pastixRhsDoubletoSingle_f08(dB, sB, info)
  use :: pastixf_interfaces, only : pastixRhsDoubletoSingle
  use :: pastixf_bindings,   only : pastixRhsDoubletoSingle_f2c
  use :: iso_c_binding,      only : c_int
  use :: pastixf_enums,      only : pastix_rhs_t
  implicit none
  type(pastix_rhs_t),  intent(in)            :: dB
  type(pastix_rhs_t),  intent(in)            :: sB
  integer(kind=c_int), intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixRhsDoubletoSingle_f2c(dB, sB)
  if ( present(info) ) info = x_info

end subroutine pastixRhsDoubletoSingle_f08

subroutine pastixRhsSingleToDouble_f08(sB, dB, info)
  use :: pastixf_interfaces, only : pastixRhsSingleToDouble
  use :: pastixf_bindings,   only : pastixRhsSingleToDouble_f2c
  use :: iso_c_binding,      only : c_int
  use :: pastixf_enums,      only : pastix_rhs_t
  implicit none
  type(pastix_rhs_t),  intent(in)            :: sB
  type(pastix_rhs_t),  intent(in)            :: dB
  integer(kind=c_int), intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixRhsSingleToDouble_f2c(sB, dB)
  if ( present(info) ) info = x_info

end subroutine pastixRhsSingleToDouble_f08

subroutine pastixRhsSchurGet_f08(pastix_data, m, n, rhsB, B, ldb, info)
  use :: pastixf_interfaces, only : pastixRhsSchurGet
  use :: pastixf_bindings,   only : pastixRhsSchurGet_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t, pastix_rhs_t
  implicit none
  type(pastix_data_t),        intent(in),    target   :: pastix_data
  integer(kind=pastix_int_t), intent(in)              :: m
  integer(kind=pastix_int_t), intent(in)              :: n
  type(pastix_rhs_t),         intent(in)              :: rhsB
  class(*),                   intent(inout), target   :: B(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldb
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_B
  integer(kind=c_int) :: x_info

  x_B = pastixGetCptrFrom2dArray(B)

  x_info = pastixRhsSchurGet_f2c(c_loc(pastix_data), m, n, rhsB, x_B, ldb)
  if ( present(info) ) info = x_info

end subroutine pastixRhsSchurGet_f08

subroutine pastixRhsSchurSet_f08(pastix_data, m, n, B, ldb, rhsB, info)
  use :: pastixf_interfaces, only : pastixRhsSchurSet
  use :: pastixf_bindings,   only : pastixRhsSchurSet_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom2dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t, pastix_rhs_t
  implicit none
  type(pastix_data_t),        intent(in),    target   :: pastix_data
  integer(kind=pastix_int_t), intent(in)              :: m
  integer(kind=pastix_int_t), intent(in)              :: n
  class(*),                   intent(inout), target   :: B(:,:)
  integer(kind=pastix_int_t), intent(in)              :: ldb
  type(pastix_rhs_t),         intent(in)              :: rhsB
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_B
  integer(kind=c_int) :: x_info

  x_B = pastixGetCptrFrom2dArray(B)

  x_info = pastixRhsSchurSet_f2c(c_loc(pastix_data), m, n, x_B, ldb, rhsB)
  if ( present(info) ) info = x_info

end subroutine pastixRhsSchurSet_f08

subroutine pastixExpand_f08(pastix_data, spm)
  use :: pastixf_interfaces, only : pastixExpand
  use :: pastixf_bindings,   only : pastixExpand_f2c
  use :: iso_c_binding,      only : c_loc
  use :: pastixf_enums,      only : pastix_data_t
  use :: spmf_enums,         only : spmatrix_t
  implicit none
  type(pastix_data_t), intent(in),    target :: pastix_data
  type(spmatrix_t),    intent(inout), target :: spm

  call pastixExpand_f2c(c_loc(pastix_data), c_loc(spm))
end subroutine pastixExpand_f08

subroutine pastixGetDiag_f08(pastix_data, x, incx, info)
  use :: pastixf_interfaces, only : pastixGetDiag
  use :: pastixf_bindings,   only : pastixGetDiag_f2c
  use :: iso_c_binding,      only : c_int, c_loc, c_ptr
  use :: pastixf_bindings,   only : pastixGetCptrFrom1dArray
  use :: pastixf_enums,      only : pastix_data_t, pastix_int_t
  implicit none
  type(pastix_data_t),        intent(in),    target   :: pastix_data
  class(*),                   intent(inout), target   :: x(:)
  integer(kind=pastix_int_t), intent(in)              :: incx
  integer(kind=c_int),        intent(out),   optional :: info

  type(c_ptr)         :: x_x
  integer(kind=c_int) :: x_info

  x_x = pastixGetCptrFrom1dArray(x)

  x_info = pastixGetDiag_f2c(c_loc(pastix_data), x_x, incx)
  if ( present(info) ) info = x_info

end subroutine pastixGetDiag_f08

subroutine pastixGetOptions_f08(argc, argv, iparm, dparm, check, scatter, &
     driver, filename)
  use :: pastixf_interfaces, only : pastixGetOptions
  use :: pastixf_bindings,   only : pastixGetOptions_f2c
  use :: iso_c_binding,      only : c_char, c_double, c_f_pointer, c_int, c_loc, c_ptr
  use :: pastixf_enums,      only : pastix_int_t
  implicit none
  integer(kind=c_int),        intent(in)             :: argc
  character(kind=c_char),     intent(inout), pointer :: argv
  integer(kind=pastix_int_t), intent(inout), target  :: iparm(:)
  real(kind=c_double),        intent(inout), target  :: dparm(:)
  integer(kind=c_int),        intent(inout), target  :: check
  integer(kind=c_int),        intent(inout), target  :: scatter
  integer(c_int),             intent(inout), target  :: driver
  character(kind=c_char),     intent(inout), pointer :: filename

  type(c_ptr) :: x_argv
  type(c_ptr) :: x_filename

  x_argv     = c_loc(argv)
  x_filename = c_loc(filename)

  call pastixGetOptions_f2c(argc, x_argv, c_loc(iparm), c_loc(dparm), &
       c_loc(check), c_loc(scatter), c_loc(driver), x_filename)
  call c_f_pointer(x_argv, argv)
  call c_f_pointer(x_filename, filename)

end subroutine pastixGetOptions_f08

subroutine pastixDumpParam_f08(pastix_data)
  use :: pastixf_interfaces, only : pastixDumpParam
  use :: pastixf_bindings,   only : pastixDumpParam_f2c
  use :: iso_c_binding,      only : c_loc
  use :: pastixf_enums,      only : pastix_data_t
  implicit none
  type(pastix_data_t), intent(in), target :: pastix_data

  call pastixDumpParam_f2c(c_loc(pastix_data))
end subroutine pastixDumpParam_f08

subroutine pastixCheckParam_f08(iparm, dparm, info)
  use :: pastixf_interfaces, only : pastixCheckParam
  use :: pastixf_bindings,   only : pastixCheckParam_f2c
  use :: iso_c_binding,      only : c_double, c_int, c_loc
  use :: pastixf_enums,      only : pastix_int_t
  implicit none
  integer(kind=pastix_int_t), intent(in),  target   :: iparm(:)
  real(kind=c_double),        intent(in),  target   :: dparm(:)
  integer(kind=c_int),        intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixCheckParam_f2c(c_loc(iparm), c_loc(dparm))
  if ( present(info) ) info = x_info

end subroutine pastixCheckParam_f08

subroutine pastixBlasGetNumThreads_f08(info)
  use :: pastixf_interfaces, only : pastixBlasGetNumThreads
  use :: pastixf_bindings,   only : pastixBlasGetNumThreads_f2c
  use :: iso_c_binding,      only : c_int
  implicit none
  integer(kind=c_int), intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixBlasGetNumThreads_f2c()
  if ( present(info) ) info = x_info

end subroutine pastixBlasGetNumThreads_f08

subroutine pastixBlasSetNumThreads_f08(nt, info)
  use :: pastixf_interfaces, only : pastixBlasSetNumThreads
  use :: pastixf_bindings,   only : pastixBlasSetNumThreads_f2c
  use :: iso_c_binding,      only : c_int
  implicit none
  integer(kind=c_int), intent(in)            :: nt
  integer(kind=c_int), intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixBlasSetNumThreads_f2c(nt)
  if ( present(info) ) info = x_info

end subroutine pastixBlasSetNumThreads_f08

subroutine pastixBlasSetNumThreadsOne_f08(info)
  use :: pastixf_interfaces, only : pastixBlasSetNumThreadsOne
  use :: pastixf_bindings,   only : pastixBlasSetNumThreadsOne_f2c
  use :: iso_c_binding,      only : c_int
  implicit none
  integer(kind=c_int), intent(out), optional :: info

  integer(kind=c_int) :: x_info

  x_info = pastixBlasSetNumThreadsOne_f2c()
  if ( present(info) ) info = x_info

end subroutine pastixBlasSetNumThreadsOne_f08

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
