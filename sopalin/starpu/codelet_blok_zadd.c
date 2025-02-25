/**
 *
 * @file codelet_blok_zadd.c
 *
 * StarPU codelet to sum fanin blocks together.
 *
 * @copyright 2016-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Alycia Lisito
 * @author Mathieu Faverge
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
 * @brief Main structure for all tasks of blok_zadd type
 */
struct cl_blok_zadd_args_s {
    profile_data_t     profile_data;
    sopalin_data_t    *sopalin_data;
    pastix_coefside_t  side;
    const SolverCblk  *cblk;
    SolverCblk        *fcblk;
    pastix_int_t       blok_m;
    pastix_int_t       fblok_m;
};

#if defined( PASTIX_STARPU_PROFILING )
/**
 * @brief Functions to profile the codelet
 *
 * Two levels of profiling are available:
 *   1) A generic one that returns the flops per worker
 *   2) A more detailed one that generate logs of the performance for each kernel
 */
starpu_profile_t blok_zadd_profile = {
    .next = NULL,
    .name = "blok_zadd"
};

/**
 * @brief Profiling registration function
 */
void blok_zadd_profile_register( void ) __attribute__( ( constructor ) );
void
blok_zadd_profile_register( void )
{
    profiling_register_cl( &blok_zadd_profile );
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(PASTIX_STARPU_PROFILING_LOG)
static void
cl_profiling_cb_blok_zadd( void *callback_arg )
{
    cl_profiling_callback( callback_arg );

    struct starpu_task                *task = starpu_task_get_current();
    struct starpu_profiling_task_info *info = task->profiling_info;

    /* Quick return */
    if ( info == NULL ) {
        return;
    }

    struct cl_blok_zadd_args_s *args     = (struct cl_blok_zadd_args_s *) callback_arg;
    pastix_fixdbl_t             flops    = args->profile_data.flops;
    pastix_fixdbl_t             duration = starpu_timing_timespec_delay_us( &info->start_time, &info->end_time );
    pastix_fixdbl_t             speed    = flops / ( 1000.0 * duration );

    const SolverBlok  *blok = args->cblk->fblokptr + args->blok_m;
    pastix_int_t M = blok_rownbr( blok );
    pastix_int_t N = cblk_colnbr( args->cblk );

    cl_profiling_log_register( task->name, "blok_zadd", M, N, 0, flops, speed );
}
#endif

#if defined(PASTIX_STARPU_PROFILING_LOG)
static void (*blok_zadd_callback)(void*) = cl_profiling_cb_blok_zadd;
#else
static void (*blok_zadd_callback)(void*) = cl_profiling_callback;
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
fct_blok_zadd_cost( struct starpu_task           *task,
                    struct starpu_perfmodel_arch *arch,
                    unsigned                      nimpl )
{
    struct cl_blok_zadd_args_s *args = (struct cl_blok_zadd_args_s *)(task->cl_arg);

    pastix_fixdbl_t   cost = 0.;
    pastix_fixdbl_t  *coefs;
    const SolverBlok *blok = args->cblk->fblokptr + args->blok_m;
    pastix_int_t      M    = blok_rownbr( blok );
    pastix_int_t      N    = cblk_colnbr( args->cblk );

    switch( arch->devices->type ) {
    case STARPU_CPU_WORKER:
        coefs = &(args->sopalin_data->cpu_models->coefficients[PastixComplex64-2][PastixKernelGEADDCblkFRFR][0]);
        break;
    case STARPU_CUDA_WORKER:
        coefs = &(args->sopalin_data->gpu_models->coefficients[PastixComplex64-2][PastixKernelGEADDCblkFRFR][0]);
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
static struct starpu_perfmodel starpu_blok_zadd_model = {
#if defined(PASTIX_STARPU_COST_PER_ARCH)
    .type               = STARPU_PER_ARCH,
    .arch_cost_function = fct_blok_zadd_cost,
#else
    .type               = STARPU_HISTORY_BASED,
#endif
    .symbol             = "blok_zadd",
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
fct_blok_zadd_cpu( void *descr[], void *cl_arg )
{
    struct cl_blok_zadd_args_s *args = (struct cl_blok_zadd_args_s *)cl_arg;
    const void                 *A;
    void                       *B;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    B = pastix_starpu_blok_get_ptr( descr[1] );

    assert( args->cblk->cblktype  & CBLK_LAYOUT_2D );
    assert( args->fcblk->cblktype & CBLK_LAYOUT_2D );

    args->profile_data.flops = cpublok_zadd( 1., args->cblk, args->fcblk, args->blok_m, args->fblok_m,
                                             A, B, NULL, 0,
                                             &( args->sopalin_data->solvmtx->lowrank ) );
}

#if defined(PASTIX_WITH_CUDA) && 0
/**
 *******************************************************************************
 *
 * @brief StarPU GPU implementation
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
fct_blok_zadd_gpu( void *descr[], void *cl_arg )
{
    struct cl_template_args_s *args = (struct cl_template_args_s *)cl_arg;
    const void                *A;
    void                      *B;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    B = pastix_starpu_blok_get_ptr( descr[1] );

    assert( args->cblk->cblktype  & CBLK_TASKS_2D );
    assert( args->fcblk->cblktype & CBLK_TASKS_2D );

    args->profile_data.flops = gpublok_zadd( 1., args->cblk, args->fcblk, args->blok_m, args->fblok_m,
                                             A, B,
                                             &( args->sopalin_data->solvmtx->lowrank ),
                                             starpu_cuda_get_local_stream() );

}
#endif /* defined(PASTIX_WITH_CUDA) */
#endif /* !defined(PASTIX_STARPU_SIMULATION) */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
CODELETS_CPU( blok_zadd, 2 );
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 *******************************************************************************
 *
 * @brief Insert the task to add a fanin cblk on the receiver side (The fanin is
 * seen on this side as the RECV cblk).  Note that the caller always execute the
 * task.
 *
 *******************************************************************************
 *
 * @param[in] sopalin_data
 *          Solver matrix information structure that will guide the algorithm.
 *
 * @param[in] side
 *          Define which side of the cblk must be tested.
 *          @arg PastixLCoef if lower part only
 *          @arg PastixUCoef if upper part only
 *          @arg PastixLUCoef if both sides.
 *
 * @param[in] cblk
 *          The column block of the matrix.
 *
 * @param[in] blok
 *          The block of the matrix.
 *
 * @param[in] fcblk
 *          The facing column block of the matrix.
 *
 * @param[in] fblok
 *          The facing block of the matrix.
 *
 * @param[in] prio
 *          The task priority.
 *
 *******************************************************************************/
void
starpu_task_blok_zadd_recv( sopalin_data_t    *sopalin_data,
                            pastix_coefside_t  side,
                            const SolverCblk  *cblk,
                            const SolverBlok  *blok,
                            SolverCblk        *fcblk,
                            SolverBlok        *fblok,
                            int                prio )
{
    struct cl_blok_zadd_args_s *cl_arg = NULL;
#if defined(PASTIX_DEBUG_STARPU)
    char                       *task_name;
#endif

    assert( blok->fcblknm == fblok->fcblknm );

    /*
     * Create the arguments array
     */
    cl_arg                        = malloc( sizeof( struct cl_blok_zadd_args_s) );
    cl_arg->sopalin_data          = sopalin_data;
#if defined(PASTIX_STARPU_PROFILING)
    cl_arg->profile_data.measures = blok_zadd_profile.measures;
    cl_arg->profile_data.flops    = NAN;
#endif
    cl_arg->side                  = side;
    cl_arg->cblk                  = cblk;
    cl_arg->fcblk                 = fcblk;
    cl_arg->blok_m                = blok - cblk->fblokptr;
    cl_arg->fblok_m               = fblok - fcblk->fblokptr;

#if defined(PASTIX_DEBUG_STARPU)
    /* This actually generates a memory leak */
    asprintf( &task_name, "%s( %ld )",
              cl_blok_zadd_cpu.name,
              (long)(cblk - sopalin_data->solvmtx->cblktab) );
#endif

    assert( cblk->cblktype & CBLK_RECV );
    assert( !(fcblk->cblktype & (CBLK_RECV|CBLK_FANIN)) );

    pastix_starpu_insert_task(
        &cl_blok_zadd_cpu,
        STARPU_CL_ARGS,                 cl_arg,                 sizeof( struct cl_blok_zadd_args_s ),
        STARPU_EXECUTE_ON_NODE,         fcblk->ownerid,
#if defined(PASTIX_STARPU_PROFILING)
        STARPU_CALLBACK_WITH_ARG_NFREE, blok_zadd_callback, cl_arg,
#endif
        STARPU_R,                       blok->handler[side],
        STARPU_RW,                      fblok->handler[side],
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
 *******************************************************************************
 *
 * @brief Insert the task to add a fanin cblk on the emitter side. Note that
 * this task is submitted only to emit a send to the owner of the associated
 * recv cblk that will perform the add. Thus, the task is always submitted but
 * never executed.
 *
 *******************************************************************************
 *
 * @param[in] sopalin_data
 *          Solver matrix information structure that will guide the algorithm.
 *
 * @param[in] side
 *          Define which side of the cblk must be tested.
 *          @arg PastixLCoef if lower part only
 *          @arg PastixUCoef if upper part only
 *          @arg PastixLUCoef if both sides.
 *
 * @param[in] cblk
 *          The column block of the matrix.
 *
 * @param[in] blok
 *          The block of the matrix.
 *
 * @param[in] prio
 *          The task priority.
 *
 *******************************************************************************/
void
starpu_task_blok_zadd_fanin( sopalin_data_t    *sopalin_data,
                             pastix_coefside_t  side,
                             const SolverCblk  *cblk,
                             const SolverBlok  *blok,
                             int                prio )
{
    assert( cblk->cblktype & CBLK_FANIN );

    pastix_starpu_insert_task(
        NULL,
        STARPU_EXECUTE_ON_NODE, cblk->ownerid,
        STARPU_R,               blok->handler[side],
#if defined(PASTIX_STARPU_HETEROPRIO)
        STARPU_PRIORITY,        BucketFacto1D,
#else
        STARPU_PRIORITY,        prio,
#endif
        0);

    (void)sopalin_data;
    (void)prio;
}

/**
 * @}
 */
