!>
!> @file pastixf_enums.F90
!>
!> PaStiX fortran 90 wrapper to define enums and datatypes
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
module pastixf_enums

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

  ! enum iparm
  enum, bind(C)
     enumerator :: IPARM_VERBOSE                        = 1
     enumerator :: IPARM_IO_STRATEGY                    = 2
     enumerator :: IPARM_NNZEROS                        = 3
     enumerator :: IPARM_NNZEROS_BLOCK_LOCAL            = 4
     enumerator :: IPARM_ALLOCATED_TERMS                = 5
     enumerator :: IPARM_PRODUCE_STATS                  = 6
     enumerator :: IPARM_TRACE                          = 7
     enumerator :: IPARM_MC64                           = 8
     enumerator :: IPARM_ORDERING                       = 9
     enumerator :: IPARM_ORDERING_DEFAULT               = 10
     enumerator :: IPARM_SCOTCH_MT                      = 11
     enumerator :: IPARM_SCOTCH_SWITCH_LEVEL            = 12
     enumerator :: IPARM_SCOTCH_CMIN                    = 13
     enumerator :: IPARM_SCOTCH_CMAX                    = 14
     enumerator :: IPARM_SCOTCH_FRAT                    = 15
     enumerator :: IPARM_METIS_CTYPE                    = 16
     enumerator :: IPARM_METIS_RTYPE                    = 17
     enumerator :: IPARM_METIS_NO2HOP                   = 18
     enumerator :: IPARM_METIS_NSEPS                    = 19
     enumerator :: IPARM_METIS_NITER                    = 20
     enumerator :: IPARM_METIS_UFACTOR                  = 21
     enumerator :: IPARM_METIS_COMPRESS                 = 22
     enumerator :: IPARM_METIS_CCORDER                  = 23
     enumerator :: IPARM_METIS_PFACTOR                  = 24
     enumerator :: IPARM_METIS_SEED                     = 25
     enumerator :: IPARM_METIS_DBGLVL                   = 26
     enumerator :: IPARM_AMALGAMATION_LVLBLAS           = 27
     enumerator :: IPARM_AMALGAMATION_LVLCBLK           = 28
     enumerator :: IPARM_REORDERING_SPLIT               = 29
     enumerator :: IPARM_REORDERING_STOP                = 30
     enumerator :: IPARM_SPLITTING_STRATEGY             = 31
     enumerator :: IPARM_SPLITTING_LEVELS_PROJECTIONS   = 32
     enumerator :: IPARM_SPLITTING_LEVELS_KWAY          = 33
     enumerator :: IPARM_SPLITTING_PROJECTIONS_DEPTH    = 34
     enumerator :: IPARM_SPLITTING_PROJECTIONS_DISTANCE = 35
     enumerator :: IPARM_SPLITTING_PROJECTIONS_WIDTH    = 36
     enumerator :: IPARM_MIN_BLOCKSIZE                  = 37
     enumerator :: IPARM_MAX_BLOCKSIZE                  = 38
     enumerator :: IPARM_TASKS2D_LEVEL                  = 39
     enumerator :: IPARM_TASKS2D_WIDTH                  = 40
     enumerator :: IPARM_ALLCAND                        = 41
     enumerator :: IPARM_INCOMPLETE                     = 42
     enumerator :: IPARM_LEVEL_OF_FILL                  = 43
     enumerator :: IPARM_FACTORIZATION                  = 44
     enumerator :: IPARM_FACTO_LOOK_SIDE                = 45
     enumerator :: IPARM_STATIC_PIVOTING                = 46
     enumerator :: IPARM_FREE_CSCUSER                   = 47
     enumerator :: IPARM_SCHUR_FACT_MODE                = 48
     enumerator :: IPARM_TRANSPOSE_SOLVE                = 49
     enumerator :: IPARM_SCHUR_SOLV_MODE                = 50
     enumerator :: IPARM_APPLYPERM_WS                   = 51
     enumerator :: IPARM_REFINEMENT                     = 52
     enumerator :: IPARM_NBITER                         = 53
     enumerator :: IPARM_ITERMAX                        = 54
     enumerator :: IPARM_GMRES_IM                       = 55
     enumerator :: IPARM_SCHEDULER                      = 56
     enumerator :: IPARM_THREAD_NBR                     = 57
     enumerator :: IPARM_SOCKET_NBR                     = 58
     enumerator :: IPARM_AUTOSPLIT_COMM                 = 59
     enumerator :: IPARM_GPU_NBR                        = 60
     enumerator :: IPARM_GPU_MEMORY_PERCENTAGE          = 61
     enumerator :: IPARM_GPU_MEMORY_BLOCK_SIZE          = 62
     enumerator :: IPARM_GLOBAL_ALLOCATION              = 63
     enumerator :: IPARM_COMPRESS_MIN_WIDTH             = 64
     enumerator :: IPARM_COMPRESS_MIN_HEIGHT            = 65
     enumerator :: IPARM_COMPRESS_WHEN                  = 66
     enumerator :: IPARM_COMPRESS_METHOD                = 67
     enumerator :: IPARM_COMPRESS_ORTHO                 = 68
     enumerator :: IPARM_COMPRESS_RELTOL                = 69
     enumerator :: IPARM_COMPRESS_PRESELECT             = 70
     enumerator :: IPARM_COMPRESS_ILUK                  = 71
     enumerator :: IPARM_MIXED                          = 72
     enumerator :: IPARM_FTZ                            = 73
     enumerator :: IPARM_MPI_THREAD_LEVEL               = 74
     enumerator :: IPARM_MODIFY_PARAMETER               = 75
     enumerator :: IPARM_START_TASK                     = 76
     enumerator :: IPARM_END_TASK                       = 77
     enumerator :: IPARM_FLOAT                          = 78
     enumerator :: IPARM_MTX_TYPE                       = 79
     enumerator :: IPARM_DOF_NBR                        = 80
     enumerator :: IPARM_SIZE                           = 80
  end enum

  ! enum dparm
  enum, bind(C)
     enumerator :: DPARM_FILL_IN            = 1
     enumerator :: DPARM_EPSILON_REFINEMENT = 2
     enumerator :: DPARM_RELATIVE_ERROR     = 3
     enumerator :: DPARM_EPSILON_MAGN_CTRL  = 4
     enumerator :: DPARM_ORDER_TIME         = 5
     enumerator :: DPARM_SYMBFACT_TIME      = 6
     enumerator :: DPARM_REORDER_TIME       = 7
     enumerator :: DPARM_BLEND_TIME         = 8
     enumerator :: DPARM_ANALYZE_TIME       = 9
     enumerator :: DPARM_PRED_FACT_TIME     = 10
     enumerator :: DPARM_FACT_TIME          = 11
     enumerator :: DPARM_FACT_FLOPS         = 12
     enumerator :: DPARM_FACT_THFLOPS       = 13
     enumerator :: DPARM_FACT_RLFLOPS       = 14
     enumerator :: DPARM_FACT_ENERGY        = 15
     enumerator :: DPARM_MEM_FR             = 16
     enumerator :: DPARM_MEM_LR             = 17
     enumerator :: DPARM_SOLV_TIME          = 18
     enumerator :: DPARM_SOLV_FLOPS         = 19
     enumerator :: DPARM_SOLV_THFLOPS       = 20
     enumerator :: DPARM_SOLV_RLFLOPS       = 21
     enumerator :: DPARM_SOLV_ENERGY        = 22
     enumerator :: DPARM_REFINE_TIME        = 23
     enumerator :: DPARM_A_NORM             = 24
     enumerator :: DPARM_COMPRESS_TOLERANCE = 25
     enumerator :: DPARM_COMPRESS_MIN_RATIO = 26
     enumerator :: DPARM_SIZE               = 26
  end enum

  ! enum task
  enum, bind(C)
     enumerator :: PastixTaskInit     = 0
     enumerator :: PastixTaskOrdering = 1
     enumerator :: PastixTaskSymbfact = 2
     enumerator :: PastixTaskAnalyze  = 3
     enumerator :: PastixTaskNumfact  = 4
     enumerator :: PastixTaskSolve    = 5
     enumerator :: PastixTaskRefine   = 6
     enumerator :: PastixTaskClean    = 7
  end enum

  ! enum trace
  enum, bind(C)
     enumerator :: PastixTraceNot          = 0
     enumerator :: PastixTraceNumfact      = 16
     enumerator :: PastixTraceSolve        = 32
     enumerator :: PastixTraceFactAndSolve = 48
  end enum

  ! enum verbose
  enum, bind(C)
     enumerator :: PastixVerboseNot = 0
     enumerator :: PastixVerboseNo  = 1
     enumerator :: PastixVerboseYes = 2
  end enum

  ! enum io
  enum, bind(C)
     enumerator :: PastixIONo        = 0
     enumerator :: PastixIOLoad      = 1
     enumerator :: PastixIOSave      = 2
     enumerator :: PastixIOLoadGraph = 4
     enumerator :: PastixIOSaveGraph = 8
     enumerator :: PastixIOLoadCSC   = 16
     enumerator :: PastixIOSaveCSC   = 32
  end enum

  ! enum fact_mode
  enum, bind(C)
     enumerator :: PastixFactModeLocal = 0
     enumerator :: PastixFactModeSchur = 1
     enumerator :: PastixFactModeBoth  = 2
  end enum

  ! enum solv_mode
  enum, bind(C)
     enumerator :: PastixSolvModeLocal     = 0
     enumerator :: PastixSolvModeInterface = 1
     enumerator :: PastixSolvModeSchur     = 2
  end enum

  ! enum refine
  enum, bind(C)
     enumerator :: PastixRefineGMRES    = 0
     enumerator :: PastixRefineCG       = 1
     enumerator :: PastixRefineSR       = 2
     enumerator :: PastixRefineBiCGSTAB = 3
  end enum

  ! enum factotype
  enum, bind(C)
     enumerator :: PastixFactPOTRF = 0
     enumerator :: PastixFactSYTRF = 1
     enumerator :: PastixFactGETRF = 2
     enumerator :: PastixFactPXTRF = 3
     enumerator :: PastixFactHETRF = 4
     enumerator :: PastixFactLLH   = 0
     enumerator :: PastixFactLDLT  = 1
     enumerator :: PastixFactLU    = 2
     enumerator :: PastixFactLLT   = 3
     enumerator :: PastixFactLDLH  = 4
  end enum

  ! enum factolookside
  enum, bind(C)
     enumerator :: PastixFactLeftLooking  = 0
     enumerator :: PastixFactRightLooking = 1
  end enum

  ! enum scheduler
  enum, bind(C)
     enumerator :: PastixSchedSequential = 0
     enumerator :: PastixSchedStatic     = 1
     enumerator :: PastixSchedParsec     = 2
     enumerator :: PastixSchedStarPU     = 3
     enumerator :: PastixSchedDynamic    = 4
  end enum

  ! enum ordering
  enum, bind(C)
     enumerator :: PastixOrderScotch   = 0
     enumerator :: PastixOrderMetis    = 1
     enumerator :: PastixOrderPersonal = 2
     enumerator :: PastixOrderPtScotch = 3
     enumerator :: PastixOrderParMetis = 4
  end enum

  ! enum mpithreadmode
  enum, bind(C)
     enumerator :: PastixMpiNone             = 0
     enumerator :: PastixMpiThreadSingle     = 1
     enumerator :: PastixMpiThreadFunneled   = 2
     enumerator :: PastixMpiThreadSerialized = 3
     enumerator :: PastixMpiThreadMultiple   = 4
  end enum

  ! enum error
  enum, bind(C)
     enumerator :: PASTIX_SUCCESS            = 0
     enumerator :: PASTIX_ERR_UNKNOWN        = 1
     enumerator :: PASTIX_ERR_ALLOC          = 2
     enumerator :: PASTIX_ERR_NOTIMPLEMENTED = 3
     enumerator :: PASTIX_ERR_OUTOFMEMORY    = 4
     enumerator :: PASTIX_ERR_THREAD         = 5
     enumerator :: PASTIX_ERR_INTERNAL       = 6
     enumerator :: PASTIX_ERR_BADPARAMETER   = 7
     enumerator :: PASTIX_ERR_FILE           = 8
     enumerator :: PASTIX_ERR_INTEGER_TYPE   = 9
     enumerator :: PASTIX_ERR_IO             = 10
     enumerator :: PASTIX_ERR_MPI            = 11
  end enum

  ! enum compress_when
  enum, bind(C)
     enumerator :: PastixCompressNever      = 0
     enumerator :: PastixCompressWhenBegin  = 1
     enumerator :: PastixCompressWhenEnd    = 2
     enumerator :: PastixCompressWhenDuring = 3
  end enum

  ! enum compress_method
  enum, bind(C)
     enumerator :: PastixCompressMethodSVD   = 0
     enumerator :: PastixCompressMethodPQRCP = 1
     enumerator :: PastixCompressMethodRQRCP = 2
     enumerator :: PastixCompressMethodTQRCP = 3
     enumerator :: PastixCompressMethodRQRRT = 4
     enumerator :: PastixCompressMethodNbr   = 5
  end enum

  ! enum compress_ortho
  enum, bind(C)
     enumerator :: PastixCompressOrthoCGS       = 0
     enumerator :: PastixCompressOrthoQR        = 1
     enumerator :: PastixCompressOrthoPartialQR = 2
  end enum

  ! enum split
  enum, bind(C)
     enumerator :: PastixSplitNot             = 0
     enumerator :: PastixSplitKway            = 1
     enumerator :: PastixSplitKwayProjections = 2
  end enum

  ! enum layout
  enum, bind(C)
     enumerator :: PastixRowMajor = 101
     enumerator :: PastixColMajor = 102
  end enum

  ! enum trans
  enum, bind(C)
     enumerator :: PastixNoTrans   = 111
     enumerator :: PastixTrans     = 112
     enumerator :: PastixConjTrans = 113
  end enum

  ! enum uplo
  enum, bind(C)
     enumerator :: PastixUpper      = 121
     enumerator :: PastixLower      = 122
     enumerator :: PastixUpperLower = 123
  end enum

  ! enum coefside
  enum, bind(C)
     enumerator :: PastixLCoef  = 0
     enumerator :: PastixUCoef  = 1
     enumerator :: PastixLUCoef = 2
  end enum

  ! enum diag
  enum, bind(C)
     enumerator :: PastixNonUnit = 131
     enumerator :: PastixUnit    = 132
  end enum

  ! enum side
  enum, bind(C)
     enumerator :: PastixLeft  = 141
     enumerator :: PastixRight = 142
  end enum

  ! enum normtype
  enum, bind(C)
     enumerator :: PastixOneNorm       = 171
     enumerator :: PastixFrobeniusNorm = 174
     enumerator :: PastixInfNorm       = 175
     enumerator :: PastixMaxNorm       = 177
  end enum

  ! enum dir
  enum, bind(C)
     enumerator :: PastixDirForward  = 391
     enumerator :: PastixDirBackward = 392
  end enum

  ! enum mtxtype
  enum, bind(C)
     enumerator :: PastixGeneral   = PastixNoTrans
     enumerator :: PastixSymmetric = PastixTrans
     enumerator :: PastixHermitian = PastixConjTrans
  end enum

  type, bind(c) :: pastix_order_t
     integer(kind=pastix_int_t) :: baseval
     integer(kind=pastix_int_t) :: vertnbr
     integer(kind=pastix_int_t) :: cblknbr
     type(c_ptr)                :: permtab
     type(c_ptr)                :: peritab
     type(c_ptr)                :: rangtab
     type(c_ptr)                :: treetab
     type(c_ptr)                :: selevtx
     integer(kind=pastix_int_t) :: sndenbr
     type(c_ptr)                :: sndetab
     type(c_ptr)                :: peritab_exp
  end type pastix_order_t

contains

  function pastix_getintsize()
    integer :: pastix_getintsize
    pastix_getintsize = kind(PASTIX_INT_KIND)
    return
  end function pastix_getintsize

end module pastixf_enums
