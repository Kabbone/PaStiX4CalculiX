/**
 *
 * @file starpu_ztrsm.c
 *
 * PaStiX ztrsm StarPU wrapper.
 *
 * @copyright 2016-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Vincent Bridonneau
 * @author Mathieu Faverge
 * @author Pierre Ramet
 * @author Alycia Lisito
 * @author Nolan Bredel
 * @author Tom Moenne-Loccoz
 * @date 2024-07-05
 * @precisions normal z -> s d c
 *
 * @addtogroup starpu_trsm_solve
 * @{
 *
 **/

#include "common.h"
#include "blend/solver.h"
#include "sopalin/sopalin_data.h"
#include "pastix_zcores.h"
#include "pastix_starpu.h"
#include "pastix_zstarpu.h"

/**
 *******************************************************************************
 *
 * @brief Apply a forward solve related to one cblk to all the right hand side.
 *        (StarPU version)
 *
 ********************************************************************************
 *
 * @param[in] enums
 *          Enums needed for the solve.
 *
 * @param[in] sopalin_data
 *          The data that provide the SolverMatrix structure from PaStiX, and
 *          descriptor of b (providing nrhs, b and ldb).
 *
 * @param[inout] rhsb
 *          The pointer to the rhs data structure that holds the vectors of the
 *          right hand side.
 *
 * @param[in] cblk
 *          The cblk structure to which block belongs to. The A and B pointers
 *          must be the coeftab of this column block.
 *          Next column blok must be accessible through cblk[1].
 *
 * @param[in] prio
 *          The priority of the task in th DAG.
 *
 *******************************************************************************/
void
starpu_cblk_ztrsmsp_forward( const args_solve_t *enums,
                             sopalin_data_t     *sopalin_data,
                             pastix_rhs_t        rhsb,
                             const SolverCblk   *cblk,
                             pastix_int_t        prio )
{
    pastix_coefside_t   cs;
    SolverMatrix       *datacode = sopalin_data->solvmtx;
    SolverCblk         *fcbk;
    SolverBlok         *blok;
    pastix_trans_t      tA;
    pastix_side_t       side  = enums->side;
    pastix_uplo_t       uplo  = enums->uplo;
    pastix_trans_t      trans = enums->trans;
    pastix_diag_t       diag  = enums->diag;
    pastix_solv_mode_t  mode  = enums->mode;

    if ( (cblk->cblktype & CBLK_IN_SCHUR) && (mode != PastixSolvModeSchur) ) {
        return;
    }

    if ( (side == PastixRight) && (uplo == PastixUpper) && (trans == PastixNoTrans) ) {
        /*  We store U^t, so we swap uplo and trans */
        tA = PastixTrans;
        cs = PastixUCoef;

        /* Right is not handled yet */
        assert( 0 );
    }
    else if ( (side == PastixRight) && (uplo == PastixLower) && (trans != PastixNoTrans) ) {
        tA = trans;
        cs = PastixLCoef;

        /* Right is not handled yet */
        assert( 0 );
    }
    else if ( (side == PastixLeft)  && (uplo == PastixUpper) && (trans != PastixNoTrans) ) {
        /*  We store U^t, so we swap uplo and trans */
        tA = PastixNoTrans;
        cs = PastixUCoef;

        /* We do not handle conjtrans in complex as we store U^t */
        assert( trans != PastixConjTrans );
    }
    else if ( (side == PastixLeft)  && (uplo == PastixLower) && (trans == PastixNoTrans) ) {
        tA = trans;
        cs = PastixLCoef;
    }
    else {
        /* This correspond to case treated in backward TRSM */
        assert(0);
        return;
    }

    /* Solve the diagonal block */
    starpu_stask_blok_ztrsm( sopalin_data, rhsb, cs, side, PastixLower,
                             tA, diag, cblk, prio );

    /* Apply the update */
    for (blok = cblk[0].fblokptr+1; blok < cblk[1].fblokptr; blok++ ) {
        fcbk  = datacode->cblktab + blok->fcblknm;

        if ( (fcbk->cblktype & CBLK_IN_SCHUR) && (mode == PastixSolvModeLocal) ) {
            return;
        }

        starpu_stask_blok_zgemm( sopalin_data, rhsb, cs, PastixLeft, tA,
                                 cblk, blok, fcbk, prio );
    }
}

/**
 *******************************************************************************
 *
 * @brief Apply a backward solve related to one cblk to all the right hand side.
 *        (StarPU version)
 *
 *******************************************************************************
 *
 * @param[in] enums
 *          Enums needed for the solve.
 *
 * @param[in] sopalin_data
 *          The data that provide the SolverMatrix structure from PaStiX, and
 *          descriptor of b (providing nrhs, b and ldb).
 *
 * @param[inout] rhsb
 *          The pointer to the rhs data structure that holds the vectors of the
 *          right hand side.
 *
 * @param[in] cblk
 *          The cblk structure to which block belongs to. The A and B pointers
 *          must be the coeftab of this column block.
 *          Next column blok must be accessible through cblk[1].
 *
 * @param[in] prio
 *          The priority of the task in th DAG.
 *
 *******************************************************************************/
