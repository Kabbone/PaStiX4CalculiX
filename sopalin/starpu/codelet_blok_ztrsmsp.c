/**
 *
 * @file codelet_blok_ztrsmsp.c
 *
 * StarPU codelets for blas-like functions
 *
 * @copyright 2016-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @author Pierre Ramet
 * @author Ian Masliah
 * @author Tom Moenne-Loccoz
 * @date 2024-07-05
 *
 * @precisions normal z -> z c d s
 *
 * @addtogroup pastix_starpu
 * @{
 *
 **/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define _GNU_SOURCE
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
#include "common.h"
#include "blend/solver.h"
#include "sopalin/sopalin_data.h"
#include "pastix_zcores.h"
#if defined(PASTIX_WITH_CUDA)
#include "pastix_zcuda.h"
#endif
#include "pastix_starpu.h"
#include "pastix_zstarpu.h"
#include "codelets.h"

/**
 * @brief Main structure for all tasks of blok_ztrsmsp type
 */
struct cl_blok_ztrsmsp_args_s {
    profile_data_t    profile_data;
    sopalin_data_t   *sopalin_data;
    pastix_side_t     side;
    pastix_uplo_t     uplo;
    pastix_trans_t    trans;
    pastix_diag_t     diag;
    const SolverCblk *cblk;
    pastix_int_t      blok_m;
};

#if defined( PASTIX_STARPU_PROFILING )
/**
 * @brief Functions to profile the codelet
 *
 * Two levels of profiling are available:
 *   1) A generic one that returns the flops per worker
 *   2) A more detailed one that generate logs of the performance for each kernel
 */
starpu_profile_t blok_ztrsmsp_profile = {
    .next = NULL,
    .name = "blok_ztrsmsp"
};

/**
 * @brief Profiling registration function
 */
