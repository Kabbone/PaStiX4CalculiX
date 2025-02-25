/**
 *
 * @file codelet_template.c
 *
 * StarPU codelets for blas-like functions
 *
 * @copyright 2016-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @date 2024-07-05
 *
 * @precisions normal z -> z c d s
 *
 * Template to generate a new codelet. You just need to query-replace template
 * by your prefered nickname.
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
 * @brief Main structure for all tasks of template type
 */
struct cl_template_args_s {
    profile_data_t    profile_data;
    sopalin_data_t   *sopalin_data;
};

/**
 * @brief Functions to profile the codelet
 *
 * Two levels of profiling are available:
 *   1) A generic one that returns the flops per worker
 *   2) A more detailed one that generate logs of the performance for each kernel
 */
#if defined( PASTIX_STARPU_PROFILING )
starpu_profile_t template_profile = {
    .next = NULL,
    .name = "template"
};

/**
 * @brief Profiling registration function
 */
void template_profile_register( void ) __attribute__( ( constructor ) );
void
template_profile_register( void )
{
    profiling_register_cl( &template_profile );
}

#if defined(PASTIX_STARPU_PROFILING_LOG)
static void
cl_profiling_cb_template( void *callback_arg )
{
    cl_profiling_callback( callback_arg );

    struct starpu_task                *task = starpu_task_get_current();
    struct starpu_profiling_task_info *info = task->profiling_info;

    /* Quick return */
    if ( info == NULL ) {
        return;
    }

    struct cl_template_args_s *args     = (struct cl_template_args_s *) callback_arg;
    pastix_fixdbl_t            flops    = args->profile_data.flops;
    pastix_fixdbl_t            duration = starpu_timing_timespec_delay_us( &info->start_time, &info->end_time );
    pastix_fixdbl_t            speed    = flops / ( 1000.0 * duration );

    pastix_int_t M = ;
    pastix_int_t N = ;
    pastix_int_t K = ;

    cl_profiling_log_register( task->name, "template", M, N, K, flops, speed );
}
#endif

#if defined(PASTIX_STARPU_PROFILING_LOG)
static void (*template_callback)(void*) = cl_profiling_cb_template;
#else
static void (*template_callback)(void*) = cl_profiling_callback;
#endif

#endif /* defined( PASTIX_STARPU_PROFILING ) */

#if defined(PASTIX_STARPU_COST_PER_ARCH)
/**
 * @brief Cost model function
 *
 * The user can switch from the pastix static model to an history based model
 * computed automatically.
 */
