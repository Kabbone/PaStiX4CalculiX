/**
 *
 * @file bcsc_zinit.c
 *
 * @copyright 2004-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @author Pierre Ramet
 * @author Xavier Lacoste
 * @author Theophile Terraz
 * @author Tony Delarue
 * @author Vincent Bridonneau
 * @author Alycia Lisito
 * @date 2024-07-09
 *
 * @precisions normal z -> c d s
 *
 **/
#include "common.h"
#include "pastix/order.h"
#include <spm.h>
#include "blend/solver.h"
#include "bcsc/bcsc.h"
#include "bcsc_z.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static inline pastix_complex64_t
__fct_id( pastix_complex64_t val ) {
    return val;
}

static inline pastix_complex64_t
__fct_conj( pastix_complex64_t val ) {
#if defined(PRECISION_c) || defined(PRECISION_z)
    return conj( val );
#else
    /* This function should not be called in this case. */
    assert(0);
    return val;
#endif
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#if defined(PASTIX_WITH_MPI)
/**
 *******************************************************************************
 *
 * @brief Stores the data the current processor has to send to the other processors
 *        in the bcsc_comm structure.
 *
 * There are two cases:
 *
 *  - If the matrix is general: the full columns and rows of the blocks are stored
 *    in Lvalues and Uvalues.
 *    The local data of the current process which are in remote blocks after
 *    the permutation need be to sent to the owner process. The data is stored
 *    in sendA if it is sent for the column only, in sendAt if it is sent for
 *    the row only and in sendAAt if it is sent for the row and column.
 *
 *  - If the matrix is Symmetric or Hermitian: only the full columns of the blocks
 *    are stored in Lvalues (and Uvalues = Lvalues). Only half of the spm is
 *    stored lower (or upper) triangular half, therefore we need to duplicate
 *    the lower (or upper) data to fill the upper (or lower half of the matrix
 *    in the blocks.
 *    The local data of the current process which are in remote blocks after
 *    the permutation need be to sent to the owner process. The data is stored
 *    in sendA if it is sent for the lower (or upper) half or the column, in
 *    sendAt if it is sent for the upper (or lower) half of the column and in
 *    sendAAt if it is sent for both the lower and upper half of the column.
 *    The diagonal values are stored in sendA only.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering that needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] col2cblk
 *          The array of matching columns with cblk indexes.
 *
 * @param[inout] bcsc_comm
 *          The structure in which the data is stored.
 *
 *******************************************************************************/
void
bcsc_zstore_data( const spmatrix_t     *spm,
                  const pastix_order_t *ord,
                  const pastix_int_t   *col2cblk,
                  bcsc_handle_comm_t   *bcsc_comm )
{
    pastix_complex64_t *values_c;
    pastix_complex64_t *values    = (pastix_complex64_t*)(spm->values);
    pastix_int_t       *colptr    = spm->colptr;
    pastix_int_t       *rowptr    = spm->rowptr;
    pastix_int_t       *loc2glob  = spm->loc2glob;
    pastix_int_t       *dofs      = spm->dofs;
    pastix_int_t        dof       = spm->dof;
    bcsc_proc_comm_t   *data_comm = bcsc_comm->data_comm;
    pastix_int_t        clustnbr  = bcsc_comm->clustnbr;
    pastix_int_t        il, jl, ig, jg, igp, jgp, igpe, jgpe;
    pastix_int_t        ownerj, owneri, baseval;
    pastix_int_t        dofi, dofj, frow, lrow;
    bcsc_exch_comm_t   *data_sendA, *data_sendAt, *data_sendAAt;
    bcsc_data_amount_t *data_cntA, *data_cntAt, *data_cntAAt;
#if defined(PRECISION_z) || defined(PRECISION_c)
    int                 sym = (spm->mtxtype == SpmSymmetric) || (spm->mtxtype == SpmHermitian);
#else
    int                 sym = (spm->mtxtype == SpmSymmetric);
#endif

    /* Allocates the sending indexes and values buffers. */
    bcsc_allocate_buf( bcsc_comm, PastixTagMemSend );

    /*
     * Allocates and initialises the counters used to fill the values
     * and indexes buffers.
     */
    MALLOC_INTERN( data_cntA, 3 * clustnbr, bcsc_data_amount_t );
    memset( data_cntA, 0, 3 * clustnbr * sizeof(bcsc_data_amount_t) );
    data_cntAt  = data_cntA + clustnbr;
    data_cntAAt = data_cntA + clustnbr * 2;

    baseval = spm->baseval;

    /* Goes through the columns of spm. */
    for ( jl = 0; jl < spm->n; jl++, colptr++, loc2glob++ ) {
        jg   = *loc2glob - baseval;
        jgp  = ord->permtab[jg];
        jgpe = ( dof > 0 ) ? jgp * dof : dofs[ jg ] - baseval;
        dofj = ( dof > 0 ) ? dof : dofs[ jg+1 ] - dofs[ jg ];

        frow = colptr[0] - baseval;
        lrow = colptr[1] - baseval;
        assert( (lrow - frow) >= 0 );

        ownerj = col2cblk[ jgpe ];

        /* The column jp belongs to another process. */
        if ( ownerj < 0 ) {
            ownerj     = - ownerj - 1;
            data_comm  = bcsc_comm->data_comm + ownerj;
            data_sendA = &( data_comm->sendA );

            /* Goes through the rows of jl. */
            for ( il = frow; il < lrow; il++ ) {
                ig     = rowptr[il] - baseval;
                igp    = ord->permtab[ig];
                igpe   = ( dof > 0 ) ? igp * dof : dofs[ ig ] - baseval;
                dofi   = ( dof > 0 ) ? dof : dofs[ ig+1 ] - dofs[ ig ];
                owneri = col2cblk[ igpe ];

                /*
                 * The diagonal values (ip, jp) belong to the same process.
                 * They are sent to owneri in the sym case for A only.
                 */
                if ( sym && ( ig == jg ) ) {
                    data_sendA->idxbuf[data_cntA[ownerj].idxcnt  ] = igp;
                    data_sendA->idxbuf[data_cntA[ownerj].idxcnt+1] = jgp;
                    data_cntA[ownerj].idxcnt += 2;

                    values_c = ((pastix_complex64_t*)(data_sendA->valbuf)) + data_cntA[ownerj].valcnt;
                    memcpy( values_c, values, dofi * dofj * sizeof(pastix_complex64_t) );
                    data_cntA[ownerj].valcnt += dofi * dofj;

                    values += dofi * dofj;
                    continue;
                }

                /* The row ip belongs to another process. */
                if ( owneri < 0 ) {
                    owneri    = - owneri - 1;
                    data_comm = bcsc_comm->data_comm + owneri;

                    /*
                     * The diagonal values (ip, jp) belong to the same process.
                     * They are sent to owneri for AAt in the general cae.
                     */
                    if ( owneri == ownerj ) {
                        data_sendAAt = &( data_comm->sendAAt );

                        data_sendAAt->idxbuf[data_cntAAt[ownerj].idxcnt  ] = igp;
                        data_sendAAt->idxbuf[data_cntAAt[ownerj].idxcnt+1] = jgp;
                        data_cntAAt[ownerj].idxcnt += 2;

                        values_c = ((pastix_complex64_t*)(data_sendAAt->valbuf)) + data_cntAAt[ownerj].valcnt;
                        memcpy( values_c, values, dofi * dofj * sizeof(pastix_complex64_t) );
                        data_cntAAt[ownerj].valcnt += dofi * dofj;
                    }
                    /*
                     * The values (ip, jp) belong to different processes.
                     * They are sent to owneri for At and to ownerj for A.
                     */
                    else {
                        data_sendAt = &( data_comm->sendAt );

                        data_sendAt->idxbuf[data_cntAt[owneri].idxcnt  ] = igp;
                        data_sendAt->idxbuf[data_cntAt[owneri].idxcnt+1] = jgp;
                        data_cntAt[owneri].idxcnt += 2;

                        values_c = ((pastix_complex64_t*)(data_sendAt->valbuf)) + data_cntAt[owneri].valcnt;
                        memcpy( values_c, values, dofi * dofj * sizeof(pastix_complex64_t) );
                        data_cntAt[owneri].valcnt += dofi * dofj;

                        data_sendA->idxbuf[data_cntA[ownerj].idxcnt  ] = igp;
                        data_sendA->idxbuf[data_cntA[ownerj].idxcnt+1] = jgp;
                        data_cntA[ownerj].idxcnt += 2;

                        values_c = ((pastix_complex64_t*)(data_sendA->valbuf)) + data_cntA[ownerj].valcnt;
                        memcpy( values_c, values, dofi * dofj * sizeof(pastix_complex64_t) );
                        data_cntA[ownerj].valcnt += dofi * dofj;
                    }
                }
                /* The row ip is local. */
                else {
                    /*
                     * The values (ip, jp) belong to ownerj.
                     * They are sent to ownerj for A.
                     */
                    data_sendA->idxbuf[data_cntA[ownerj].idxcnt  ] = igp;
                    data_sendA->idxbuf[data_cntA[ownerj].idxcnt+1] = jgp;
                    data_cntA[ownerj].idxcnt += 2;

                    values_c = ((pastix_complex64_t*)(data_sendA->valbuf)) + data_cntA[ownerj].valcnt;
                    memcpy( values_c, values, dofi * dofj * sizeof(pastix_complex64_t) );
                    data_cntA[ownerj].valcnt += dofi * dofj;
                }
                values += dofi * dofj;
            }
        }
        /* The column jp is local. */
        else {
            /* Goes through the rows of j. */
            for ( il = frow; il < lrow; il++ ) {
                ig     = rowptr[il] - baseval;
                igp    = ord->permtab[ig];
                igpe   = ( dof > 0 ) ? igp * dof : dofs[ ig ] - baseval;
                dofi   = ( dof > 0 ) ? dof : dofs[ ig+1 ] - dofs[ ig ];
                owneri = col2cblk[ igpe ];

                /*
                 * The diagonal values (ip, jp) have already been
                 * added to globcol in the sym case.
                 */
                if ( sym && ( ig == jg ) ) {
                    values += dofi * dofj;
                    continue;
                }

                /* The row ip belongs to another process. */
                if ( owneri < 0 ) {
                    owneri    = - owneri - 1;
                    data_comm = bcsc_comm->data_comm + owneri;
                    data_sendAt = &( data_comm->sendAt );

                    /*
                     * The values (ip, jp) belong to owneri.
                     * They are sent to ownerj for At.
                     */
                    data_sendAt->idxbuf[data_cntAt[owneri].idxcnt  ] = igp;
                    data_sendAt->idxbuf[data_cntAt[owneri].idxcnt+1] = jgp;
                    data_cntAt[owneri].idxcnt += 2;

                    values_c = ((pastix_complex64_t*)(data_sendAt->valbuf)) + data_cntAt[owneri].valcnt;
                    memcpy( values_c, values, dofi * dofj * sizeof(pastix_complex64_t) );
                    data_cntAt[owneri].valcnt += dofi * dofj;
                }
                values += dofi * dofj;
            }
        }
    }

    memFree_null( data_cntA );
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Adds the remote data of A to the bcsc structure.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering that needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated.
 *
 * @param[in] values_buf
 *          The array containing the remote values.
 *
 * @param[in] idx_cnt
 *          The index for the begining of the indexes array buffer
 *          corresponding to the values_buf.
 *
 * @param[in] idx_size
 *          The number of indexes corresponding to the values_buf.
 *
 * @param[in] AAt
 *          TODO
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zhandle_recv_A( const spmatrix_t     *spm,
                     const pastix_order_t *ord,
                     const SolverMatrix   *solvmtx,
                     pastix_bcsc_t        *bcsc,
                     pastix_complex64_t   *values_buf,
                     pastix_int_t          idx_cnt,
                     pastix_int_t          idx_size,
                     pastix_int_t          AAt )
{
    bcsc_handle_comm_t *bcsc_comm = bcsc->bcsc_comm;
    pastix_int_t       *dofs = spm->dofs;
    pastix_int_t        dof  = spm->dof;
    SolverCblk         *cblk;
    pastix_int_t       *coltab;
    pastix_int_t        k, igp, jgp, ig, jg, igpe, jgpe;
    pastix_int_t        itercblk;
    pastix_int_t        idofi, idofj, dofi, dofj;
    pastix_int_t        colidx, rowidx, pos;
    pastix_int_t        clustnum = bcsc_comm->clustnum;
    pastix_complex64_t *Values   = bcsc->Lvalues;
    pastix_int_t       *indexes  = ( AAt == 0 ) ? bcsc_comm->data_comm[clustnum].sendA.idxbuf :
                                                  bcsc_comm->data_comm[clustnum].sendAAt.idxbuf;
    pastix_int_t        nbelt    = 0;

    /*
     * Goes through the received indexes in order to add the data
     * received.
     */
    indexes += idx_cnt;
    for ( k = 0; k < idx_size; k+=2, indexes+=2 ) {
        /* Gets received indexes jgp and igp. */
        igp = indexes[0];
        jgp = indexes[1];

        ig   = ord->peritab[igp];
        jg   = ord->peritab[jgp];
        igpe = (dof > 0) ? igp * dof : dofs[ ig ] - spm->baseval;
        jgpe = (dof > 0) ? jgp * dof : dofs[ jg ] - spm->baseval;
        dofi = (dof > 0) ? dof : dofs[ig+1] - dofs[ig];
        dofj = (dof > 0) ? dof : dofs[jg+1] - dofs[jg];

        itercblk = bcsc->col2cblk[ jgpe ];
        assert( itercblk >= 0 );

        /* Gets the block on which the data received will be added. */
        cblk   = solvmtx->cblktab + itercblk;
        coltab = bcsc->cscftab[cblk->bcscnum].coltab;

        /* Goes through the values at (igp, jgp). */
        colidx = jgpe - cblk->fcolnum;
        for ( idofj = 0; idofj < dofj; idofj++, colidx++ ) {
            rowidx = igpe;
            pos    = coltab[ colidx ];
            for ( idofi = 0; idofi < dofi; idofi++, rowidx++, pos++, values_buf++ ) {
                /* Adds the row igp. */
                assert( rowidx >= 0 );
                assert( rowidx < spm->gNexp );
                bcsc->rowtab[ pos ] = rowidx;
                /* Adds the data at row igp and column jgp. */
                Values[ pos ]       = *values_buf;
                nbelt++;
            }
            coltab[ colidx ] += dofi;
            assert( coltab[colidx] <= coltab[colidx+1] );
        }
    }

    return nbelt;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Adds the remote data of At to the bcsc structure.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering that needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] rowtab
 *          The row tab of the bcsc or the row tab associated to At.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated.
 *
 * @param[in] values_buf
 *          The array containing the remote values.
 *
 * @param[in] idx_cnt
 *          The index for the begining of the indexes array buffer
 *          corresponding to the values_buf.
 *
 * @param[in] idx_size
 *          The number of indexes corresponding to the values_buf.
 *
 * @param[in] AAt
 *          TODO
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zhandle_recv_At( const spmatrix_t     *spm,
                      const pastix_order_t *ord,
                      const SolverMatrix   *solvmtx,
                      pastix_int_t         *rowtab,
                      pastix_bcsc_t        *bcsc,
                      pastix_complex64_t   *values_buf,
                      pastix_int_t          idx_cnt,
                      pastix_int_t          idx_size,
                      pastix_int_t          AAt )
{
    bcsc_handle_comm_t *bcsc_comm = bcsc->bcsc_comm;
    pastix_int_t       *dofs = spm->dofs;
    pastix_int_t        dof  = spm->dof;
    SolverCblk         *cblk;
    pastix_int_t       *coltab;
    pastix_int_t        k, igp, jgp, ig, jg, igpe, jgpe;
    pastix_int_t        itercblk;
    pastix_int_t        idofi, idofj, dofi, dofj;
    pastix_int_t        colidx, rowidx, pos;
    pastix_int_t        clustnum = bcsc_comm->clustnum;
    pastix_int_t       *indexes  = ( AAt == 0 ) ? bcsc_comm->data_comm[clustnum].sendAt.idxbuf :
                                                  bcsc_comm->data_comm[clustnum].sendAAt.idxbuf;
    pastix_complex64_t *Values;
    pastix_complex64_t (*_bcsc_conj)(pastix_complex64_t) = __fct_id;
    pastix_int_t        nbelt     = 0;

    /* Gets the Uvalues in the general case and the Lvalues in the sym case. */
    if ( spm->mtxtype == SpmGeneral ) {
        _bcsc_conj = __fct_id;
        Values = (pastix_complex64_t*)(bcsc->Uvalues);
    }
    else {
        if( spm->mtxtype == SpmHermitian ) {
            _bcsc_conj = __fct_conj;
        }
        if( spm->mtxtype == SpmSymmetric ) {
            _bcsc_conj = __fct_id;
        }
        Values = (pastix_complex64_t*)(bcsc->Lvalues);
    }

    /*
     * Goes through the received indexes in order to add the data
     * received.
     */
    indexes += idx_cnt;
    for ( k = 0; k < idx_size; k+=2, indexes+=2 ) {
        /* Gets received indexes igp and jgp. */
        igp = indexes[0];
        jgp = indexes[1];

        ig   = ord->peritab[igp];
        jg   = ord->peritab[jgp];
        igpe = (dof > 0) ? igp * dof : dofs[ ig ] - spm->baseval;
        jgpe = (dof > 0) ? jgp * dof : dofs[ jg ] - spm->baseval;
        dofi = (dof > 0) ? dof : dofs[ig+1] - dofs[ig];
        dofj = (dof > 0) ? dof : dofs[jg+1] - dofs[jg];

        itercblk = bcsc->col2cblk[ igpe ];
        assert( itercblk >= 0 );

        /* Gets the block on which the data received will be added. */
        cblk   = solvmtx->cblktab + itercblk;
        coltab = bcsc->cscftab[cblk->bcscnum].coltab;

        /* Goes through the values at (igp, jgp). */
        for ( idofj = 0; idofj < dofj; idofj++ ) {
            rowidx = jgpe + idofj;
            colidx = igpe - cblk->fcolnum;
            for ( idofi = 0; idofi < dofi; idofi++, colidx++, values_buf++ ) {
                pos = coltab[ colidx ];
                /* Adds the row jgp. */
                assert( rowidx >= 0 );
                assert( rowidx < spm->gNexp );
                rowtab[ pos ] = rowidx;

                /* Adds the data at row jgp and column igp. */
                Values[ pos ] = _bcsc_conj( *values_buf );
                coltab[ colidx ] ++;
                assert( coltab[colidx] <= coltab[colidx+1] );
                nbelt++;
            }
        }
    }
    return nbelt;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Adds the remote data of At to the bcsc structure.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering that needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] rowtab
 *          The row tab of the bcsc or the row tab associated to At.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated.
 *
 * @param[in] values_buf
 *          The array containing the remote values.
 *
 * @param[in] idx_cnt
 *          The index for the begining of the indexes array buffer
 *          corresponding to the values_buf.
 *
 * @param[in] idx_size
 *          The number of indexes corresponding to the values_buf.
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zhandle_recv_AAt( const spmatrix_t     *spm,
                       const pastix_order_t *ord,
                       const SolverMatrix   *solvmtx,
                       pastix_int_t         *rowtab,
                       pastix_bcsc_t        *bcsc,
                       pastix_complex64_t   *values_buf,
                       pastix_int_t          idx_cnt,
                       pastix_int_t          idx_size )
{
    bcsc_handle_comm_t *bcsc_comm = bcsc->bcsc_comm;
    pastix_int_t       *dofs = spm->dofs;
    pastix_int_t        dof  = spm->dof;
    SolverCblk         *cblki, *cblkj;
    pastix_int_t       *coltabi, *coltabj;
    pastix_int_t        k, igp, jgp, ig, jg, igpe, jgpe;
    pastix_int_t        itercblki, itercblkj;
    pastix_int_t        idofi, idofj, dofi, dofj;
    pastix_int_t        colj, rowj, posj;
    pastix_int_t        coli, rowi, posi;
    pastix_int_t        clustnum = bcsc_comm->clustnum;
    pastix_int_t       *indexes  = bcsc_comm->data_comm[clustnum].sendAAt.idxbuf;
    pastix_complex64_t *ValuesA, *ValuesAt;
    pastix_complex64_t (*_bcsc_conj)(pastix_complex64_t) = __fct_id;
    pastix_int_t        nbelt     = 0;

    /* Gets the Uvalues in the general case and the Lvalues in the sym case. */
    if ( spm->mtxtype == SpmGeneral ) {
        _bcsc_conj = __fct_id;
        ValuesAt = (pastix_complex64_t*)(bcsc->Uvalues);
    }
    else {
        if( spm->mtxtype == SpmHermitian ) {
            _bcsc_conj = __fct_conj;
        }
        if( spm->mtxtype == SpmSymmetric ) {
            _bcsc_conj = __fct_id;
        }
        ValuesAt = (pastix_complex64_t*)(bcsc->Lvalues);
    }
    ValuesA = (pastix_complex64_t*)(bcsc->Lvalues);

    /*
     * Goes through the received indexes in order to add the data
     * received.
     */
    indexes += idx_cnt;
    for ( k = 0; k < idx_size; k+=2, indexes+=2 ) {
        /* Gets received indexes igp and jgp. */
        igp = indexes[0];
        jgp = indexes[1];

        ig   = ord->peritab[igp];
        jg   = ord->peritab[jgp];
        igpe = (dof > 0) ? igp * dof : dofs[ ig ] - spm->baseval;
        jgpe = (dof > 0) ? jgp * dof : dofs[ jg ] - spm->baseval;
        dofi = (dof > 0) ? dof : dofs[ig+1] - dofs[ig];
        dofj = (dof > 0) ? dof : dofs[jg+1] - dofs[jg];

        itercblki = bcsc->col2cblk[ igpe ];
        itercblkj = bcsc->col2cblk[ jgpe ];
        assert( itercblki >= 0 );
        assert( itercblkj >= 0 );

        /* Gets the block on which the data received will be added. */
        cblki   = solvmtx->cblktab + itercblki;
        cblkj   = solvmtx->cblktab + itercblkj;
        coltabi = bcsc->cscftab[cblki->bcscnum].coltab;
        coltabj = bcsc->cscftab[cblkj->bcscnum].coltab;

        /* Goes through the values at (igp, jgp). */
        colj = jgpe - cblkj->fcolnum;
        for ( idofj = 0; idofj < dofj; idofj++ ) {
            rowj = igpe;
            posj = coltabj[ colj ];
            rowi = jgpe + idofj;
            coli = igpe - cblki->fcolnum;
            for ( idofi = 0; idofi < dofi; idofi++, coli++, rowj++, posj++, values_buf++ ) {
                posi = coltabi[ coli ];
                /* Adds the row jgp /igp. */
                assert( rowi >= 0 );
                assert( rowj >= 0 );
                assert( rowi < spm->gNexp );
                assert( rowj < spm->gNexp );

                rowtab[ posi ]       = rowi;
                bcsc->rowtab[ posj ] = rowj;

                /* Adds the data at row jgp and column igp. */
                ValuesAt[ posi ] = _bcsc_conj( *values_buf );
                ValuesA [ posj ] = *values_buf;
                coltabi[ coli ] ++;
                assert( coltabi[coli] <= coltabi[coli+1] );
                nbelt++;
            }
            coltabj[ colj ] += dofi;
            assert( coltabj[colj] <= coltabj[colj+1] );
        }
    }

    return nbelt;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Exchanges and stores the remote values of A.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering which needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated with the remote values of A.
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zexchange_values_A( const spmatrix_t     *spm,
                         const pastix_order_t *ord,
                         const SolverMatrix   *solvmtx,
                         pastix_bcsc_t        *bcsc )
{
    bcsc_handle_comm_t *bcsc_comm   = bcsc->bcsc_comm;
    pastix_int_t        clustnbr    = bcsc_comm->clustnbr;
    pastix_int_t        clustnum    = bcsc_comm->clustnum;
    bcsc_proc_comm_t   *data_comm   = bcsc_comm->data_comm;
    bcsc_proc_comm_t   *data_send   = NULL;
    bcsc_proc_comm_t   *data_recv   = NULL;
    pastix_complex64_t *val_buf     = NULL;
    pastix_int_t        idx_size    = 0;
    pastix_int_t        counter_req = 0;
    pastix_int_t        nbelt_recv  = 0;
    pastix_int_t        cnt         = 0;
    pastix_int_t        idx_cnt[clustnbr];
    MPI_Status          statuses[clustnbr-1];
    MPI_Request         requests[clustnbr-1];
    bcsc_data_amount_t *sends, *recvs;
    pastix_int_t        c_send, c_recv, k;

    /* Allocates the receiving indexes and values buffers. */
    if ( bcsc_comm->max_idx != 0 ) {
        MALLOC_INTERN( val_buf, bcsc_comm->max_val, pastix_complex64_t );
    }

    for ( k = 0; k < clustnbr; k++ ) {
        if ( k == clustnum ) {
            idx_cnt[k] = 0;
            continue;
        }
        idx_cnt[ k ] = cnt;
        cnt += data_comm[k].recvA.idxcnt;
    }

    /* Sends the values. */
    c_send = (clustnum+1) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        data_send = data_comm + c_send;
        sends     = &( data_send->sendA.size );
        if ( c_send == clustnum ) {
            continue;
        }

        /* Posts the emissions of the values. */
        if ( sends->valcnt != 0 ) {
            MPI_Isend( data_send->sendA.valbuf, sends->valcnt, PASTIX_MPI_COMPLEX64,
                       c_send, PastixTagValuesA, bcsc_comm->comm, &requests[counter_req++] );
        }
        c_send = (c_send+1) % clustnbr;
    }

    /* Receives the values. */
    c_recv = (clustnum-1+clustnbr) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        data_recv = data_comm + c_recv;
        recvs     = &( data_recv->recvA );
        if ( c_recv == clustnum ) {
            continue;
        }

        /* Posts the receptions of the values. */
        if ( recvs->valcnt != 0 ) {
            MPI_Recv( val_buf, recvs->valcnt, PASTIX_MPI_COMPLEX64,
                      c_recv, PastixTagValuesA, bcsc_comm->comm, MPI_STATUS_IGNORE );
            idx_size = recvs->idxcnt;
            nbelt_recv += bcsc_zhandle_recv_A( spm, ord, solvmtx, bcsc,
                                               val_buf, idx_cnt[c_recv], idx_size, 0 );
        }
        c_recv = (c_recv-1+clustnbr) % clustnbr;
    }

    MPI_Waitall( counter_req, requests, statuses );
    free( val_buf );
    assert( nbelt_recv == bcsc_comm->data_comm[clustnum].recvA.valcnt );
    bcsc_free_buf( bcsc_comm, PastixTagMemSendValA );
    bcsc_free_buf( bcsc_comm, PastixTagMemRecvIdxA );
    return nbelt_recv;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Exchanges and stores the remote values of At.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering which needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] rowtab
 *          The row tab of the bcsc or the row tab associated to At.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated with the remote values of At.
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zexchange_values_At( const spmatrix_t     *spm,
                          const pastix_order_t *ord,
                          const SolverMatrix   *solvmtx,
                          pastix_int_t         *rowtab,
                          pastix_bcsc_t        *bcsc )
{
    bcsc_handle_comm_t *bcsc_comm   = bcsc->bcsc_comm;
    pastix_int_t        clustnbr    = bcsc_comm->clustnbr;
    pastix_int_t        clustnum    = bcsc_comm->clustnum;
    bcsc_proc_comm_t   *data_comm   = bcsc_comm->data_comm;
    bcsc_proc_comm_t   *data_send   = NULL;
    bcsc_proc_comm_t   *data_recv   = NULL;
    pastix_complex64_t *val_buf     = NULL;
    pastix_int_t        idx_size    = 0;
    pastix_int_t        counter_req = 0;
    pastix_int_t        nbelt_recv  = 0;
    pastix_int_t        cnt         = 0;
    pastix_int_t        idx_cnt[clustnbr];
    MPI_Status          statuses[clustnbr-1];
    MPI_Request         requests[clustnbr-1];
    bcsc_data_amount_t *sends, *recvs;
    pastix_int_t        c_send, c_recv, k;

    /* Allocates the receiving indexes and values buffers. */
    if ( bcsc_comm->max_idx != 0 ) {
        MALLOC_INTERN( val_buf, bcsc_comm->max_val, pastix_complex64_t );
    }

    for ( k = 0; k < clustnbr; k++ ) {
        if ( k == clustnum ) {
            idx_cnt[k] = 0;
            continue;
        }
        idx_cnt[ k ] = cnt;
        cnt += data_comm[k].recvAt.idxcnt;
    }

    /* Sends the values. */
    c_send = (clustnum+1) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        data_send = data_comm + c_send;
        sends     = &( data_send->sendAt.size );
        if ( c_send == clustnum ) {
            continue;
        }

        /* Posts the emissions of the values. */
        if (  sends->valcnt != 0 ) {
            MPI_Isend( data_send->sendAt.valbuf, sends->valcnt, PASTIX_MPI_COMPLEX64,
                       c_send, PastixTagValuesAt, bcsc_comm->comm, &requests[counter_req++] );
        }
        c_send = (c_send+1) % clustnbr;
    }

    /* Receives the values. */
    c_recv = (clustnum-1+clustnbr) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        data_recv = data_comm + c_recv;
        recvs     = &( data_recv->recvAt );
        if ( c_recv == clustnum ) {
            continue;
        }

        /* Posts the receptions of the values. */
        if ( recvs->valcnt != 0 ) {
            MPI_Recv( val_buf, recvs->valcnt, PASTIX_MPI_COMPLEX64,
                      c_recv, PastixTagValuesAt, bcsc_comm->comm, MPI_STATUS_IGNORE );
            idx_size = recvs->idxcnt;
            nbelt_recv += bcsc_zhandle_recv_At( spm, ord, solvmtx, rowtab, bcsc,
                                                val_buf, idx_cnt[c_recv], idx_size, 0 );
        }
        c_recv = (c_recv-1+clustnbr) % clustnbr;
    }

    MPI_Waitall( counter_req, requests, statuses );
    free( val_buf );
    assert( nbelt_recv == bcsc_comm->data_comm[clustnum].recvAt.valcnt );
    bcsc_free_buf( bcsc_comm, PastixTagMemSendValAt );
    bcsc_free_buf( bcsc_comm, PastixTagMemRecvIdxAt );
    return nbelt_recv;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Exchanges and stores the remote values of A.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering which needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] rowtab
 *          The row tab of the bcsc or the row tab associated to At.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated with the remote values of A.
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zexchange_values_AAt( const spmatrix_t     *spm,
                           const pastix_order_t *ord,
                           const SolverMatrix   *solvmtx,
                           pastix_int_t         *rowtabAt,
                           pastix_bcsc_t        *bcsc )
{
    bcsc_handle_comm_t *bcsc_comm   = bcsc->bcsc_comm;
    pastix_int_t        clustnbr    = bcsc_comm->clustnbr;
    pastix_int_t        clustnum    = bcsc_comm->clustnum;
    bcsc_proc_comm_t   *data_comm   = bcsc_comm->data_comm;
    bcsc_proc_comm_t   *data_send   = NULL;
    bcsc_proc_comm_t   *data_recv   = NULL;
    pastix_complex64_t *val_buf     = NULL;
    pastix_int_t        idx_size    = 0;
    pastix_int_t        counter_req = 0;
    pastix_int_t        nbelt_recv  = 0;
    pastix_int_t        cnt         = 0;
    pastix_int_t        idx_cnt[clustnbr];
    MPI_Status          statuses[clustnbr-1];
    MPI_Request         requests[clustnbr-1];
    bcsc_data_amount_t *sends;
    bcsc_exch_comm_t   *recvs;
    pastix_int_t        c_send, c_recv, k;

    /* Allocates the receiving indexes and values buffers. */
    if ( bcsc_comm->max_idx != 0 ) {
        if ( spm->mtxtype != SpmGeneral ) {
            MALLOC_INTERN( val_buf, bcsc_comm->max_val, pastix_complex64_t );
        }
        else {
            if ( rowtabAt == bcsc->rowtab ) {
                bcsc_allocate_buf( bcsc_comm, PastixTagMemRecvValAAt);
            }
        }
    }

    for ( k = 0; k < clustnbr; k++ ) {
        if ( k == clustnum ) {
            idx_cnt[k] = 0;
            continue;
        }
        idx_cnt[ k ] = cnt;
        cnt += data_comm[k].recvAAt.size.idxcnt;
    }

    /* Sends the values. */
    c_send = (clustnum+1) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        if ( rowtabAt != bcsc->rowtab ) {
            break;
        }
        data_send = data_comm + c_send;
        sends     = &( data_send->sendAAt.size );
        if ( c_send == clustnum ) {
            continue;
        }

        /* Posts the emissions of the values. */
        if ( sends->valcnt != 0 ) {
            MPI_Isend( data_send->sendAAt.valbuf, sends->valcnt, PASTIX_MPI_COMPLEX64,
                       c_send, PastixTagValuesAAt, bcsc_comm->comm, &requests[counter_req++] );
        }
        c_send = (c_send+1) % clustnbr;
    }

    /* Receives the values. */
    c_recv = (clustnum-1+clustnbr) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        data_recv = data_comm + c_recv;
        recvs     = &( data_recv->recvAAt );
        if ( c_recv == clustnum ) {
            continue;
        }

        /* Posts the receptions of the values. */
        if ( recvs->size.valcnt != 0 ) {
            if ( spm->mtxtype == SpmGeneral ) {
                val_buf = recvs->valbuf;
            }
            if ( rowtabAt == bcsc->rowtab ) {
                MPI_Recv( val_buf, recvs->size.valcnt, PASTIX_MPI_COMPLEX64,
                          c_recv, PastixTagValuesAAt, bcsc_comm->comm, MPI_STATUS_IGNORE );
            }
            idx_size = recvs->size.idxcnt;

            if ( spm->mtxtype != SpmGeneral ) {
                nbelt_recv += bcsc_zhandle_recv_AAt( spm, ord, solvmtx, rowtabAt, bcsc,
                                                     val_buf, idx_cnt[c_recv], idx_size );
            }
            else {
                if ( rowtabAt == bcsc->rowtab ) {
                    nbelt_recv += bcsc_zhandle_recv_A( spm, ord, solvmtx, bcsc, val_buf,
                                                       idx_cnt[c_recv], idx_size, 1 );
                }
                else {
                    nbelt_recv += bcsc_zhandle_recv_At( spm, ord, solvmtx, rowtabAt, bcsc,
                                                        val_buf, idx_cnt[c_recv], idx_size, 1 );
                }
            }
        }
        c_recv = (c_recv-1+clustnbr) % clustnbr;
    }

    if ( rowtabAt == bcsc->rowtab ) {
        MPI_Waitall( counter_req, requests, statuses );
    }
    if ( spm->mtxtype != SpmGeneral ) {
        free( val_buf );
    }
    assert( nbelt_recv == bcsc_comm->data_comm[clustnum].recvAAt.size.valcnt );
    if ( ( spm->mtxtype != SpmGeneral ) || ( rowtabAt != bcsc->rowtab ) ) {
        bcsc_free_buf( bcsc_comm, PastixTagMemRecvIdxAAt );
        bcsc_free_buf( bcsc_comm, PastixTagMemSendValAAt );
    }
    if ( ( spm->mtxtype == SpmGeneral ) && ( rowtabAt != bcsc->rowtab ) ) {
        bcsc_free_buf( bcsc_comm, PastixTagMemRecvAAt );
    }
    return nbelt_recv;
}
#endif

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Initializes the A values of the block csc stored in the given spm.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering which needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated.
 *
 *******************************************************************************
 *
 * @retval TODO
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zinit_A( const spmatrix_t     *spm,
              const pastix_order_t *ord,
              const SolverMatrix   *solvmtx,
              pastix_bcsc_t        *bcsc )
{
    pastix_complex64_t *values   = (pastix_complex64_t*)(spm->values);
    pastix_complex64_t *Lvalues  = (pastix_complex64_t*)(bcsc->Lvalues);
    pastix_int_t       *loc2glob = spm->loc2glob;
    pastix_int_t       *colptr   = spm->colptr;
    pastix_int_t       *rowptr   = spm->rowptr;
    pastix_int_t       *dofs     = spm->dofs;
    pastix_int_t        dof      = spm->dof;
    pastix_int_t        nbelt    = 0;
    SolverCblk         *cblk;
    pastix_int_t       *coltab;
    pastix_int_t        k, j, ig, jg, igp, jgp, igpe, jgpe;
    pastix_int_t        itercblk, baseval;
    pastix_int_t        idofi, idofj, dofi, dofj;
    pastix_int_t        colidx, rowidx, pos;

    baseval = spm->baseval;
    /*
     * Initializes the values of the matrix A in the blocked csc format. This
     * applies the permutation to the values array.
     *
     * Goes through the columns of the spm.
     * j is the initial local column index.
     * jg is the initial global column index.
     * jgpis the "new" jg index -> the one resulting from the permutation.
     */
    for ( j = 0; j < spm->n; j++, colptr++, loc2glob++ ) {
        jg   = ( spm->loc2glob == NULL ) ? j : *loc2glob - baseval;
        jgp  = ord->permtab[jg];
        jgpe = ( dof > 0 ) ? jgp * dof : dofs[ jg ] - baseval;
        dofj = ( dof > 0 ) ? dof : dofs[ jg+1 ] - dofs[ jg ];

        itercblk = bcsc->col2cblk[ jgpe ];

        /*
         * If MPI is used in shared memory, the block can belong to another
         * processor, in this case the column is skigped.
         */
        /* The block of the column jgpbelongs to the current processor. */
        if ( itercblk >= 0 ) {
            /* Gets the block in which the data will be added. */
            cblk   = solvmtx->cblktab + itercblk;
            coltab = bcsc->cscftab[cblk->bcscnum].coltab;
            /*
             * Goes through the row of the column j.
             * ig is the initial global row index (for the row index local = global).
             * igpis the "new" ig index -> the one resulting from the permutation.
             */
            for ( k = colptr[0]; k < colptr[1]; k++, rowptr++ ) {
                ig   = *rowptr - baseval;
                igp  = ord->permtab[ig];
                igpe = ( dof > 0 ) ? igp * dof : dofs[ ig ] - baseval;
                dofi = ( dof > 0 ) ? dof : dofs[ ig+1 ] - dofs[ ig ];

                /* Copies the values from element (ig, jg) to position (igpe, jgpe) into expanded bcsc. */
                colidx = jgpe - cblk->fcolnum;
                for ( idofj = 0; idofj < dofj; idofj++, colidx++ ) {
                    rowidx = igpe;
                    pos    = coltab[ colidx ];
                    for ( idofi = 0; idofi < dofi; idofi++, rowidx++, pos++, values++ ) {
                        assert( rowidx >= 0 );
                        assert( rowidx < spm->gNexp );
                        bcsc->rowtab[ pos ] = rowidx;
                        Lvalues[ pos ]      = *values;
                        nbelt++;
                    }
                    coltab[ colidx ] += dofi;
                    assert( coltab[ colidx ] <= coltab[ colidx+1 ] );
                }
            }
        }
        /* The block of the column jgpbelongs to another processor. */
        else {
            for ( k = colptr[0]; k < colptr[1]; k++, rowptr++ ) {
                ig   = *rowptr - baseval;
                dofi = ( dof > 0 ) ? dof : dofs[ ig+1 ] - dofs[ ig ];

                values += dofi * dofj;
            }
        }
    }

    return nbelt;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Initializes the At values in the block cscstored in the given spm.
 *
 * This routine initializes either :
 *      The symmetric upper part (L^t)
 *      The hermitian upper part (L^h)
 *      The transpose part of A (A^t -> U)
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering which needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] rowtab
 *          The row tab of the bcsc or the row tab associated to At.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc fields are updated.
 *
 *******************************************************************************
 *
 * @retval TODO
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_zinit_At( const spmatrix_t     *spm,
               const pastix_order_t *ord,
               const SolverMatrix   *solvmtx,
               pastix_int_t         *rowtab,
               pastix_bcsc_t        *bcsc )
{
    pastix_complex64_t *values  = (pastix_complex64_t*)(spm->values);
    pastix_complex64_t *Uvalues;
    pastix_int_t       *loc2glob = spm->loc2glob;
    pastix_int_t       *colptr   = spm->colptr;
    pastix_int_t       *rowptr   = spm->rowptr;
    pastix_int_t       *dofs     = spm->dofs;
    pastix_int_t        dof      = spm->dof;
    pastix_int_t        nbelt    = 0;
    SolverCblk         *cblk;
    pastix_int_t       *coltab;
    pastix_int_t        k, j, ig, jg, igp, jgp, igpe, jgpe;
    pastix_int_t        itercblk, baseval;
    pastix_int_t        idofi, idofj, dofi, dofj;
    pastix_int_t        colidx, rowidx, pos;
    pastix_complex64_t (*_bcsc_conj)(pastix_complex64_t) = NULL;

    /* We're working on U. */
    if ( spm->mtxtype == SpmGeneral ) {
        _bcsc_conj = __fct_id;
        Uvalues = (pastix_complex64_t*)(bcsc->Uvalues);
    }
    /* L^[t|h] part of the matrix. */
    else {
        /*
         * precision_generator/sub.py will change SpmHermitian to SpmSymmetric
         * Don't use else or else if.
         */
        if( spm->mtxtype == SpmHermitian ) {
            _bcsc_conj = __fct_conj;
        }
        if( spm->mtxtype == SpmSymmetric ) {
            _bcsc_conj = __fct_id;
        }
        Uvalues = (pastix_complex64_t*)(bcsc->Lvalues);
    }
    assert( _bcsc_conj != NULL );

    baseval = spm->baseval;
    /*
     * Initializes the values of the matrix A^t in the blocked csc format. This
     * applies the permutation to the values array.
     *
     * Goes through the columns of the spm.
     * j is the initial local column index.
     * jg is the initial global column index.
     * jgpis the "new" jg index -> the one resulting from the permutation.
     */
    for ( j = 0; j < spm->n; j++, colptr++, loc2glob++ ) {
        jg   = ( spm->loc2glob == NULL ) ? j : *loc2glob - baseval;
        jgp  = ord->permtab[jg];
        jgpe = ( dof > 0 ) ? jgp * dof : dofs[ jg ] - baseval;
        dofj = ( dof > 0 ) ? dof : dofs[ jg+1 ] - dofs[ jg ];

        /*
         * Goes through the row of the column j.
         * ig is the initial global row index (for the row index local = global).
         * igpis the "new" ig index -> the one resulting from the permutation.
         */
        for ( k = colptr[0]; k < colptr[1]; k++, rowptr++ ) {
            ig   = *rowptr - baseval;
            igp  = ord->permtab[ig];
            igpe = ( dof > 0 ) ? igp * dof : dofs[ ig ] - baseval;
            dofi = ( dof > 0 ) ? dof : dofs[ ig+1 ] - dofs[ ig ];

            /* Diagonal block of a symmetric matrix. */
            if ( ( ig == jg ) && ( spm->mtxtype != SpmGeneral ) ) {
                values += dofi * dofj;
                continue;
            }
            itercblk = bcsc->col2cblk[ igpe ];

            /*
             * If MPI is used in shared memory, the block can belong to another
             * processor, in this case the row is skigped.
             */
            /* The block of the row igpbelongs to the current processor. */
            if ( itercblk >= 0 ) {
                /* Gets the block in which the data will be added. */
                cblk   = solvmtx->cblktab + itercblk;
                coltab = bcsc->cscftab[cblk->bcscnum].coltab;

                /* Copies the values from element (ig, jg) to position (igpe, jgpe) into expanded bcsc. */
                for ( idofj = 0; idofj < dofj; idofj++ ) {
                    rowidx = jgpe + idofj;
                    colidx = igpe - cblk->fcolnum;
                    for ( idofi = 0; idofi < dofi; idofi++, colidx++, values++ ) {
                        pos = coltab[ colidx ];

                        /* Copies the index/value */
                        assert( rowidx >= 0 );
                        assert( rowidx < spm->gNexp );
                        rowtab[ pos ]  = rowidx;
                        Uvalues[ pos ] = _bcsc_conj( *values );

                        coltab[ colidx ]++;
                        nbelt++;
                    }
                }
            }
            /* The block of the row igpbelongs to another processor. */
            else {
                values += dofi * dofj;
                continue;
            }
        }
    }
    return nbelt;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Sorts the block csc subarray associated to each column block.
 *
 *******************************************************************************
 *
 * @param[in] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *
 * @param[in] rowtab
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] valtab
 *          The ordering that needs to be applied on the spm to generate the
 *          block csc.
 *
 *******************************************************************************/