void blok_ztrsmsp_profile_register( void ) __attribute__( ( constructor ) );
void
blok_ztrsmsp_profile_register( void )
{
    profiling_register_cl( &blok_ztrsmsp_profile );
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(PASTIX_STARPU_PROFILING_LOG)
static void
cl_profiling_cb_blok_ztrsmsp( void *callback_arg )
{
    cl_profiling_callback( callback_arg );

    struct starpu_task                *task = starpu_task_get_current();
    struct starpu_profiling_task_info *info = task->profiling_info;

    /* Quick return */
    if ( info == NULL ) {
        return;
    }

    struct cl_blok_ztrsmsp_args_s *args     = (struct cl_blok_ztrsmsp_args_s *) callback_arg;
    pastix_fixdbl_t                flops    = args->profile_data.flops;
    pastix_fixdbl_t                duration = starpu_timing_timespec_delay_us( &info->start_time, &info->end_time );
    pastix_fixdbl_t                speed    = flops / ( 1000.0 * duration );

    pastix_int_t M = blok_rownbr_ext( args->cblk->fblokptr + args->blok_m );
    pastix_int_t N = cblk_colnbr( args->cblk );

    cl_profiling_log_register( task->name, "blok_ztrsmsp", M, N, 0, flops, speed );
}
#endif

#if defined(PASTIX_STARPU_PROFILING_LOG)
static void (*blok_ztrsmsp_callback)(void*) = cl_profiling_cb_blok_ztrsmsp;
#else
static void (*blok_ztrsmsp_callback)(void*) = cl_profiling_callback;
#endif
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#endif /* defined( PASTIX_STARPU_PROFILING ) */

#if defined(PASTIX_STARPU_COST_PER_ARCH)
/**
 *******************************************************************************
 *
 * @brief Cost model function
 *
 * The user can switch from the pastix static model to an history based model
 * computed automatically.
 *
 *******************************************************************************
 *
 * @param[in] task
 *          TODO
 *
 * @param[in] arch
 *          TODO
 *
 * @param[in] nimpl
 *          TODO
 *
 *******************************************************************************
 *
 * @retval TODO
 *
 *******************************************************************************/
static inline pastix_fixdbl_t
fct_blok_ztrsmsp_cost( struct starpu_task           *task,
                       struct starpu_perfmodel_arch *arch,
                       unsigned                      nimpl )
{
    struct cl_blok_ztrsmsp_args_s *args = (struct cl_blok_ztrsmsp_args_s *)(task->cl_arg);

    pastix_fixdbl_t  cost = 0.;
    pastix_fixdbl_t *coefs;
    pastix_int_t     M = blok_rownbr_ext( args->cblk->fblokptr + args->blok_m );
    pastix_int_t     N = cblk_colnbr( args->cblk );

    switch( arch->devices->type ) {
    case STARPU_CPU_WORKER:
        coefs = &(args->sopalin_data->cpu_models->coefficients[PastixComplex64-2][PastixKernelTRSMBlok2d][0]);
        break;
    case STARPU_CUDA_WORKER:
        coefs = &(args->sopalin_data->gpu_models->coefficients[PastixComplex64-2][PastixKernelTRSMBlok2d][0]);
        break;
    default:
        assert(0);
        return 0.;
    }

    /* Get cost in us */
    cost = modelsGetCost2Param( coefs, M, N ) * 1e6;

    (void)nimpl;
    return cost;
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static struct starpu_perfmodel starpu_blok_ztrsmsp_model = {
#if defined(PASTIX_STARPU_COST_PER_ARCH)
    .type               = STARPU_PER_ARCH,
    .arch_cost_function = fct_blok_ztrsmsp_cost,
#else
    .type               = STARPU_HISTORY_BASED,
#endif
    .symbol             = "blok_ztrsmsp",
};
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#if !defined(PASTIX_STARPU_SIMULATION)
/**
 *******************************************************************************
 *
 * @brief StarPU CPU implementation
 *
 *******************************************************************************
 *
 * @param[in] descr
 *          TODO
 *
 * @param[in] cl_arg
 *          TODO
 *
 *******************************************************************************/
static void
fct_blok_ztrsmsp_cpu( void *descr[], void *cl_arg )
{
    struct cl_blok_ztrsmsp_args_s *args = (struct cl_blok_ztrsmsp_args_s *)cl_arg;
    const void                    *A;
    void                          *C;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    C = pastix_starpu_blok_get_ptr( descr[1] );

    assert( args->cblk->cblktype & CBLK_TASKS_2D );

    args->profile_data.flops = cpublok_ztrsmsp( args->side, args->uplo,
                                                args->trans, args->diag,
                                                args->cblk, args->blok_m, A, C,
                                                &(args->sopalin_data->solvmtx->lowrank) );
}

/**
 * @brief StarPU GPU implementation
 */
#if defined(PASTIX_WITH_CUDA)
static void
fct_blok_ztrsmsp_gpu( void *descr[], void *cl_arg )
{
    struct cl_blok_ztrsmsp_args_s *args = (struct cl_blok_ztrsmsp_args_s *)cl_arg;
    const void                    *A;
    void                          *C;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    C = pastix_starpu_blok_get_ptr( descr[1] );

    assert( args->cblk->cblktype & CBLK_TASKS_2D );

    args->profile_data.flops = gpublok_ztrsmsp( args->side, args->uplo,
                                                args->trans, args->diag,
                                                args->cblk, args->blok_m, A, C,
                                                &(args->sopalin_data->solvmtx->lowrank),
                                                starpu_cuda_get_local_stream() );
}
#endif /* defined(PASTIX_WITH_CUDA) */
#endif /* !defined(PASTIX_STARPU_SIMULATION) */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
CODELETS_GPU( blok_ztrsmsp, 2, STARPU_CUDA_ASYNC );
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 *******************************************************************************
 *
 * @brief TODO
 *
 *******************************************************************************
 *
 * @param[in] sopalin_data
 *          TODO
 *
 * @param[in] coef
 *          TODO
 *
 * @param[in] side
 *          TODO
 *
 * @param[in] uplo
 *          TODO
 *
 * @param[in] trans
 *          TODO
 *
 * @param[in] diag
 *          TODO
 *
 * @param[in] cblk
 *          TODO
 *
 * @param[in] blok
 *          TODO
 *
 * @param[in] prio
 *          TODO
 *
 *******************************************************************************/
void
starpu_task_blok_ztrsmsp( sopalin_data_t   *sopalin_data,
                          pastix_coefside_t coef,
                          pastix_side_t     side,
                          pastix_uplo_t     uplo,
                          pastix_trans_t    trans,
                          pastix_diag_t     diag,
                          const SolverCblk *cblk,
                          SolverBlok       *blok,
                          int               prio )
{
    struct cl_blok_ztrsmsp_args_s *cl_arg        = NULL;
    long long                      execute_where = cl_blok_ztrsmsp_any.where;
    int                            need_exec     = 1;
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
    char                          *task_name;
#endif

    pastix_int_t blok_m = blok - cblk->fblokptr;

    /*
     * Check if it needs to be submitted
     */
#if defined(PASTIX_WITH_MPI)
    {
        int need_submit = 0;
        if ( cblk->ownerid == sopalin_data->solvmtx->clustnum ) {
            need_submit = 1;
        }
        else {
            need_exec = 0;
        }
        if ( starpu_mpi_cached_receive( blok->handler[coef] ) ) {
            need_submit = 1;
        }
        if ( !need_submit ) {
            return;
        }
    }
#endif

    /*
     * Create the arguments array
     */
    if ( need_exec ) {
        cl_arg                        = malloc( sizeof( struct cl_blok_ztrsmsp_args_s ) );
        cl_arg->sopalin_data          = sopalin_data;
#if defined(PASTIX_STARPU_PROFILING)
        cl_arg->profile_data.measures = blok_ztrsmsp_profile.measures;
        cl_arg->profile_data.flops    = NAN;
#endif
        cl_arg->side                  = side;
        cl_arg->uplo                  = uplo;
        cl_arg->trans                 = trans;
        cl_arg->diag                  = diag;
        cl_arg->cblk                  = cblk;
        cl_arg->blok_m                = blok_m;

#if defined(PASTIX_WITH_CUDA)
        if ( (cblk->cblktype & CBLK_COMPRESSED) ) {
            execute_where &= (~STARPU_CUDA);
        }
#endif
    }

#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
    /* This actually generates a memory leak */
    asprintf( &task_name, "%s( %ld, %ld, %ld )",
              cl_blok_ztrsmsp_any.name,
              (long)(cblk - sopalin_data->solvmtx->cblktab),
              (long)(blok - sopalin_data->solvmtx->bloktab),
              (long)coef );
#endif

    pastix_starpu_insert_task(
        &cl_blok_ztrsmsp_any,
        STARPU_CL_ARGS,                 cl_arg,                sizeof( struct cl_blok_ztrsmsp_args_s ),
        STARPU_EXECUTE_WHERE,           execute_where,
#if defined(PASTIX_STARPU_PROFILING)
        STARPU_CALLBACK_WITH_ARG_NFREE, blok_ztrsmsp_callback, cl_arg,
#endif
        STARPU_R,                       cblk->fblokptr->handler[coef],
        STARPU_RW,                      blok->handler[coef],
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
        STARPU_NAME,                    task_name,
#endif
#if defined(PASTIX_STARPU_HETEROPRIO)
        STARPU_PRIORITY,                BucketTRSM2D,
#else
        STARPU_PRIORITY,                prio,
#endif
        0);
    (void)prio;
}

/**
 * @}
 */