void
starpu_cblk_ztrsmsp_backward( const args_solve_t *enums,
                              sopalin_data_t     *sopalin_data,
                              pastix_rhs_t        rhsb,
                              const SolverCblk   *cblk,
                              pastix_int_t        prio )
{
    pastix_coefside_t   cs;
    SolverMatrix       *datacode = sopalin_data->solvmtx;
    SolverCblk         *fcbk;
    SolverBlok         *blok;
    pastix_int_t        j;
    pastix_trans_t      tA;
    pastix_side_t       side  = enums->side;
    pastix_uplo_t       uplo  = enums->uplo;
    pastix_trans_t      trans = enums->trans;
    pastix_diag_t       diag  = enums->diag;
    pastix_solv_mode_t  mode  = enums->mode;

    /*
     *  Left / Upper / NoTrans (Backward)
     */
    if ( (side == PastixLeft)  && (uplo == PastixUpper) && (trans == PastixNoTrans) ) {
        /*  We store U^t, so we swap uplo and trans */
        tA = PastixTrans;
        cs = PastixUCoef;
    }
    else if ( (side == PastixLeft)  && (uplo == PastixLower) && (trans != PastixNoTrans) ) {
        tA = trans;
        cs = PastixLCoef;
    }
    else if ( (side == PastixRight) && (uplo == PastixUpper) && (trans != PastixNoTrans) ) {
        /*  We store U^t, so we swap uplo and trans */
        tA = PastixNoTrans;
        cs = PastixUCoef;

        /* Right is not handled yet */
        assert( 0 );

        /* We do not handle conjtrans in complex as we store U^t */
        assert( trans != PastixConjTrans );
    }
    else if ( (side == PastixRight) && (uplo == PastixLower) && (trans == PastixNoTrans) ) {
        tA = trans;
        cs = PastixLCoef;

        /* Right is not handled yet */
        assert( 0 );
    }
    else {
        /* This correspond to case treated in forward TRSM */
        assert(0);
        return;
    }
    assert( !(cblk->cblktype & CBLK_RECV) );

    if ( ( !(cblk->cblktype & CBLK_IN_SCHUR) || (mode == PastixSolvModeSchur) ) &&
         ( !(cblk->cblktype & CBLK_FANIN) ) )
    {
        /* Solve the diagonal block */
        starpu_stask_blok_ztrsm( sopalin_data, rhsb, cs, side, PastixLower,
                                 tA, diag, cblk, prio );
    }

    /* Apply the update */
    for (j   = cblk[1].brownum-1; j>=cblk[0].brownum; j-- ) {
        blok = datacode->bloktab + datacode->browtab[j];
        fcbk = datacode->cblktab + blok->lcblknm;

        if ( (fcbk->cblktype & CBLK_IN_SCHUR) && (mode == PastixSolvModeInterface) ) {
            continue;
        }
        if ( fcbk->cblktype & CBLK_RECV ) {
            continue;
        }

        starpu_stask_blok_zgemm( sopalin_data, rhsb, cs, PastixRight, tA,
                                 cblk, blok, fcbk, prio );
    }
}

/**
 *******************************************************************************
 *
 * @brief Apply a TRSM on a problem with 1 dimension (StarPU version)
 *
 *******************************************************************************
 *
 * @param[in] pastix_data
 *          The data that provide the mode.
 *
 * @param[in] sopalin_data
 *          The data that provide the SolverMatrix structure from PaStiX., and
 *          descriptor of b (providing nrhs, b and ldb).
 *
 * @param[inout] rhsb
 *          The pointer to the rhs data structure that holds the vectors of the
 *          right hand side.
 *
 * @param[in] enums
 *          Enums needed for the solve.
 *
 *******************************************************************************/