static inline void
bcsc_zsort( const pastix_bcsc_t *bcsc,
            pastix_int_t        *rowtab,
            pastix_complex64_t  *valtab )
{
    bcsc_cblk_t *block;
    pastix_int_t block_num, col, len_col_block, block_nbr, col_nbr;
    void        *sortptr[2];

    block_nbr = bcsc->cscfnbr;
    block     = bcsc->cscftab;
    for ( block_num = 0; block_num < block_nbr; block_num++, block++ ) {

        col_nbr = block->colnbr;
        for ( col = 0; col < col_nbr; col++ ) {

            sortptr[0] = (void*)(rowtab + block->coltab[col]);
            sortptr[1] = (void*)(valtab + block->coltab[col]);

            len_col_block = block->coltab[col+1] - block->coltab[col];
#if !defined(NDEBUG)
            {
                pastix_int_t gN = bcsc->gN;
                pastix_int_t i;

                for ( i = 0; i < len_col_block; i++ ) {
                    assert( rowtab[ block->coltab[col] + i ] >= 0 );
                    assert( rowtab[ block->coltab[col] + i ] < gN );
                }
            }
#endif

            z_qsortIntFloatAsc( sortptr, len_col_block );
        }
    }
}

/**
 *******************************************************************************
 *
 * @brief Initializes a centralize pastix_complex64_t block csc.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] ord
 *          The ordering that needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure that describe the data distribution.
 *
 * @param[in] initAt
 *          A flag to enable/disable the initialization of A'.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc stores the input spm with the permutation applied
 *          and grouped according to the distribution described in solvmtx.
 *
 * @param[in] valuesize
 *         The number of non zero unknowns in the matrix.
 *
 *******************************************************************************/
