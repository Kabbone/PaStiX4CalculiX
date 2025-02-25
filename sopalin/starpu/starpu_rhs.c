/**
 *
 * @file starpu_rhs.c
 *
 * PaStiX dense matrix descriptor for StarPU.
 *
 * @copyright 2016-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @author Pierre Ramet
 * @author Alycia Lisito
 * @date 2024-07-05
 *
 * @ingroup pastix_starpu
 * @{
 *
 **/
#include "common.h"
#include "blend/solver.h"
#include "pastix_starpu.h"
#include <starpu_data.h>

/**
 *******************************************************************************
 *
 * @brief Generate the StarPU descriptor of the dense matrix.
 *
 * This function creates the StarPU descriptor that will provide tha data
 * mapping and memory location to StarPU for the computation.
 *
 *******************************************************************************
 *
 * @param[inout] solvmtx
 *          The solver matrix structure that describes the dense matrix for
 *          PaStiX.
 *
 * @param[inout] mpitag
 *          The mpitag.
 *
 * @param[inout] clustnum
 *          The clustnum.
 *
 * @param[in] ncol
 *          The number of columns of the given matrix. The number of rows is
 *          given by the solvmtx structure.
 *
 * @param[in] A
 *          The pointer to the matrix.
 *
 * @param[in] lda
 *          The leading dimension of the matrix A.
 *
 * @param[in] typesize
 *          The memory size of the arithmetic used to store the matrix
 *          coefficients.
 *
 * @param[in] nodes
 *          The number of processes used to solve the problem.
 *
 * @param[in] myrank
 *          The rank of the calling process.
 *
 ******************************************************************************/
void
pastix_starpu_rhs_data_register( starpu_data_handle_t *handleptr,
                                 int64_t               mpitag,
                                 int                   clustnum,
                                 const SolverCblk     *cblk,
                                 size_t                typesize,
                                 pastix_int_t          m,
                                 pastix_int_t          n,
                                 char                 *A,
                                 pastix_int_t          lda )
{
    int       home_node = STARPU_MAIN_RAM;
    uintptr_t ptr       = (uintptr_t)A;
    int64_t   tag_cblk  = -1;

    if ( cblk->cblktype & ( CBLK_FANIN | CBLK_RECV ) ) {
        home_node = -1;
        lda       = m;
        ptr       = 0;
    }
    else {
        assert( ptr != 0 );
    }

    /* The default StarPU dense matrix type is used to describe the rhs. */
    starpu_matrix_data_register( handleptr, home_node, ptr, lda, m, n, typesize );

#if defined(PASTIX_WITH_MPI)
    if ( cblk->cblktype & CBLK_FANIN ) {
        starpu_data_set_reduction_methods( *handleptr, NULL, &cl_rhs_init_cpu );
        tag_cblk = mpitag + cblk->gfaninnum;
        starpu_mpi_data_register( *handleptr, tag_cblk, clustnum );
    }
    else {
        if ( cblk->cblktype & CBLK_RECV ) {
            starpu_data_set_reduction_methods( *handleptr, NULL, &cl_rhs_init_cpu );
            tag_cblk = mpitag + cblk->gfaninnum;
        }
        else {
            tag_cblk = mpitag + cblk->gcblknum;
        }
        starpu_mpi_data_register( *handleptr, tag_cblk, cblk->ownerid );
    }
#endif

#if defined(PASTIX_DEBUG_STARPU)
        fprintf( stderr, "[%2d][pastix][%s] Solve cblk=%d, owner=%d, tag=%ld, size=%ld\n",
                 clustnum, __func__, cblk->gcblknum, cblk->ownerid, tag_cblk,
                 n * cblk_colnbr( cblk ) * pastix_size_of( solvmtx->flttype ) );
#endif

    (void)mpitag;
    (void)clustnum;
    (void)tag_cblk;
}

/**
 *******************************************************************************
 *
 * @brief Generate the StarPU descriptor of the dense matrix.
 *
 * This function creates the StarPU descriptor that will provide tha data
 * mapping and memory location to StarPU for the computation.
 *
 *******************************************************************************
 *
 * @param[inout] solvmtx
 *          The solver matrix structure that describes the dense matrix for
 *          PaStiX.
 *
 * @param[inout] rhsb
 *          The pointer to the rhs data structure that holds the vectors of the
 *          right hand side.
 *
 * @param[in] typesize
 *          The memory size of the arithmetic used to store the matrix
 *          coefficients.
 *
 * @param[in] nodes
 *          The number of processes used to solve the problem.
 *
 * @param[in] myrank
 *          The rank of the calling process.
 *
 ******************************************************************************/
