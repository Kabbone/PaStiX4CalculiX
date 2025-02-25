/**
 *
 * @file codelet_cblk_zgetrfsp.c
 *
 * StarPU codelets for LU functions
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
#include "pastix_starpu.h"
#include "pastix_zstarpu.h"
#include "codelets.h"

/**
 * @brief Main structure for all tasks of cblk_zgemmsp type
 */
struct cl_cblk_zgetrfsp_args_s {
    profile_data_t  profile_data;
    sopalin_data_t *sopalin_data;
    SolverCblk     *cblk;
};

#if defined(PASTIX_STARPU_PROFILING)
/**
 * @brief Functions to profile the codelet
 *
 * Two levels of profiling are available:
 *   1) A generic one that returns the flops per worker
 *   2) A more detailed one that generate logs of the performance for each kernel
 */
starpu_profile_t cblk_zgetrfsp_profile = {
    .next = NULL,
    .name = "cblk_zgetrfsp"
};

/**
 * @brief Profiling registration function
 */
void cblk_zgetrfsp_profile_register( void ) __attribute__( ( constructor ) );
void
cblk_zgetrfsp_profile_register( void )
{
    profiling_register_cl( &cblk_zgetrfsp_profile );
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(PASTIX_STARPU_PROFILING_LOG)
static void
cl_profiling_cb_cblk_zgetrfsp( void *callback_arg )
{
    cl_profiling_callback( callback_arg );

    struct starpu_task                *task = starpu_task_get_current();
    struct starpu_profiling_task_info *info = task->profiling_info;

    /* Quick return */
    if ( info == NULL ) {
        return;
    }

    struct cl_cblk_zgetrfsp_args_s *args     = (struct cl_cblk_zgetrfsp_args_s *) callback_arg;
    pastix_fixdbl_t                 flops    = args->profile_data.flops;
    pastix_fixdbl_t                 duration = starpu_timing_timespec_delay_us( &info->start_time, &info->end_time );
    pastix_fixdbl_t                 speed    = flops / ( 1000.0 * duration );

    pastix_int_t M = args->cblk->stride;
    pastix_int_t N = cblk_colnbr( args->cblk );
    M -= N;

    cl_profiling_log_register( task->name, "cblk_zgetrfsp", M, N, 0, flops, speed );
}
#endif

#if defined(PASTIX_STARPU_PROFILING_LOG)
static void (*cblk_zgetrfsp_callback)(void*) = cl_profiling_cb_cblk_zgetrfsp;
#else
static void (*cblk_zgetrfsp_callback)(void*) = cl_profiling_callback;
#endif
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#endif /* defined(PASTIX_STARPU_PROFILING) */

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
fct_cblk_zgetrfsp_cost( struct starpu_task           *task,
                        struct starpu_perfmodel_arch *arch,
                        unsigned                      nimpl )
{
    struct cl_cblk_zgetrfsp_args_s *args = (struct cl_cblk_zgetrfsp_args_s *)(task->cl_arg);

    pastix_fixdbl_t  cost = 0.;
    pastix_fixdbl_t *coefs1, *coefs2;
    pastix_int_t     M = args->cblk->stride;
    pastix_int_t     N = cblk_colnbr( args->cblk );
    M -= N;

    switch( arch->devices->type ) {
    case STARPU_CPU_WORKER:
        coefs1 = &(args->sopalin_data->cpu_models->coefficients[PastixComplex64-2][PastixKernelGETRF][0]);
        coefs2 = &(args->sopalin_data->cpu_models->coefficients[PastixComplex64-2][PastixKernelTRSMCblk2d][0]);
        break;
    case STARPU_CUDA_WORKER:
        coefs1 = &(args->sopalin_data->gpu_models->coefficients[PastixComplex64-2][PastixKernelGETRF][0]);
        coefs2 = &(args->sopalin_data->gpu_models->coefficients[PastixComplex64-2][PastixKernelTRSMCblk2d][0]);
        break;
    default:
        assert(0);
        return 0.;
    }

    /* Get cost in us */
    cost  = modelsGetCost1Param( coefs1, N );
    cost += modelsGetCost2Param( coefs2, M, N ) * 2.;

    (void)nimpl;
    return cost;
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static struct starpu_perfmodel starpu_cblk_zgetrfsp_model = {
#if defined( PASTIX_STARPU_COST_PER_ARCH )
    .type               = STARPU_PER_ARCH,
    .arch_cost_function = cblk_getrf_cost,
#else
    .type               = STARPU_HISTORY_BASED,
#endif
    .symbol             = "cblk_zgetrfsp",
};

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
fct_cblk_zgetrfsp_cpu( void *descr[], void *cl_arg )
{
    struct cl_cblk_zgetrfsp_args_s *args = (struct cl_cblk_zgetrfsp_args_s *)cl_arg;
    void                           *L;
    void                           *U;

    L = pastix_starpu_cblk_get_ptr( descr[0] );
    U = pastix_starpu_cblk_get_ptr( descr[1] );

    cpucblk_zgetrfsp1d_panel( args->sopalin_data->solvmtx, args->cblk, L, U );
}
#endif /* !defined(PASTIX_STARPU_SIMULATION) */

CODELETS_CPU( cblk_zgetrfsp, 2 );
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
 * @param[in] cblk
 *          TODO
 *
 * @param[in] prio
 *          TODO
 *
 *******************************************************************************/
void
starpu_task_cblk_zgetrfsp( sopalin_data_t *sopalin_data,
                           SolverCblk     *cblk,
                           int             prio )
{
    struct cl_cblk_zgetrfsp_args_s *cl_arg    = NULL;
    int                             need_exec = 1;
#if defined(PASTIX_DEBUG_STARPU)
    char                           *task_name;
#endif

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
        if ( starpu_mpi_cached_receive( cblk->handler[0] ) ) {
            need_submit = 1;
        }
        if ( starpu_mpi_cached_receive( cblk->handler[1] ) ) {
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
        cl_arg                        = malloc( sizeof( struct cl_cblk_zgetrfsp_args_s) );
        cl_arg->sopalin_data          = sopalin_data;
#if defined(PASTIX_STARPU_PROFILING)
        cl_arg->profile_data.measures = cblk_zgetrfsp_profile.measures;
        cl_arg->profile_data.flops    = NAN;
#endif
        cl_arg->cblk                  = cblk;
    }

#if defined(PASTIX_DEBUG_STARPU)
    /* This actually generates a memory leak */
    asprintf( &task_name, "%s( %ld )",
              cl_cblk_zgetrfsp_cpu.name,
              (long)(cblk - sopalin_data->solvmtx->cblktab) );
#endif

    pastix_starpu_insert_task(
        &cl_cblk_zgetrfsp_cpu,
        STARPU_CL_ARGS,                 cl_arg,                 sizeof( struct cl_cblk_zgetrfsp_args_s ),
#if defined(PASTIX_STARPU_PROFILING)
        STARPU_CALLBACK_WITH_ARG_NFREE, cblk_zgetrfsp_callback, cl_arg,
#endif
        STARPU_RW,                      cblk->handler[0],
        STARPU_RW,                      cblk->handler[1],
#if defined(PASTIX_DEBUG_STARPU)
        STARPU_NAME,                    task_name,
#endif
#if defined(PASTIX_STARPU_HETEROPRIO)
        STARPU_PRIORITY,                BucketFacto1D,
#else
        STARPU_PRIORITY,                prio,
#endif
        0);
    (void)prio;
}

/**
 * @}
 */