void
bcsc_zinit( const spmatrix_t     *spm,
            const pastix_order_t *ord,
            const SolverMatrix   *solvmtx,
            int                   initAt,
            pastix_bcsc_t        *bcsc,
            pastix_int_t          valuesize )
{
    pastix_int_t nbelt = 0;
#if defined(PASTIX_WITH_MPI)
    pastix_int_t nbelt_recv = 0;
    bcsc_handle_comm_t *bcsc_comm = bcsc->bcsc_comm;
#endif

    /* Exchanges the data if the spm is distributed and adds the received data of A. */
#if defined(PASTIX_WITH_MPI)
    if ( bcsc_comm != NULL ) {
        nbelt_recv  = bcsc_zexchange_values_A( spm, ord, solvmtx, bcsc );
        nbelt_recv += bcsc_zexchange_values_AAt( spm, ord, solvmtx, bcsc->rowtab, bcsc );
        nbelt += nbelt_recv;
    }
#endif

    /* Initializes the blocked structure of the matrix A. */
    nbelt += bcsc_zinit_A( spm, ord, solvmtx, bcsc );

    /* Initializes the blocked structure of the matrix At if the matrix is not general. */
    if ( spm->mtxtype != SpmGeneral ) {
        /* Exchanges the data if the spm is distributed and adds the received data of A. */
#if defined(PASTIX_WITH_MPI)
        if ( bcsc_comm != NULL ) {
            nbelt_recv = bcsc_zexchange_values_At(  spm, ord, solvmtx, bcsc->rowtab, bcsc );
            nbelt += nbelt_recv;
        }
#endif
        nbelt += bcsc_zinit_At( spm, ord, solvmtx, bcsc->rowtab, bcsc );
    }

    /* Restores the correct coltab arrays. */
    bcsc_restore_coltab( bcsc );

    /* Sorts the csc. */
    bcsc_zsort( bcsc, bcsc->rowtab, bcsc->Lvalues );

    /* Initializes the blocked structure of the matrix At if the matrix is general. */
    if ( spm->mtxtype == SpmGeneral ) {
        /* If only the refinement is performed A^t is not required. */
        if ( initAt ) {
            pastix_int_t i;
            MALLOC_INTERN( bcsc->Uvalues,  valuesize, pastix_complex64_t );
            MALLOC_INTERN( bcsc->Urowtab,  valuesize, pastix_int_t );

            for (i=0; i<valuesize; i++) {
                bcsc->Urowtab[i] = -1;
            }
            nbelt = bcsc_zinit_At( spm, ord, solvmtx, bcsc->Urowtab, bcsc );
#if defined(PASTIX_WITH_MPI)
            if ( bcsc_comm != NULL ) {
                nbelt_recv  = bcsc_zexchange_values_At(  spm, ord, solvmtx, bcsc->Urowtab, bcsc );
                nbelt_recv += bcsc_zexchange_values_AAt( spm, ord, solvmtx, bcsc->Urowtab, bcsc );
                nbelt += nbelt_recv;
            }
#endif
            /* Restores the correct coltab arrays */
            bcsc_restore_coltab( bcsc );

            /* Sorts the transposed csc. Urowtab is kept as persistent row-index
             * array for Uvalues (used by bcsc_build_spm2bcsc and bcscUpdateValues). */
            bcsc_zsort( bcsc, bcsc->Urowtab, bcsc->Uvalues );
        }
    }
    /* In the case of SpmHermitian, conj is applied when used to save memory space */
    else {
        bcsc->Uvalues = bcsc->Lvalues;
    }

    (void) nbelt;
#if defined(PASTIX_WITH_MPI)
    (void) nbelt_recv;
#endif
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Updates Lvalues / Uvalues using precomputed scatter maps.
 *
 * O(nnz) values-only update for the case where the sparsity pattern is
 * unchanged since the last bcscInit() call.  Uses the spm2bcsc_L / spm2bcsc_Lt
 * / spm2bcsc_U index maps built by bcsc_build_spm2bcsc() to scatter each
 * spm->values[k] directly to its final sorted position in Lvalues or Uvalues.
 * No fill traversal, no coltab advancement, and no sort are performed.
 *
 * @param[in] spm
 *          The sparse matrix with updated values (same sparsity as before).
 *
 * @param[in] ord
 *          Unused (kept for API symmetry with bcsc_zinit).
 *
 * @param[in] solvmtx
 *          Unused (kept for API symmetry with bcsc_zinit).
 *
 * @param[in] initAt
 *          Non-zero if Uvalues must be updated (general LU case).
 *
 * @param[inout] bcsc
 *          On entry, a fully initialised bcsc with valid spm2bcsc_* maps.
 *          On exit, Lvalues and Uvalues contain the new values.
 *
 * @param[in] valuesize
 *          Unused (kept for API symmetry with bcsc_zinit).
 *
 *******************************************************************************/
void
bcsc_zupdatevalues( const spmatrix_t     *spm,
                    const pastix_order_t *ord,
                    const SolverMatrix   *solvmtx,
                    int                   initAt,
                    pastix_bcsc_t        *bcsc,
                    pastix_int_t          valuesize )
{
    pastix_complex64_t *values  = (pastix_complex64_t *)( spm->values );
    pastix_complex64_t *Lvalues = (pastix_complex64_t *)( bcsc->Lvalues );
    pastix_int_t       *spm2L   = bcsc->spm2bcsc_L;
    pastix_int_t        nnzexp  = spm->nnzexp;
    pastix_int_t        k;
#if defined(PASTIX_WITH_MPI)
    bcsc_handle_comm_t *bcsc_comm = bcsc->bcsc_comm;
#endif

    /* MPI: re-exchange remote values using the already-known index maps. */
#if defined(PASTIX_WITH_MPI)
    if ( bcsc_comm != NULL ) {
        bcsc_zexchange_values_A(   spm, ord, solvmtx, bcsc );
        bcsc_zexchange_values_AAt( spm, ord, solvmtx, bcsc->rowtab, bcsc );
        if ( spm->mtxtype != SpmGeneral ) {
            bcsc_zexchange_values_At( spm, ord, solvmtx, bcsc->rowtab, bcsc );
        }
    }
#endif

    /* Direct scatter: overwrite Lvalues at their pre-sorted positions. */
    for ( k = 0; k < nnzexp; k++ ) {
        if ( spm2L[k] >= 0 ) {
            Lvalues[ spm2L[k] ] = values[k];
        }
    }

    /* Symmetric / hermitian: scatter the transposed (conjugated) contribution. */
    if ( spm->mtxtype != SpmGeneral && bcsc->spm2bcsc_Lt != NULL ) {
        pastix_complex64_t (*_bcsc_conj)(pastix_complex64_t) = __fct_id;
        pastix_int_t *spm2Lt = bcsc->spm2bcsc_Lt;
        if ( spm->mtxtype == SpmHermitian ) { _bcsc_conj = __fct_conj; }
        if ( spm->mtxtype == SpmSymmetric ) { _bcsc_conj = __fct_id;   }
        for ( k = 0; k < nnzexp; k++ ) {
            if ( spm2Lt[k] >= 0 ) {
                Lvalues[ spm2Lt[k] ] = _bcsc_conj( values[k] );
            }
        }
    }

    /* General LU: scatter the transposed contribution into Uvalues. */
    if ( spm->mtxtype == SpmGeneral && initAt &&
         bcsc->Uvalues != NULL && bcsc->spm2bcsc_U != NULL ) {
        pastix_complex64_t *Uvalues = (pastix_complex64_t *)( bcsc->Uvalues );
        pastix_int_t       *spm2U   = bcsc->spm2bcsc_U;
#if defined(PASTIX_WITH_MPI)
        if ( bcsc_comm != NULL ) {
            bcsc_zexchange_values_At(  spm, ord, solvmtx, bcsc->Urowtab, bcsc );
            bcsc_zexchange_values_AAt( spm, ord, solvmtx, bcsc->Urowtab, bcsc );
        }
#endif
        for ( k = 0; k < nnzexp; k++ ) {
            if ( spm2U[k] >= 0 ) {
                Uvalues[ spm2U[k] ] = values[k];  /* SpmGeneral: no conjugation */
            }
        }
    }
    /* SpmHermitian: Uvalues == Lvalues, already updated above. */

    (void)ord; (void)solvmtx; (void)valuesize;
#if defined(PASTIX_WITH_MPI)
    (void)bcsc_comm;
#endif
}