void
starpu_rhs_init( SolverMatrix *solvmtx,
                 pastix_rhs_t  rhsb,
                 int           typesize,
                 int           nodes,
                 int           myrank )
{
    starpu_data_handle_t *handler;
    SolverCblk           *cblk;
    pastix_int_t          cblknbr, cblknum, nrow;
    pastix_int_t          ncol    = rhsb->n;
    starpu_rhs_desc_t    *rhsdesc = rhsb->starpu_desc;

    if ( rhsdesc != NULL ) {
        if ( ( ncol    == rhsdesc->ncol ) &&
             ( rhsb->b == rhsdesc->dataptr ) ) {
            return;
        }
        starpu_rhs_destroy( rhsdesc );
    }
    else {
        rhsdesc = (starpu_rhs_desc_t*)malloc(sizeof(starpu_rhs_desc_t));
    }

    cblknbr = solvmtx->cblknbr;

#if defined(PASTIX_WITH_MPI)
    {
        int64_t tag_desc = ( (int64_t) ( solvmtx->gcblknbr + solvmtx->gfanincblknbr ) );
        rhsdesc->mpitag  = pastix_starpu_tag_book( tag_desc );
    }
#endif
    rhsdesc->ncol      = ncol;
    rhsdesc->typesze   = pastix_size_of( typesize );
    rhsdesc->solvmtx   = solvmtx;
    rhsdesc->handletab = malloc( cblknbr * sizeof(starpu_data_handle_t) );
    rhsdesc->dataptr   = rhsb->b;

    /* Initialize 1D cblk handlers */
    cblk    = rhsdesc->solvmtx->cblktab;
    handler = rhsdesc->handletab;
    for( cblknum = 0; cblknum < cblknbr; cblknum++, cblk++, handler++ ) {
        nrow = cblk_colnbr( cblk );

        pastix_starpu_rhs_data_register( handler, rhsdesc->mpitag, solvmtx->clustnum, cblk, rhsdesc->typesze,
                                         nrow, ncol, (char*)rhsb->b + (cblk->lcolidx * rhsdesc->typesze), rhsb->ld );
    }

    rhsb->starpu_desc = rhsdesc;

    (void)nodes;
    (void)myrank;
}

/**
 *******************************************************************************
 *
 * @brief Submit asynchronous calls to retrieve the data on main memory.
 *
 *******************************************************************************
 *
 * @param[inout] rhsdesc
 *          The dense matrix descriptor to retrieve on main memory.
 *
 ******************************************************************************/
void
starpu_rhs_getoncpu( starpu_rhs_desc_t *rhsdesc )
{
    starpu_data_handle_t *handler = rhsdesc->handletab;
    SolverCblk *cblk;
    pastix_int_t cblknbr, cblknum;

    cblk    = rhsdesc->solvmtx->cblktab;
    cblknbr = rhsdesc->solvmtx->cblknbr;
    for(cblknum=0; cblknum<cblknbr; cblknum++, cblk++, handler++)
    {
        assert( handler );

#if defined(PASTIX_WITH_MPI)
        starpu_mpi_cache_flush( rhsdesc->solvmtx->solv_comm, *handler );
#endif
        if ( cblk->ownerid == rhsdesc->solvmtx->clustnum ) {
            starpu_data_acquire_cb( *handler, STARPU_R,
                                    (void (*)(void*))&starpu_data_release,
                                    *handler );
        }
    }
}

/**
 *******************************************************************************
 *
 * @brief Free the StarPU descriptor of the dense matrix.
 *
 * This function destroys the StarPU descriptor, but do not free the matrix data
 * that are managed by PaStiX.
 *
 *******************************************************************************
 *
 * @param[inout] rhsdesc
 *          The descriptor to free.
 *
 ******************************************************************************/
void
starpu_rhs_destroy( starpu_rhs_desc_t *rhsdesc )
{
    starpu_data_handle_t *handler = rhsdesc->handletab;
    SolverCblk *cblk;
    pastix_int_t cblknbr, cblknum;

    cblk    = rhsdesc->solvmtx->cblktab;
    cblknbr = rhsdesc->solvmtx->cblknbr;
    for( cblknum = 0; cblknum < cblknbr; cblknum++, cblk++, handler++ ) {
        assert( handler );
        starpu_data_unregister( *handler );
    }

    free( rhsdesc->handletab );
    rhsdesc->handletab = NULL;

    pastix_starpu_tag_release( rhsdesc->mpitag );
}

/**
 * @}
 */
