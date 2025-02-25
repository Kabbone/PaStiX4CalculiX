!>
!> @file pastixf_bindings.f90
!>
!> PaStiX Fortran to C bindings module
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
module pastixf_bindings
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

     function pastixOrderInit_f2c(ordeptr, baseval, vertnbr, cblknbr, perm, &
          invp, rang, tree) &
          bind(c, name='pastixOrderInit_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastixOrderInit_f2c
       type(c_ptr),                value :: ordeptr
       integer(kind=pastix_int_t), value :: baseval
       integer(kind=pastix_int_t), value :: vertnbr
       integer(kind=pastix_int_t), value :: cblknbr
       type(c_ptr),                value :: perm
       type(c_ptr),                value :: invp
       type(c_ptr),                value :: rang
       type(c_ptr),                value :: tree
     end function pastixOrderInit_f2c

     function pastixOrderAlloc_f2c(ordeptr, vertnbr, cblknbr) &
          bind(c, name='pastixOrderAlloc_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastixOrderAlloc_f2c
       type(c_ptr),                value :: ordeptr
       integer(kind=pastix_int_t), value :: vertnbr
       integer(kind=pastix_int_t), value :: cblknbr
     end function pastixOrderAlloc_f2c

     function pastixOrderAllocId_f2c(ordeptr, vertnbr) &
          bind(c, name='pastixOrderAllocId_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastixOrderAllocId_f2c
       type(c_ptr),                value :: ordeptr
       integer(kind=pastix_int_t), value :: vertnbr
     end function pastixOrderAllocId_f2c

     subroutine pastixOrderExit_f2c(ordeptr) &
          bind(c, name='pastixOrderExit_f2c')
       use :: iso_c_binding, only : c_ptr
       implicit none
       type(c_ptr), value :: ordeptr
     end subroutine pastixOrderExit_f2c

     subroutine pastixOrderBase_f2c(ordeptr, baseval) &
          bind(c, name='pastixOrderBase_f2c')
       use :: iso_c_binding, only : c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       type(c_ptr),                value :: ordeptr
       integer(kind=pastix_int_t), value :: baseval
     end subroutine pastixOrderBase_f2c

     function pastixOrderCheck_f2c(ordeptr) &
          bind(c, name='pastixOrderCheck_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastixOrderCheck_f2c
       type(c_ptr),  value :: ordeptr
     end function pastixOrderCheck_f2c

     function pastixOrderCopy_f2c(ordedst, ordesrc) &
          bind(c, name='pastixOrderCopy_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastixOrderCopy_f2c
       type(c_ptr),  value :: ordedst
       type(c_ptr),  value :: ordesrc
     end function pastixOrderCopy_f2c

     function pastixOrderGet_f2c(pastix_data) &
          bind(c, name='pastixOrderGet_f2c')
       use :: iso_c_binding, only : c_ptr
       implicit none
       type(c_ptr)        :: pastixOrderGet_f2c
       type(c_ptr), value :: pastix_data
     end function pastixOrderGet_f2c

     subroutine pastixOrderBcast_f2c(ordemesh, root, pastix_comm) &
          bind(c, name='pastixOrderBcast_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       type(c_ptr),         value :: ordemesh
       integer(kind=c_int), value :: root
       integer(kind=c_int), value :: pastix_comm
     end subroutine pastixOrderBcast_f2c

     function pastixOrderGrid_f2c(myorder, nx, ny, nz) &
          bind(c, name='pastixOrderGrid_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastixOrderGrid_f2c
       type(c_ptr)                       :: myorder
       integer(kind=pastix_int_t), value :: nx
       integer(kind=pastix_int_t), value :: ny
       integer(kind=pastix_int_t), value :: nz
     end function pastixOrderGrid_f2c

     function pastixOrderLoad_f2c(pastix_data, ordeptr) &
          bind(c, name='pastixOrderLoad_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastixOrderLoad_f2c
       type(c_ptr),  value :: pastix_data
       type(c_ptr),  value :: ordeptr
     end function pastixOrderLoad_f2c

     function pastixOrderSave_f2c(pastix_data, ordeptr) &
          bind(c, name='pastixOrderSave_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastixOrderSave_f2c
       type(c_ptr),  value :: pastix_data
       type(c_ptr),  value :: ordeptr
     end function pastixOrderSave_f2c

     function pastix_f2c(pastix_data, pastix_comm, n, colptr, rowptr, values, &
          perm, invp, B, nrhs, iparm, dparm) &
          bind(c, name='pastix_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastix_f2c
       type(c_ptr)                       :: pastix_data
       integer(kind=c_int),        value :: pastix_comm
       integer(kind=pastix_int_t), value :: n
       type(c_ptr),                value :: colptr
       type(c_ptr),                value :: rowptr
       type(c_ptr),                value :: values
       type(c_ptr),                value :: perm
       type(c_ptr),                value :: invp
       type(c_ptr),                value :: B
       integer(kind=pastix_int_t), value :: nrhs
       type(c_ptr),                value :: iparm
       type(c_ptr),                value :: dparm
     end function pastix_f2c

     subroutine pastixInitParam_f2c(iparm, dparm) &
          bind(c, name='pastixInitParam_f2c')
       use :: iso_c_binding, only : c_ptr
       implicit none
       type(c_ptr), value :: iparm
       type(c_ptr), value :: dparm
     end subroutine pastixInitParam_f2c

     subroutine pastixInit_f2c(pastix_data, pastix_comm, iparm, dparm) &
          bind(c, name='pastixInit_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       type(c_ptr)                :: pastix_data
       integer(kind=c_int), value :: pastix_comm
       type(c_ptr),         value :: iparm
       type(c_ptr),         value :: dparm
     end subroutine pastixInit_f2c

     subroutine pastixInitWithAffinity_f2c(pastix_data, pastix_comm, iparm, &
          dparm, bindtab) &
          bind(c, name='pastixInitWithAffinity_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       type(c_ptr)                :: pastix_data
       integer(kind=c_int), value :: pastix_comm
       type(c_ptr),         value :: iparm
       type(c_ptr),         value :: dparm
       type(c_ptr),         value :: bindtab
     end subroutine pastixInitWithAffinity_f2c

     subroutine pastixFinalize_f2c(pastix_data) &
          bind(c, name='pastixFinalize_f2c')
       use :: iso_c_binding, only : c_ptr
       implicit none
       type(c_ptr) :: pastix_data
     end subroutine pastixFinalize_f2c

     function pastix_task_analyze_f2c(pastix_data, spm) &
          bind(c, name='pastix_task_analyze_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_task_analyze_f2c
       type(c_ptr),  value :: pastix_data
       type(c_ptr),  value :: spm
     end function pastix_task_analyze_f2c

     function pastix_task_numfact_f2c(pastix_data, spm) &
          bind(c, name='pastix_task_numfact_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_task_numfact_f2c
       type(c_ptr),  value :: pastix_data
       type(c_ptr),  value :: spm
     end function pastix_task_numfact_f2c

     function pastix_task_solve_f2c(pastix_data, m, nrhs, B, ldb) &
          bind(c, name='pastix_task_solve_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastix_task_solve_f2c
       type(c_ptr),                value :: pastix_data
       integer(kind=pastix_int_t), value :: m
       integer(kind=pastix_int_t), value :: nrhs
       type(c_ptr),                value :: B
       integer(kind=pastix_int_t), value :: ldb
     end function pastix_task_solve_f2c

     function pastix_task_refine_f2c(pastix_data, n, nrhs, B, ldb, X, ldx) &
          bind(c, name='pastix_task_refine_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastix_task_refine_f2c
       type(c_ptr),                value :: pastix_data
       integer(kind=pastix_int_t), value :: n
       integer(kind=pastix_int_t), value :: nrhs
       type(c_ptr),                value :: B
       integer(kind=pastix_int_t), value :: ldb
       type(c_ptr),                value :: X
       integer(kind=pastix_int_t), value :: ldx
     end function pastix_task_refine_f2c

     function pastix_task_solve_and_refine_f2c(pastix_data, n, nrhs, B, ldb, &
          X, ldx) &
          bind(c, name='pastix_task_solve_and_refine_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastix_task_solve_and_refine_f2c
       type(c_ptr),                value :: pastix_data
       integer(kind=pastix_int_t), value :: n
       integer(kind=pastix_int_t), value :: nrhs
       type(c_ptr),                value :: B
       integer(kind=pastix_int_t), value :: ldb
       type(c_ptr),                value :: X
       integer(kind=pastix_int_t), value :: ldx
     end function pastix_task_solve_and_refine_f2c

     function pastix_subtask_order_f2c(pastix_data, spm, myorder) &
          bind(c, name='pastix_subtask_order_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_subtask_order_f2c
       type(c_ptr),  value :: pastix_data
       type(c_ptr),  value :: spm
       type(c_ptr),  value :: myorder
     end function pastix_subtask_order_f2c

     function pastix_subtask_symbfact_f2c(pastix_data) &
          bind(c, name='pastix_subtask_symbfact_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_subtask_symbfact_f2c
       type(c_ptr),  value :: pastix_data
     end function pastix_subtask_symbfact_f2c

     function pastix_subtask_reordering_f2c(pastix_data) &
          bind(c, name='pastix_subtask_reordering_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_subtask_reordering_f2c
       type(c_ptr),  value :: pastix_data
     end function pastix_subtask_reordering_f2c

     function pastix_subtask_blend_f2c(pastix_data) &
          bind(c, name='pastix_subtask_blend_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_subtask_blend_f2c
       type(c_ptr),  value :: pastix_data
     end function pastix_subtask_blend_f2c

     function pastix_subtask_spm2bcsc_f2c(pastix_data, spm) &
          bind(c, name='pastix_subtask_spm2bcsc_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_subtask_spm2bcsc_f2c
       type(c_ptr),  value :: pastix_data
       type(c_ptr),  value :: spm
     end function pastix_subtask_spm2bcsc_f2c

     function pastix_subtask_bcsc2ctab_f2c(pastix_data) &
          bind(c, name='pastix_subtask_bcsc2ctab_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_subtask_bcsc2ctab_f2c
       type(c_ptr),  value :: pastix_data
     end function pastix_subtask_bcsc2ctab_f2c

     function pastix_subtask_sopalin_f2c(pastix_data) &
          bind(c, name='pastix_subtask_sopalin_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastix_subtask_sopalin_f2c
       type(c_ptr),  value :: pastix_data
     end function pastix_subtask_sopalin_f2c

     function pastix_subtask_applyorder_f2c(pastix_data, dir, m, n, B, ldb, &
          Bp) &
          bind(c, name='pastix_subtask_applyorder_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t, pastix_rhs_t
       implicit none
       integer(kind=c_int)               :: pastix_subtask_applyorder_f2c
       type(c_ptr),                value :: pastix_data
       integer(c_int),             value :: dir
       integer(kind=pastix_int_t), value :: m
       integer(kind=pastix_int_t), value :: n
       type(c_ptr),                value :: B
       integer(kind=pastix_int_t), value :: ldb
       type(pastix_rhs_t),         value :: Bp
     end function pastix_subtask_applyorder_f2c

     function pastix_subtask_trsm_f2c(pastix_data, side, uplo, trans, diag, b) &
          bind(c, name='pastix_subtask_trsm_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastix_subtask_trsm_f2c
       type(c_ptr),        value :: pastix_data
       integer(c_int),     value :: side
       integer(c_int),     value :: uplo
       integer(c_int),     value :: trans
       integer(c_int),     value :: diag
       type(pastix_rhs_t), value :: b
     end function pastix_subtask_trsm_f2c

     function pastix_subtask_diag_f2c(pastix_data, b) &
          bind(c, name='pastix_subtask_diag_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastix_subtask_diag_f2c
       type(c_ptr),        value :: pastix_data
       type(pastix_rhs_t), value :: b
     end function pastix_subtask_diag_f2c

     function pastix_subtask_solve_f2c(pastix_data, b) &
          bind(c, name='pastix_subtask_solve_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastix_subtask_solve_f2c
       type(c_ptr),        value :: pastix_data
       type(pastix_rhs_t), value :: b
     end function pastix_subtask_solve_f2c

     function pastix_subtask_refine_f2c(pastix_data, b, x) &
          bind(c, name='pastix_subtask_refine_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastix_subtask_refine_f2c
       type(c_ptr),        value :: pastix_data
       type(pastix_rhs_t), value :: b
       type(pastix_rhs_t), value :: x
     end function pastix_subtask_refine_f2c

     function pastix_subtask_solve_adv_f2c(pastix_data, transA, b) &
          bind(c, name='pastix_subtask_solve_adv_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastix_subtask_solve_adv_f2c
       type(c_ptr),        value :: pastix_data
       integer(c_int),     value :: transA
       type(pastix_rhs_t), value :: b
     end function pastix_subtask_solve_adv_f2c

     subroutine pastixIsolateUnknowns_f2c(pastix_data, n, list) &
          bind(c, name='pastixIsolateUnknowns_f2c')
       use :: iso_c_binding, only : c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       type(c_ptr),                value :: pastix_data
       integer(kind=pastix_int_t), value :: n
       type(c_ptr),                value :: list
     end subroutine pastixIsolateUnknowns_f2c

     subroutine pastixSetSchurUnknownList_f2c(pastix_data, n, list) &
          bind(c, name='pastixSetSchurUnknownList_f2c')
       use :: iso_c_binding, only : c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       type(c_ptr),                value :: pastix_data
       integer(kind=pastix_int_t), value :: n
       type(c_ptr),                value :: list
     end subroutine pastixSetSchurUnknownList_f2c

     function pastixGetSchur_f2c(pastix_data, S, lds) &
          bind(c, name='pastixGetSchur_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastixGetSchur_f2c
       type(c_ptr),                value :: pastix_data
       type(c_ptr),                value :: S
       integer(kind=pastix_int_t), value :: lds
     end function pastixGetSchur_f2c

     function pastixRhsInit_f2c(rhs) &
          bind(c, name='pastixRhsInit_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastixRhsInit_f2c
       type(c_ptr),  value :: rhs
     end function pastixRhsInit_f2c

     function pastixRhsFinalize_f2c(rhs) &
          bind(c, name='pastixRhsFinalize_f2c')
       use :: iso_c_binding, only : c_int
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastixRhsFinalize_f2c
       type(pastix_rhs_t), value :: rhs
     end function pastixRhsFinalize_f2c

     function pastixRhsDoubletoSingle_f2c(dB, sB) &
          bind(c, name='pastixRhsDoubletoSingle_f2c')
       use :: iso_c_binding, only : c_int
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastixRhsDoubletoSingle_f2c
       type(pastix_rhs_t), value :: dB
       type(pastix_rhs_t), value :: sB
     end function pastixRhsDoubletoSingle_f2c

     function pastixRhsSingleToDouble_f2c(sB, dB) &
          bind(c, name='pastixRhsSingleToDouble_f2c')
       use :: iso_c_binding, only : c_int
       use :: pastixf_enums, only : pastix_rhs_t
       implicit none
       integer(kind=c_int)       :: pastixRhsSingleToDouble_f2c
       type(pastix_rhs_t), value :: sB
       type(pastix_rhs_t), value :: dB
     end function pastixRhsSingleToDouble_f2c

     function pastixRhsSchurGet_f2c(pastix_data, m, n, rhsB, B, ldb) &
          bind(c, name='pastixRhsSchurGet_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t, pastix_rhs_t
       implicit none
       integer(kind=c_int)               :: pastixRhsSchurGet_f2c
       type(c_ptr),                value :: pastix_data
       integer(kind=pastix_int_t), value :: m
       integer(kind=pastix_int_t), value :: n
       type(pastix_rhs_t),         value :: rhsB
       type(c_ptr),                value :: B
       integer(kind=pastix_int_t), value :: ldb
     end function pastixRhsSchurGet_f2c

     function pastixRhsSchurSet_f2c(pastix_data, m, n, B, ldb, rhsB) &
          bind(c, name='pastixRhsSchurSet_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t, pastix_rhs_t
       implicit none
       integer(kind=c_int)               :: pastixRhsSchurSet_f2c
       type(c_ptr),                value :: pastix_data
       integer(kind=pastix_int_t), value :: m
       integer(kind=pastix_int_t), value :: n
       type(c_ptr),                value :: B
       integer(kind=pastix_int_t), value :: ldb
       type(pastix_rhs_t),         value :: rhsB
     end function pastixRhsSchurSet_f2c

     subroutine pastixExpand_f2c(pastix_data, spm) &
          bind(c, name='pastixExpand_f2c')
       use :: iso_c_binding, only : c_ptr
       implicit none
       type(c_ptr), value :: pastix_data
       type(c_ptr), value :: spm
     end subroutine pastixExpand_f2c

     function pastixGetDiag_f2c(pastix_data, x, incx) &
          bind(c, name='pastixGetDiag_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       use :: pastixf_enums, only : pastix_int_t
       implicit none
       integer(kind=c_int)               :: pastixGetDiag_f2c
       type(c_ptr),                value :: pastix_data
       type(c_ptr),                value :: x
       integer(kind=pastix_int_t), value :: incx
     end function pastixGetDiag_f2c

     subroutine pastixGetOptions_f2c(argc, argv, iparm, dparm, check, scatter, &
          driver, filename) &
          bind(c, name='pastixGetOptions_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int), value :: argc
       type(c_ptr)                :: argv
       type(c_ptr),         value :: iparm
       type(c_ptr),         value :: dparm
       type(c_ptr),         value :: check
       type(c_ptr),         value :: scatter
       type(c_ptr),         value :: driver
       type(c_ptr)                :: filename
     end subroutine pastixGetOptions_f2c

     subroutine pastixDumpParam_f2c(pastix_data) &
          bind(c, name='pastixDumpParam_f2c')
       use :: iso_c_binding, only : c_ptr
       implicit none
       type(c_ptr), value :: pastix_data
     end subroutine pastixDumpParam_f2c

     function pastixCheckParam_f2c(iparm, dparm) &
          bind(c, name='pastixCheckParam_f2c')
       use :: iso_c_binding, only : c_int, c_ptr
       implicit none
       integer(kind=c_int) :: pastixCheckParam_f2c
       type(c_ptr),  value :: iparm
       type(c_ptr),  value :: dparm
     end function pastixCheckParam_f2c

     function pastixBlasGetNumThreads_f2c() &
          bind(c, name='pastixBlasGetNumThreads_f2c')
       use :: iso_c_binding, only : c_int
       implicit none
       integer(kind=c_int) :: pastixBlasGetNumThreads_f2c
     end function pastixBlasGetNumThreads_f2c

     function pastixBlasSetNumThreads_f2c(nt) &
          bind(c, name='pastixBlasSetNumThreads_f2c')
       use :: iso_c_binding, only : c_int
       implicit none
       integer(kind=c_int)        :: pastixBlasSetNumThreads_f2c
       integer(kind=c_int), value :: nt
     end function pastixBlasSetNumThreads_f2c

     function pastixBlasSetNumThreadsOne_f2c() &
          bind(c, name='pastixBlasSetNumThreadsOne_f2c')
       use :: iso_c_binding, only : c_int
       implicit none
       integer(kind=c_int) :: pastixBlasSetNumThreadsOne_f2c
     end function pastixBlasSetNumThreadsOne_f2c
  end interface
end module pastixf_bindings
