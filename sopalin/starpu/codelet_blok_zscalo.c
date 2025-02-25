/**
 *
 * @file codelet_blok_zscalo.c
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
 * @author Alycia Lisito
 * @author Nolan Bredel
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
 * @brief Main structure for all tasks of blok_zscalo type
 */
struct cl_blok_zscalo_args_s {
    profile_data_t    profile_data;
    sopalin_data_t   *sopalin_data;
    pastix_trans_t    trans;
    const SolverCblk *cblk;
    pastix_int_t      blok_m;
};

/**
 * @brief Functions to profile the codelet
 *
 * Two levels of profiling are available:
 *   1) A generic one that returns the flops per worker
 *   2) A more detailed one that generate logs of the performance for each kernel
 */
#if defined( PASTIX_STARPU_PROFILING )
starpu_profile_t blok_zscalo_profile = {
    .next = NULL,
    .name = "blok_zscalo"
};

/**
 * @brief Profiling registration function
 */
void blok_zscalo_profile_register( void ) __attribute__( ( constructor ) );
void
blok_zscalo_profile_register( void )
{
    profiling_register_cl( &blok_zscalo_profile );
}

static void (*blok_ztrsmsp_callback)(void*) = cl_profiling_callback;

#endif /* defined( PASTIX_STARPU_PROFILING ) */

#if defined(PASTIX_STARPU_COST_PER_ARCH)
/**
 * @brief Cost model function
 *
 * The user can switch from the pastix static model to an history based model
 * computed automatically.
 */
static inline pastix_fixdbl_t
fct_blok_zscalo_cost( struct starpu_task           *task,
                      struct starpu_perfmodel_arch *arch,
                      unsigned                      nimpl )
{
    struct cl_blok_zscalo_args_s *args = (struct cl_blok_zscalo_args_s *)(task->cl_arg);

    pastix_fixdbl_t  cost = 0.;
    pastix_fixdbl_t *coefs;
    pastix_int_t     M = blok_rownbr_ext( args->cblk->fblokptr + args->blok_m );
    pastix_int_t     N = cblk_colnbr( args->cblk );

    switch( arch->devices->type ) {
    case STARPU_CPU_WORKER:
        coefs = &(args->sopalin_data->cpu_models->coefficients[PastixComplex64-2][PastixKernelSCALOBlok][0]);
        break;
    case STARPU_CUDA_WORKER:
        coefs = &(args->sopalin_data->gpu_models->coefficients[PastixComplex64-2][PastixKernelSCALOBlok][0]);
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

static struct starpu_perfmodel starpu_blok_zscalo_model = {
#if defined(PASTIX_STARPU_COST_PER_ARCH)
    .type               = STARPU_PER_ARCH,
    .arch_cost_function = fct_blok_zscalo_cost,
#else
    .type               = STARPU_HISTORY_BASED,
#endif
    .symbol             = "blok_zscalo",
};

#if !defined(PASTIX_STARPU_SIMULATION)
/**
 * @brief StarPU CPU implementation
 */
static void
fct_blok_zscalo_cpu( void *descr[], void *cl_arg )
{
    struct cl_blok_zscalo_args_s *args = (struct cl_blok_zscalo_args_s *)cl_arg;
    const void                   *A;
    const void                   *D;
    void                         *B;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    D = pastix_starpu_blok_get_ptr( descr[1] );
    B = pastix_starpu_blok_get_ptr( descr[2] );

    assert( args->cblk->cblktype & CBLK_TASKS_2D );

    cpublok_zscalo( args->trans, args->cblk, args->blok_m, A, D, B );
}
#endif /* !defined(PASTIX_STARPU_SIMULATION) */

CODELETS_CPU( blok_zscalo, 3 );

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
 * @param[in] trans
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
starpu_task_blok_zscalo( sopalin_data_t   *sopalin_data,
                         pastix_trans_t    trans,
                         const SolverCblk *cblk,
                         SolverBlok       *blok,
                         int               prio )
{
    struct cl_blok_zscalo_args_s *cl_arg    = NULL;
    int                           need_exec = 1;
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
    char                         *task_name;
#endif

    starpu_data_handle_t *handler = (starpu_data_handle_t *)( blok->handler );
    pastix_int_t          blok_m  = blok - cblk->fblokptr;

    pastix_starpu_register_blok( handler+1, cblk, blok, PastixComplex64 );

#if defined(PASTIX_WITH_MPI)
    {
        int64_t tag_desc = sopalin_data->solvmtx->starpu_desc->mpitag;
        int64_t tag_cblk = 2 * sopalin_data->solvmtx->gcblknbr;
        int64_t tag_blok = 2 * (blok - sopalin_data->solvmtx->bloktab) + 1;

        starpu_mpi_data_register( *(handler + 1),
                                  tag_desc + tag_cblk + tag_blok,
                                  cblk->ownerid );
    }
#endif /* PASTIX_WITH_MPI */

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
        if ( !need_submit ) {
            return;
        }
    }
#endif

    /*
     * Create the arguments array
     */
    if ( need_exec ) {
        cl_arg                        = malloc( sizeof(struct cl_blok_zscalo_args_s) );
        cl_arg->sopalin_data          = sopalin_data;
#if defined(PASTIX_STARPU_PROFILING)
        cl_arg->profile_data.measures = blok_zscalo_profile.measures;
        cl_arg->profile_data.flops    = NAN;
#endif
        cl_arg->trans                 = trans;
        cl_arg->cblk                  = cblk;
        cl_arg->blok_m                = blok_m;
    }

#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
    /* This actually generates a memory leak */
    asprintf( &task_name, "%s( %ld, %ld )",
              cl_blok_zscalo_cpu.name,
              (long)(cblk - sopalin_data->solvmtx->cblktab),
              (long)(blok - sopalin_data->solvmtx->bloktab) );
#endif

    pastix_starpu_insert_task(
        &cl_blok_zscalo_cpu,
        STARPU_CL_ARGS,                 cl_arg,                sizeof( struct cl_blok_zscalo_args_s ),
#if defined(PASTIX_STARPU_PROFILING)
        STARPU_CALLBACK_WITH_ARG_NFREE, blok_zscalo_callback, cl_arg,
#endif
        STARPU_R,                       blok->handler[0],
        STARPU_R,                       cblk->fblokptr->handler[0],
        STARPU_W,                       blok->handler[1],
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
        STARPU_NAME,                    task_name,
#endif
#if defined(PASTIX_STARPU_HETEROPRIO)
        STARPU_PRIORITY,                BucketScalo,
#else
        STARPU_PRIORITY,                prio,
#endif
        0);
    (void)prio;
}
/**
 * @}
 */
