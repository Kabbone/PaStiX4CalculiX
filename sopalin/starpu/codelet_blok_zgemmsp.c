/**
 *
 * @file codelet_blok_zgemmsp.c
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
 * @brief Main structure for all tasks of blok_zgemmsp type
 */
struct cl_blok_zgemmsp_args_s {
    profile_data_t    profile_data;
    sopalin_data_t   *sopalin_data;
    pastix_trans_t    trans;
    const SolverCblk *cblk;
    SolverCblk       *fcblk;
    pastix_int_t      blok_mk;
    pastix_int_t      blok_nk;
    pastix_int_t      blok_mn;
};

#if defined( PASTIX_STARPU_PROFILING )
/**
 * @brief Functions to profile the codelet
 *
 * Two levels of profiling are available:
 *   1) A generic one that returns the flops per worker
 *   2) A more detailed one that generate logs of the performance for each kernel
 */
starpu_profile_t blok_zgemmsp_profile = {
    .next = NULL,
    .name = "blok_zgemmsp"
};

/**
 * @brief Profiling registration function
 */
void blok_zgemmsp_profile_register( void ) __attribute__( ( constructor ) );
void
blok_zgemmsp_profile_register( void )
{
    profiling_register_cl( &blok_zgemmsp_profile );
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(PASTIX_STARPU_PROFILING_LOG)
static void
cl_profiling_cb_blok_zgemmsp( void *callback_arg )
{
    cl_profiling_callback( callback_arg );

    struct starpu_task                *task = starpu_task_get_current();
    struct starpu_profiling_task_info *info = task->profiling_info;

    /* Quick return */
    if ( info == NULL ) {
        return;
    }

    struct cl_blok_zgemmsp_args_s *args     = (struct cl_blok_zgemmsp_args_s *) callback_arg;
    pastix_fixdbl_t                flops    = args->profile_data.flops;
    pastix_fixdbl_t                duration = starpu_timing_timespec_delay_us( &info->start_time, &info->end_time );
    pastix_fixdbl_t                speed    = flops / ( 1000.0 * duration );

    pastix_int_t M = blok_rownbr_ext( args->cblk->fblokptr + args->blok_mk );
    pastix_int_t N = blok_rownbr_ext( args->cblk->fblokptr + args->blok_nk );
    pastix_int_t K = cblk_colnbr( args->cblk );

    cl_profiling_log_register( task->name, "blok_zgemmsp", M, N, K, flops, speed );
}
#endif

#if defined(PASTIX_STARPU_PROFILING_LOG)
static void (*blok_zgemmsp_callback)(void*) = cl_profiling_cb_blok_zgemmsp;
#else
static void (*blok_zgemmsp_callback)(void*) = cl_profiling_callback;
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
fct_blok_zgemmsp_cost( struct starpu_task           *task,
                       struct starpu_perfmodel_arch *arch,
                       unsigned                      nimpl )
{
    struct cl_blok_zgemmsp_args_s *args = (struct cl_blok_zgemmsp_args_s *)(task->cl_arg);

    pastix_fixdbl_t  cost = 0.;
    pastix_fixdbl_t *coefs;
    pastix_int_t     M = blok_rownbr_ext( args->cblk->fblokptr + args->blok_mk );
    pastix_int_t     N = blok_rownbr_ext( args->cblk->fblokptr + args->blok_nk );
    pastix_int_t     K = cblk_colnbr( args->cblk );

    switch( arch->devices->type ) {
    case STARPU_CPU_WORKER:
        coefs = &(args->sopalin_data->cpu_models->coefficients[PastixComplex64-2][PastixKernelGEMMBlok2d2d][0]);
        break;
    case STARPU_CUDA_WORKER:
        coefs = &(args->sopalin_data->gpu_models->coefficients[PastixComplex64-2][PastixKernelGEMMBlok2d2d][0]);
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
static struct starpu_perfmodel starpu_blok_zgemmsp_model = {
#if defined(PASTIX_STARPU_COST_PER_ARCH)
    .type               = STARPU_PER_ARCH,
    .arch_cost_function = fct_blok_zgemmsp_cost,
#else
    .type               = STARPU_HISTORY_BASED,
#endif
    .symbol             = "blok_zgemmsp",
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
fct_blok_zgemmsp_cpu( void *descr[], void *cl_arg )
{
    struct cl_blok_zgemmsp_args_s *args = (struct cl_blok_zgemmsp_args_s *)cl_arg;
    const void                    *A;
    const void                    *B;
    void                          *C;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    B = pastix_starpu_blok_get_ptr( descr[1] );
    C = pastix_starpu_blok_get_ptr( descr[2] );

    assert( args->cblk->cblktype  & CBLK_TASKS_2D );
    assert( args->fcblk->cblktype & CBLK_TASKS_2D );

    args->profile_data.flops = cpublok_zgemmsp( args->trans,
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
fct_blok_zgemmsp_gpu( void *descr[], void *cl_arg )
{
    struct cl_blok_zgemmsp_args_s *args = (struct cl_blok_zgemmsp_args_s *)cl_arg;
    const void                    *A;
    const void                    *B;
    void                          *C;

    A = pastix_starpu_blok_get_ptr( descr[0] );
    B = pastix_starpu_blok_get_ptr( descr[1] );
    C = pastix_starpu_blok_get_ptr( descr[2] );

    assert( args->cblk->cblktype  & CBLK_TASKS_2D );
    assert( args->fcblk->cblktype & CBLK_TASKS_2D );

    args->profile_data.flops = gpublok_zgemmsp( args->trans,
                                                args->cblk, args->fcblk,
                                                args->blok_mk, args->blok_nk, args->blok_mn,
                                                A, B, C,
                                                &(args->sopalin_data->solvmtx->lowrank),
                                                starpu_cuda_get_local_stream() );
}
#endif /* defined(PASTIX_WITH_CUDA) */
#endif /* !defined(PASTIX_STARPU_SIMULATION) */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
CODELETS_GPU( blok_zgemmsp, 3, STARPU_CUDA_ASYNC );
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
 * @param[in] sideA
 *          TODO
 *
 * @param[in] sideB
 *          TODO
 *
 * @param[in] trans
 *          TODO
 *
 * @param[in] cblk
 *          TODO
 *
 * @param[in] fcblk
 *          TODO
 *
 * @param[in] blokA
 *          TODO
 *
 * @param[in] blokB
 *          TODO
 *
 * @param[in] prio
 *          TODO
 *
 *******************************************************************************/
void
starpu_task_blok_zgemmsp( sopalin_data_t   *sopalin_data,
                          pastix_coefside_t sideA,
                          pastix_coefside_t sideB,
                          pastix_trans_t    trans,
                          SolverCblk       *cblk,
                          SolverCblk       *fcblk,
                          const SolverBlok *blokA,
                          const SolverBlok *blokB,
                          int               prio )
{
    struct cl_blok_zgemmsp_args_s *cl_arg        = NULL;
    long long                      execute_where = cl_blok_zgemmsp_any.where;
    int                            need_exec     = 1;
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
    char                          *task_name;
#endif

    pastix_int_t frownum;
    pastix_int_t lrownum;
    pastix_int_t blok_mn = 0, j = 0;
    pastix_int_t blok_mk = blokA - cblk->fblokptr;
    pastix_int_t blok_nk = blokB - cblk->fblokptr;
    SolverBlok  *blokC   = fcblk->fblokptr;

    assert( blok_nk <= blok_mk );

    do {
        frownum = blokC->frownum;
        lrownum = blokC->lrownum;
        blok_mn += j;
        j = 1;

        /* Increase lrownum as long as blocks are facing the same cblk */
        while( (blokC < fcblk[1].fblokptr-1) &&
               (blokC[0].fcblknm == blokC[1].fcblknm) &&
               (blokC[0].lcblknm == blokC[1].lcblknm) )
        {
            blokC++; j++;
            lrownum = blokC->lrownum;
        }
        blokC++;
    }
    while( !((blokA->frownum >= frownum) &&
             (blokA->lrownum <= lrownum)) );

    blokC = fcblk->fblokptr + blok_mn;

    assert( blokA->lcblknm == blokB->lcblknm );
    assert( blokB->fcblknm == blokC->lcblknm );
    assert( blokC->frownum <= blokA->frownum );
    assert( blokA[-1].fcblknm != blokA[0].fcblknm );
    assert( blokB[-1].fcblknm != blokB[0].fcblknm );
    assert( (blok_mn == 0) || (blokC[-1].fcblknm != blokC[0].fcblknm) );

    /*
     * Check if it needs to be submitted
     */
#if defined(PASTIX_WITH_MPI)
    {
        int need_submit = 0;
        if ( cblk->ownerid == sopalin_data->solvmtx->clustnum ) {
            need_submit = 1;
        }
        if ( (fcblk->cblktype & CBLK_FANIN) ||
             (fcblk->ownerid == sopalin_data->solvmtx->clustnum) ) {
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
        cl_arg                        = malloc( sizeof( struct cl_blok_zgemmsp_args_s ) );
        cl_arg->sopalin_data          = sopalin_data;
#if defined(PASTIX_STARPU_PROFILING)
        cl_arg->profile_data.measures = blok_zgemmsp_profile.measures;
        cl_arg->profile_data.flops    = NAN;
#endif
        cl_arg->trans                 = trans;
        cl_arg->cblk                  = cblk;
        cl_arg->fcblk                 = fcblk;
        cl_arg->blok_mk               = blok_mk;
        cl_arg->blok_nk               = blok_nk;
        cl_arg->blok_mn               = blok_mn;

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
    asprintf( &task_name, "%s( %ld, %ld, %ld, %ld )",
              cl_blok_zgemmsp_any.name,
              (long)(blokA - sopalin_data->solvmtx->bloktab),
              (long)(blokB - sopalin_data->solvmtx->bloktab),
              (long)(blokC - sopalin_data->solvmtx->bloktab),
              (long)sideA );
#endif

    pastix_starpu_insert_task(
        &cl_blok_zgemmsp_any,
        STARPU_CL_ARGS,                 cl_arg,                sizeof( struct cl_blok_zgemmsp_args_s ),
        STARPU_EXECUTE_WHERE,           execute_where,
#if defined(PASTIX_STARPU_PROFILING)
        STARPU_CALLBACK_WITH_ARG_NFREE, blok_zgemmsp_callback, cl_arg,
#endif
        STARPU_R,                       blokA->handler[sideA],
        STARPU_R,                       blokB->handler[sideB],
        STARPU_RW,                      blokC->handler[sideA],
#if defined(PASTIX_DEBUG_STARPU) || defined(PASTIX_STARPU_PROFILING_LOG)
        STARPU_NAME,                    task_name,
#endif
#if defined(PASTIX_STARPU_HETEROPRIO)
        STARPU_PRIORITY,                BucketGEMM2D,
#else
        STARPU_PRIORITY,                prio,
#endif
        0);
    (void)prio;
}

/**
 * @}
 */