void
starpu_ztrsm_sp1dplus( pastix_data_t      *pastix_data,
                       sopalin_data_t     *sopalin_data,
                       pastix_rhs_t        rhsb,
                       const args_solve_t *enums )
{
    SolverMatrix *datacode = sopalin_data->solvmtx;
    SolverCblk   *cblk;
#if defined(PASTIX_WITH_MPI)
    SolverCblk   *fcblk;
#endif
    pastix_int_t  i, cblknbr, prio;

    /* Backward like */
    if ( enums->solve_step == PastixSolveBackward ) {
        cblknbr = (enums->mode == PastixSolvModeLocal) ? datacode->cblkschur : datacode->cblknbr;
        cblk    = datacode->cblktab + cblknbr - 1;

        for ( i = cblknbr-1; i >= 0; i--, cblk-- ) {
            prio = i;

#if defined(PASTIX_WITH_MPI)
            /* If this is a recv, let's locally copy and send the accumulation */
            if ( cblk->cblktype & CBLK_RECV ) {
                fcblk = datacode->cblktab + cblk->fblokptr->fcblknm;
                starpu_stask_blok_zcpy_bwd_recv( sopalin_data, rhsb, cblk, fcblk, prio );
                starpu_mpi_data_migrate( datacode->solv_comm,
                                         rhsb->starpu_desc->handletab[i],
                                         cblk->ownerid );
               continue;
            }

            /* If this is a fanin, let's submit the receive */
            if ( cblk->cblktype & CBLK_FANIN ) {
                starpu_mpi_data_migrate( datacode->solv_comm,
                                         rhsb->starpu_desc->handletab[i],
                                         datacode->clustnum );
            }
#endif
            starpu_cblk_ztrsmsp_backward( enums, sopalin_data, rhsb, cblk, prio );
        }
    }
    /* Forward like */
    else {
        cblknbr = (enums->mode == PastixSolvModeSchur) ? datacode->cblknbr : datacode->cblkschur;
        cblk    = datacode->cblktab;

        for ( i = 0; i < cblknbr; i++, cblk++ ) {
            prio = cblknbr - i;

#if defined(PASTIX_WITH_MPI)
            /* If this is a fanin, let's submit the send */
            if ( cblk->cblktype & CBLK_FANIN ) {
                starpu_mpi_data_migrate( datacode->solv_comm,
                                         rhsb->starpu_desc->handletab[i],
                                         cblk->ownerid );
                continue;
            }

            /* If this is a recv, let's locally sum the accumulation received */
            if ( cblk->cblktype & CBLK_RECV ) {
                starpu_mpi_data_migrate( datacode->solv_comm,
                                         rhsb->starpu_desc->handletab[i],
                                         datacode->clustnum );
                fcblk = datacode->cblktab + cblk->fblokptr->fcblknm;
                starpu_stask_blok_zadd_fwd_recv( sopalin_data, rhsb, cblk, fcblk, prio );
                continue;
            }
#endif
            starpu_cblk_ztrsmsp_forward( enums, sopalin_data, rhsb, cblk, prio );
        }
    }

    for ( i = 0; i < cblknbr; i++ ) {
        starpu_data_wont_use( rhsb->starpu_desc->handletab[i] );
    }
    (void)pastix_data;
}

/**
 *******************************************************************************
 *
 * @brief Apply the TRSM solve (StarPU version).
 *
 *******************************************************************************
 *
 * @param[in] pastix_data
 *          Provide informations about starpu and the schur solving mode.
 *
 * @param[in] enums
 *          Enums needed for the solve.
 *
 * @param[in] sopalin_data
 *          The data that provide the SolverMatrix structure from PaStiX, and
 *          descriptor of b (providing nrhs, b and ldb).
 *
 * @param[inout] rhsb
 *          The pointer to the rhs data structure that holds the vectors of the
 *          right hand side.
 *
 *******************************************************************************/
void
starpu_ztrsm( pastix_data_t      *pastix_data,
              const args_solve_t *enums,
              sopalin_data_t     *sopalin_data,
              pastix_rhs_t        rhsb )
{
    starpu_sparse_matrix_desc_t *sdesc = sopalin_data->solvmtx->starpu_desc;
    starpu_rhs_desc_t           *ddesc = rhsb->starpu_desc;

    /*
     * Start StarPU if not already started
     */
    if (pastix_data->starpu == NULL) {
        int argc = 0;
        pastix_starpu_init( pastix_data, &argc, NULL, NULL );
    }

    if ( sdesc == NULL ) {
        /* Create the sparse matrix descriptor */
        starpu_sparse_matrix_init( sopalin_data->solvmtx,
                                   PastixHermitian,
                                   pastix_data->inter_node_procnbr,
                                   pastix_data->inter_node_procnum,
                                   PastixComplex64 );
        sdesc = sopalin_data->solvmtx->starpu_desc;
    }

    if ( ddesc == NULL ) {
        /* Create the dense matrix descriptor */
        starpu_rhs_init( pastix_data->solvmatr, rhsb,
                         PastixComplex64,
                         pastix_data->inter_node_procnbr,
                         pastix_data->inter_node_procnum );
        ddesc = rhsb->starpu_desc;
    }

#if defined(STARPU_USE_FXT)
    if (pastix_data->iparm[IPARM_TRACE] & PastixTraceSolve) {
        starpu_fxt_start_profiling();
    }
#endif
    starpu_resume();
    starpu_ztrsm_sp1dplus( pastix_data, sopalin_data, rhsb, enums );

    starpu_sparse_matrix_getoncpu( sdesc );
    starpu_rhs_getoncpu( ddesc );
    starpu_task_wait_for_all();
#if defined(PASTIX_WITH_MPI)
    starpu_mpi_wait_for_all( pastix_data->pastix_comm );
    starpu_mpi_barrier(pastix_data->inter_node_comm);
#endif
    starpu_pause();
#if defined(STARPU_USE_FXT)
    if (pastix_data->iparm[IPARM_TRACE] & PastixTraceSolve) {
        starpu_fxt_stop_profiling();
    }
#endif

    return;
}

/**
 *@}
 */