static inline pastix_fixdbl_t
fct_template_cost( struct starpu_task           *task,
                       struct starpu_perfmodel_arch *arch,
                       unsigned                      nimpl )
{
    struct cl_template_args_s *args = (struct cl_template_args_s *)(task->cl_arg);

    pastix_fixdbl_t  cost = 0.;
    pastix_fixdbl_t *coefs;
    pastix_int_t     M = blok_rownbr_ext( args->cblk->fblokptr + args->blok_mk );
    pastix_int_t     N = blok_rownbr_ext( args->cblk->fblokptr + args->blok_nk );
    pastix_int_t     K = cblk_colnbr( args->cblk );

    switch( arch->devices->type ) {
    case STARPU_CPU_WORKER:
        coefs = &(args->sopalin_data->cpu_models->coefficients[PastixComplex64-2][PastixKernelXXXX][0]);
        break;
    case STARPU_CUDA_WORKER:
        coefs = &(args->sopalin_data->gpu_models->coefficients[PastixComplex64-2][PastixKernelXXXX][0]);
        break;
    default:
        assert(0);
        return 0.;
    }

    /* Get cost in us */
    cost = modelsGetCost3Param( coefs, M, N, K ) * 1e6;

    (void)nimpl;
    return cost;
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static struct starpu_perfmodel starpu_template_model = {
#if defined(PASTIX_STARPU_COST_PER_ARCH)
    .type               = STARPU_PER_ARCH,
    .arch_cost_function = fct_template_cost,
#else
    .type               = STARPU_HISTORY_BASED,
#endif
    .symbol             = "template",
};
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#if !defined(PASTIX_STARPU_SIMULATION)
/**
 * @brief StarPU CPU implementation
 */
static void
fct_template_cpu( void *descr[], void *cl_arg )
{
    struct cl_template_args_s *args = (struct cl_template_args_s *)cl_arg;
    const void                    *A;
    const void                    *B;
    void                          *C;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    B = pastix_starpu_blok_get_ptr( descr[1] );
    C = pastix_starpu_blok_get_ptr( descr[2] );

    assert( args->cblk->cblktype  & CBLK_TASKS_2D );
    assert( args->fcblk->cblktype & CBLK_TASKS_2D );

    args->profile_data.flops = cputemplate( args->trans,
                                                args->cblk, args->fcblk,
                                                args->blok_mk, args->blok_nk, args->blok_mn,
                                                A, B, C,
                                                &(args->sopalin_data->solvmtx->lowrank) );
}

/**
 * @brief StarPU GPU implementation
 */
#if defined(PASTIX_WITH_CUDA)
static void
fct_template_gpu( void *descr[], void *cl_arg )
{
    struct cl_template_args_s *args = (struct cl_template_args_s *)cl_arg;
    const void                    *A;
    const void                    *B;
    void                          *C;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    B = pastix_starpu_blok_get_ptr( descr[1] );
    C = pastix_starpu_blok_get_ptr( descr[2] );

    assert( args->cblk->cblktype  & CBLK_TASKS_2D );
    assert( args->fcblk->cblktype & CBLK_TASKS_2D );

    args->profile_data.flops = gputemplate( args->trans,
                                                args->cblk, args->fcblk,
                                                args->blok_mk, args->blok_nk, args->blok_mn,
                                                A, B, C,
                                                &(args->sopalin_data->solvmtx->lowrank),
                                                starpu_cuda_get_local_stream() );
}
#endif /* defined(PASTIX_WITH_CUDA) */
#endif /* !defined(PASTIX_STARPU_SIMULATION) */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
CODELETS_GPU( template, 3, STARPU_CUDA_ASYNC );
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

void
starpu_task_template( sopalin_data_t   *sopalin_data,
                          pastix_coefside_t sideA,
                          pastix_coefside_t sideB,
                          pastix_trans_t    trans,
                          const SolverCblk *cblk,
                          SolverCblk       *fcblk,
                          const SolverBlok *blokA,
                          const SolverBlok *blokB,
                          int               prio )
{
    struct cl_template_args_s *cl_arg        = NULL;
    long long                      execute_where = cl_template_any.where;
    int                            need_exec     = 1;
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
    char                          *task_name;
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
        if ( fcblk->ownerid == sopalin_data->solvmtx->clustnum ) {
            need_submit = 1;
        }
        else {
            need_exec = 0;
        }
        if ( starpu_mpi_cached_receive( blokC->handler[sideA] ) ) {
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
        cl_arg                        = malloc( sizeof( struct cl_template_args_s ) );
        cl_arg->sopalin_data          = sopalin_data;
#if defined(PASTIX_STARPU_PROFILING)
        cl_arg->profile_data.measures = template_profile.measures;
        cl_arg->profile_data.flops    = NAN;
#endif

#if defined(PASTIX_WITH_CUDA)
        if ( (cblk->cblktype  & CBLK_COMPRESSED) ||
             (fcblk->cblktype & CBLK_COMPRESSED) )
        {
            /* Disable CUDA */
            execute_where &= (~STARPU_CUDA);
        }
#endif
    }

#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
    /* This actually generates a memory leak */
    asprintf( &task_name, "%s( %ld, %ld )",
              cl_template_any.name,
               );
#endif

    pastix_starpu_insert_task(
        &cl_template_any,
        STARPU_CL_ARGS,                 cl_arg,                sizeof( struct cl_template_args_s ),
        STARPU_EXECUTE_WHERE,           execute_where,
#if defined(PASTIX_STARPU_PROFILING)
        STARPU_CALLBACK_WITH_ARG_NFREE, template_callback, cl_arg,
#endif
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
        STARPU_NAME,                    task_name,
#endif
#if defined(PASTIX_STARPU_HETEROPRIO)
        STARPU_PRIORITY,                BucketXXXX,
#else
        STARPU_PRIORITY,                prio,
#endif
        0);
    (void)prio;
}

/**
 * @}
 */
