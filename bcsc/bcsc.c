/**
 *
 * @file bcsc.c
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
 * @author Alycia Lisito
 * @date 2024-07-05
 *
 **/
#include "common.h"
#include "pastix/order.h"
#include <spm.h>
#include "blend/solver.h"
#include "bcsc/bcsc.h"

#include "bcsc/bcsc_z.h"
#include "bcsc/bcsc_c.h"
#include "bcsc/bcsc_d.h"
#include "bcsc/bcsc_s.h"

#define BCSC_COMM_NBR 6

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Initializes the bcsc_handle_comm_t structure.
 *
 *******************************************************************************
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[out] bcsc
 *          The bcsc.
 *
 *******************************************************************************/
void
bcsc_handle_comm_init( const SolverMatrix *solvmtx,
                       pastix_bcsc_t      *bcsc )
{
    pastix_int_t        size = sizeof(bcsc_handle_comm_t) + (solvmtx->clustnbr-1)*sizeof(bcsc_proc_comm_t);
    bcsc_handle_comm_t *bcsc_comm;

    bcsc->bcsc_comm = (bcsc_handle_comm_t *)malloc( size );
    bcsc_comm = bcsc->bcsc_comm;

    bcsc_comm->flttype  = bcsc->flttype;
    bcsc_comm->clustnbr = solvmtx->clustnbr;
    bcsc_comm->clustnum = solvmtx->clustnum;
    bcsc_comm->comm     = solvmtx->solv_comm;

    memset( bcsc_comm->data_comm, 0, bcsc_comm->clustnbr * sizeof(bcsc_proc_comm_t) );
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Frees the bcsc_handle_comm pointers.
 *
 *******************************************************************************
 *
 * @param[inout] bcsc_comm
 *          The bcsc_handle_comm_t structure.
 *
 *******************************************************************************/
void
bcsc_handle_comm_exit( bcsc_handle_comm_t *bcsc_comm )
{
    int c;
    int clustnbr = bcsc_comm->clustnbr;
    bcsc_proc_comm_t *data;

    for ( c = 0; c < clustnbr; c++ ) {
        data = bcsc_comm->data_comm + c;

        if( data->sendA.idxbuf != NULL ) {
            memFree_null( data->sendA.idxbuf );
        }
        if( data->sendA.valbuf != NULL ) {
            memFree_null( data->sendA.valbuf );
        }
        if( data->sendAt.idxbuf != NULL ) {
            memFree_null( data->sendAt.idxbuf );
        }
        if( data->sendAt.valbuf != NULL ) {
            memFree_null( data->sendAt.valbuf );
        }
        if( data->sendAAt.idxbuf != NULL ) {
            memFree_null( data->sendAAt.idxbuf );
        }
        if( data->sendAAt.valbuf != NULL ) {
            memFree_null( data->sendAAt.valbuf );
        }
        if( data->recvAAt.idxbuf != NULL ) {
            memFree_null( data->recvAAt.idxbuf );
        }
        if( data->recvAAt.valbuf != NULL ) {
            memFree_null( data->recvAAt.valbuf );
        }
    }
}

#if defined(PASTIX_WITH_MPI)
/**
 *******************************************************************************
 *
 * @ingroup bcsc
 *
 * @brief Computes the maximum size of the sending indexes and values buffers.
 *
 *******************************************************************************
 *
 * @param[inout] bcsc_comm
 *         On entry the bcsc_comm initialized.
 *         At exit the fields max_idx and max_val of bcsc_comm are updated.
 *
 *******************************************************************************
 *
 * @retval PASTIX_SUCCESS
 *
 *******************************************************************************/
static inline int
bcsc_compute_max( bcsc_handle_comm_t *bcsc_comm )
{
    bcsc_proc_comm_t *data          = NULL;
    bcsc_proc_comm_t *data_local    = NULL;
    pastix_int_t      clustnbr      = bcsc_comm->clustnbr;
    pastix_int_t      clustnum      = bcsc_comm->clustnum;
    pastix_int_t      max_idx       = 0;
    pastix_int_t      max_val       = 0;
    pastix_int_t      idxsum_A  = 0;
    pastix_int_t      valsum_A  = 0;
    pastix_int_t      idxsum_At = 0;
    pastix_int_t      valsum_At = 0;
    pastix_int_t      idxsum_AAt = 0;
    pastix_int_t      valsum_AAt = 0;
    pastix_int_t      idxcnt_A, idxcnt_At, idxcnt_AAt, valcnt_A, valcnt_At, valcnt_AAt, c;

    /* Receives the amount of indexes and values. */
    for ( c = 0; c < clustnbr; c++ ) {
        data = bcsc_comm->data_comm + c;
        if ( c == clustnum ) {
            continue;
        }

        idxcnt_A   = data->recvA.idxcnt;
        idxcnt_At  = data->recvAt.idxcnt;
        idxcnt_AAt = data->recvAAt.size.idxcnt;
        valcnt_A   = data->recvA.valcnt;
        valcnt_At  = data->recvAt.valcnt;
        valcnt_AAt = data->recvAAt.size.valcnt;

        max_idx = pastix_imax( max_idx, idxcnt_A);
        max_idx = pastix_imax( max_idx, idxcnt_At);
        max_idx = pastix_imax( max_idx, idxcnt_AAt);
        max_val = pastix_imax( max_val, valcnt_A);
        max_val = pastix_imax( max_val, valcnt_At);
        max_val = pastix_imax( max_val, valcnt_AAt);

        idxsum_A   += idxcnt_A;
        valsum_A   += valcnt_A;
        idxsum_At  += idxcnt_At;
        valsum_At  += valcnt_At;
        idxsum_AAt += idxcnt_AAt;
        valsum_AAt += valcnt_AAt;
    }

    data_local = bcsc_comm->data_comm + clustnum;
    data_local->recvA.idxcnt        = idxsum_A;
    data_local->recvA.valcnt        = valsum_A;
    data_local->recvAt.idxcnt       = idxsum_At;
    data_local->recvAt.valcnt       = valsum_At;
    data_local->recvAAt.size.idxcnt = idxsum_AAt;
    data_local->recvAAt.size.valcnt = valsum_AAt;

    assert( max_idx <= 2 * max_val );

    bcsc_comm->max_idx = max_idx;
    bcsc_comm->max_val = max_val;

    return PASTIX_SUCCESS;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc
 *
 * @brief Allocates the sending buffers in bcsc_comm->data_comm. These buffers
 * are filled with the sending values.
 *
 *******************************************************************************
 *
 * @param[inout] bcsc_comm
 *         On entry the bcsc_comm initialized.
 *         At exit the arrays of bcsc_comm->data_comm are allocated.
 *
 * @param[in] mode
 *         If PastixTagMemRecvIdx: allocates receiving indexes A and At buffers.
 *         If PastixTagMemSend: allocates sending indexes and values A and At
 *              buffers.
 *         If PastixTagMemRecvValAAt: allocates receiving values AAt buffers, it
 *              used only if the spm is general.
 *
 *******************************************************************************
 *
 * @retval PASTIX_SUCCESS
 *
 *******************************************************************************/
int
bcsc_allocate_buf( bcsc_handle_comm_t *bcsc_comm,
                   bcsc_tag_e          mode  )
{
    bcsc_proc_comm_t *data     = NULL;
    pastix_int_t      clustnbr = bcsc_comm->clustnbr;
    pastix_int_t      clustnum = bcsc_comm->clustnum;
    pastix_int_t      c;
    size_t            size;

    if ( mode == PastixTagMemRecvIdx ) {
        data = bcsc_comm->data_comm + clustnum;

        if ( ( data->recvA.idxcnt > 0 ) && ( data->sendA.idxbuf == NULL ) ) {
            MALLOC_INTERN( data->sendA.idxbuf, data->recvA.idxcnt,  pastix_int_t );
        }

        if ( ( data->recvAt.idxcnt > 0 ) && ( data->sendAt.idxbuf == NULL ) ) {
            MALLOC_INTERN(  data->sendAt.idxbuf, data->recvAt.idxcnt, pastix_int_t );
        }

        if ( ( data->recvAAt.size.idxcnt > 0 ) && ( data->sendAAt.idxbuf == NULL ) ) {
            MALLOC_INTERN(  data->sendAAt.idxbuf, data->recvAAt.size.idxcnt, pastix_int_t );
        }
    }

    if ( mode == PastixTagMemRecvValAAt ) {
        for ( c = 0; c < clustnbr; c ++ ) {
            data = bcsc_comm->data_comm + c;
            if ( c == clustnum ) {
                continue;
            }
            if ( ( data->recvAAt.size.valcnt > 0 ) && ( data->recvAAt.valbuf == NULL ) ) {
                size = data->recvAAt.size.valcnt * pastix_size_of( bcsc_comm->flttype );
                MALLOC_INTERN( data->recvAAt.valbuf, size, char );
            }
        }
    }

    if ( mode == PastixTagMemSend ) {
        for ( c = 0; c < clustnbr; c ++ ) {
            data = bcsc_comm->data_comm + c;

            if ( c == clustnum ) {
                continue;
            }

            if ( ( data->sendA.size.idxcnt > 0 ) && ( data->sendA.idxbuf == NULL ) ) {
                MALLOC_INTERN( data->sendA.idxbuf, data->sendA.size.idxcnt, pastix_int_t );
            }
            if ( ( data->sendA.size.valcnt > 0 ) && ( data->sendA.valbuf == NULL ) ) {
                size = data->sendA.size.valcnt * pastix_size_of( bcsc_comm->flttype );
                MALLOC_INTERN( data->sendA.valbuf, size, char );
            }

            if ( ( data->sendAt.size.idxcnt > 0 ) && ( data->sendAt.idxbuf == NULL ) ) {
                MALLOC_INTERN( data->sendAt.idxbuf, data->sendAt.size.idxcnt, pastix_int_t );
            }
            if ( ( data->sendAt.size.valcnt > 0 ) && ( data->sendAt.valbuf == NULL ) ) {
                size = data->sendAt.size.valcnt * pastix_size_of( bcsc_comm->flttype );
                MALLOC_INTERN( data->sendAt.valbuf, size, char );
            }

            if ( ( data->sendAAt.size.idxcnt > 0 ) && ( data->sendAAt.idxbuf == NULL ) ) {
                MALLOC_INTERN( data->sendAAt.idxbuf, data->sendAAt.size.idxcnt, pastix_int_t );
            }
            if ( ( data->sendAAt.size.valcnt > 0 ) && ( data->sendAAt.valbuf == NULL ) ) {
                size = data->sendAAt.size.valcnt * pastix_size_of( bcsc_comm->flttype );
                MALLOC_INTERN( data->sendAAt.valbuf, size, char );
            }
        }
    }

    return PASTIX_SUCCESS;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc
 *
 * @brief Frees the sending and receiving buffers in bcsc_comm->data_comm.
 * These buffers are filled with the sending adn receiving values.
 *
 *******************************************************************************
 *
 * @param[inout] bcsc_comm
 *         On entry the bcsc_comm initialized.
 *         At exit the arrays of bcsc_comm->data_comm are freed.
 *
 * @param[in] mode
 *         If PastixTagMemSendIdx: frees sending indexes A, At and AAt buffers.
 *         If PastixTagMemSendValA: frees sending values A buffers.
 *         If PastixTagMemSendValAt: frees sending values At buffers.
 *         If PastixTagMemSendValAAt: frees sending values AAt buffers.
 *         If PastixTagMemRecvIdxA: frees receiving indexes A buffers.
 *         If PastixTagMemRecvIdxAt: frees receiving indexes At buffers.
 *         If PastixTagMemRecvAAt:frees receiving indexes and values if the
 *             spm is general AAt buffers.
 *
 *******************************************************************************
 *
 * @retval PASTIX_SUCCESS
 *
 *******************************************************************************/
int
bcsc_free_buf( bcsc_handle_comm_t *bcsc_comm,
               bcsc_tag_e          mode )
{
    bcsc_proc_comm_t *data     = NULL;
    pastix_int_t      clustnbr = bcsc_comm->clustnbr;
    pastix_int_t      clustnum = bcsc_comm->clustnum;
    pastix_int_t      c;

    if ( mode == PastixTagMemSendIdx ) {
        for ( c = 0; c < clustnbr; c ++ ) {
            data = bcsc_comm->data_comm + c;
            if ( c == clustnum ) {
                continue;
            }
            if ( data->sendA.idxbuf != NULL ) {
                memFree_null( data->sendA.idxbuf );
            }
            if ( data->sendAt.idxbuf != NULL ) {
                memFree_null( data->sendAt.idxbuf );
            }
            if ( data->sendAAt.idxbuf != NULL ) {
                memFree_null( data->sendAAt.idxbuf );
            }
        }
    }

    if ( mode == PastixTagMemSendValA ) {
        for ( c = 0; c < clustnbr; c ++ ) {
            data = bcsc_comm->data_comm + c;
            if ( c == clustnum ) {
                continue;
            }
            if ( data->sendA.valbuf != NULL ) {
                memFree_null( data->sendA.valbuf );
            }
        }
    }

    if ( mode == PastixTagMemSendValAt ) {
        for ( c = 0; c < clustnbr; c ++ ) {
            data = bcsc_comm->data_comm + c;
            if ( c == clustnum ) {
                continue;
            }
            if ( data->sendAt.valbuf != NULL ) {
                memFree_null( data->sendAt.valbuf );
            }
        }
    }

    if ( mode == PastixTagMemSendValAAt ) {
        for ( c = 0; c < clustnbr; c ++ ) {
            data = bcsc_comm->data_comm + c;
            if ( c == clustnum ) {
                continue;
            }
            if ( data->sendAAt.valbuf != NULL ) {
                memFree_null( data->sendAAt.valbuf );
            }
        }
    }

    if ( mode == PastixTagMemRecvIdxA ) {
        data = bcsc_comm->data_comm + clustnum;
        if ( data->sendA.idxbuf != NULL ) {
            memFree_null( data->sendA.idxbuf );
        }
    }

    if ( mode == PastixTagMemRecvIdxAt ) {
        data = bcsc_comm->data_comm + clustnum;
        if ( data->sendAt.idxbuf != NULL ) {
            memFree_null(  data->sendAt.idxbuf );
        }
    }

    if ( mode == PastixTagMemRecvAAt ) {
        data = bcsc_comm->data_comm + clustnum;
        if ( data->sendAAt.idxbuf != NULL ) {
            memFree_null( data->sendAAt.idxbuf );
        }
        if ( data->recvAAt.valbuf != NULL ) {
            memFree_null( data->recvAAt.valbuf );
        }
    }

    return PASTIX_SUCCESS;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Exchanges the amount of data the current processor will send to and
 *        receive from each processor.
 *
 *******************************************************************************
 *
 * @param[in] bcsc_comm
 *          The bcsc_handle_comm_t structure.
 *
 *******************************************************************************/
void
bcsc_exchange_amount_of_data( bcsc_handle_comm_t *bcsc_comm )
{
    bcsc_proc_comm_t   *data_comm   = bcsc_comm->data_comm;
    pastix_int_t        clustnbr    = bcsc_comm->clustnbr;
    pastix_int_t        clustnum    = bcsc_comm->clustnum;
    bcsc_proc_comm_t   *data_send   = NULL;
    bcsc_proc_comm_t   *data_recv   = NULL;
    pastix_int_t        counter_req = 0;
    MPI_Status          statuses[(clustnbr-1)*BCSC_COMM_NBR];
    MPI_Request         requests[(clustnbr-1)*BCSC_COMM_NBR];
    bcsc_data_amount_t *sends, *recvs;
    pastix_int_t        c_send, c_recv, k;

    /* Exchanges the amount of indexes and values. */
    c_send = (clustnum+1) % clustnbr;
    c_recv = (clustnum-1+clustnbr) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        data_send = data_comm + c_send;
        data_recv = data_comm + c_recv;

        if ( c_send == clustnum ) {
            continue;
        }

        /* Exchanges the amount of indexes and values for A. */
        sends = &( data_send->sendA.size );
        recvs = &( data_recv->recvA );
        MPI_Irecv( recvs, 2, PASTIX_MPI_INT, c_recv,
                   PastixTagCountA, bcsc_comm->comm, &requests[counter_req++] );

        MPI_Isend( sends, 2, PASTIX_MPI_INT, c_send,
                   PastixTagCountA, bcsc_comm->comm, &requests[counter_req++] );

        /* Exchanges the amount of indexes and values for At. */
        sends = &( data_send->sendAt.size );
        recvs = &( data_recv->recvAt );
        MPI_Irecv( recvs, 2, PASTIX_MPI_INT, c_recv,
                   PastixTagCountAt, bcsc_comm->comm, &requests[counter_req++] );

        MPI_Isend( sends, 2, PASTIX_MPI_INT, c_send,
                   PastixTagCountAt, bcsc_comm->comm, &requests[counter_req++] );

        /* Exchanges the amount of indexes and values for AAt. */
        sends = &( data_send->sendAAt.size );
        recvs = &( data_recv->recvAAt.size );
        MPI_Irecv( recvs, 2, PASTIX_MPI_INT, c_recv,
                   PastixTagCountAAt, bcsc_comm->comm, &requests[counter_req++] );

        MPI_Isend( sends, 2, PASTIX_MPI_INT, c_send,
                   PastixTagCountAAt, bcsc_comm->comm, &requests[counter_req++] );

        c_send = (c_send+1) % clustnbr;
        c_recv = (c_recv-1+clustnbr) % clustnbr;
    }

    MPI_Waitall( counter_req, requests, statuses );

    bcsc_compute_max( bcsc_comm );

    return;
}
#endif

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Creates the array which represents the repartition of each column
 *        in the block structure. The array size is spm->gNexp where:
 *            - col2cblk[k] = cblknum, with cblknum the index of the block column
 *              where the column k is stored.
 *        This routine is called when the matrix is in shared memory.
 *
 *******************************************************************************
 *
 * @param[in] solvmtx
 *          The solvmtx structure associated to the problem.
 *
 * @param[in,out] bcsc
 *           The internal block CSC structure.
 *           The number of local columns is updated.
 *
 *******************************************************************************
 *
 * @return The col2cblk array which gives the repartition of the solvmtx columns
 *         into the block structure.
 *
 *******************************************************************************/
pastix_int_t *
bcsc_init_col2cblk_shm( const SolverMatrix  *solvmtx,
                        const pastix_bcsc_t *bcsc )
{
    pastix_int_t      j;
    pastix_int_t      cblknum;
    pastix_int_t     *col2cblk;

    /* Allocates the col2cblk. */
    MALLOC_INTERN( col2cblk, bcsc->gN, pastix_int_t );
    memset( col2cblk, 0xff, bcsc->gN * sizeof(pastix_int_t) );

    const SolverCblk *cblk    = solvmtx->cblktab;
    pastix_int_t      cblknbr = solvmtx->cblknbr;
    /* Goes through the blocks. */
    for ( cblknum = 0; cblknum < cblknbr; cblknum++, cblk++ ) {
        if ( cblk->cblktype & (CBLK_FANIN|CBLK_RECV) ) {
            continue;
        }
        /*
         * Goes through the columns of the block and adds the number of
         * the block in col2cblk at the corresponding index.
         */
        for ( j = cblk->fcolnum; j <= cblk->lcolnum; j++ ) {
            col2cblk[j] = cblknum;
        }
    }

    return col2cblk;
}

#if defined(PASTIX_WITH_MPI)
/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Creates the array which represents the repartition of each column
 *        in the block structure. The array size is spm->gNexp where:
 *            - col2cblk[k] = - (owner + 1) if the column is not stored in a local block
 *            - col2cblk[k] = cblknum, if the column k is stored in a local block, with
 *              cblknum the index of this block column.
 *       This routine is called when the matrix is in distributed memory.
 *
 *******************************************************************************
 *
 * @param[in] solvmtx
 *          The solvmtx structure associated to the problem.
 *
 * @param[in,out] bcsc
 *           The internal block CSC structure.
 *           The number of local columns is updated.
 *
 *******************************************************************************
 *
 * @return The col2cblk array which gives the repartition of the solvmtx columns
 *         into the block structure.
 *
 *******************************************************************************/
pastix_int_t *
bcsc_init_col2cblk_dst( const SolverMatrix  *solvmtx,
                        const pastix_bcsc_t *bcsc )
{
    pastix_int_t  n, nr = 0;
    pastix_int_t  k, j, c;
    pastix_int_t  clustnum = solvmtx->clustnum;
    pastix_int_t  clustnbr = solvmtx->clustnbr;
    pastix_int_t  fcolnum, lcolnum, cblknum;
    pastix_int_t *col2cblk;
    pastix_int_t *col2cblk_bcast = NULL;

    /* Allocates the col2cblk. */
    MALLOC_INTERN( col2cblk, bcsc->gN, pastix_int_t );
    memset( col2cblk, 0xff, bcsc->gN * sizeof(pastix_int_t) );

    for( c = 0; c < clustnbr; c++ ) {
        if ( c == clustnum ) {
            const SolverCblk *cblk    = solvmtx->cblktab;
            pastix_int_t      cblknbr = solvmtx->cblknbr;
            pastix_int_t      colcount;

            n = (solvmtx->cblknbr - solvmtx->faninnbr - solvmtx->recvnbr) * 2;

            /* Sends the size of data. */
            MPI_Bcast( &n, 1, PASTIX_MPI_INT, c, solvmtx->solv_comm );

            if ( n == 0 ) {
                continue;
            }

            if ( n > nr ) {
                pastix_int_t *tmp;
                nr = n;
                tmp = (pastix_int_t *)realloc( col2cblk_bcast, nr * sizeof(pastix_int_t) );
                 /* Protection for static analysis */
                if ( pastix_unlikely( tmp == NULL ) ) {
                    pastix_print_error( "Error reallocating col2cblk_bcast\n" );
                }
                col2cblk_bcast = tmp;
            }

            colcount = 0;
            k = 0;
            /* Goes through the blocks. */
            for ( cblknum = 0; cblknum < cblknbr; cblknum++, cblk++ ) {
                if ( cblk->cblktype & (CBLK_FANIN|CBLK_RECV) ) {
                    continue;
                }
                assert( col2cblk_bcast != NULL );

                /* Adds the first and last columns of the block in col2cblk_bcast. */
                col2cblk_bcast[k]   = cblk->fcolnum;
                col2cblk_bcast[k+1] = cblk->lcolnum;
                k += 2;
                /*
                 * Goes through the columns of the block and adds the
                 * block number in col2cblk.
                 */
                for ( j = cblk->fcolnum; j <= cblk->lcolnum; j++ ) {
                    colcount++;
                    col2cblk[j] = cblknum;
                }
            }
            assert( colcount == bcsc->n );

            /* Sends the col2cblk_bcast. */
            if ( n > 0 ) {
                MPI_Bcast( col2cblk_bcast, n, PASTIX_MPI_INT, c, solvmtx->solv_comm );
            }
        }
        else {
            /* Receives the size of data from c. */
            MPI_Bcast( &n, 1, PASTIX_MPI_INT, c, solvmtx->solv_comm );

            if ( n == 0 ) {
                continue;
            }

            if ( n > nr ) {
                pastix_int_t *tmp;
                nr = n;
                tmp = (pastix_int_t *)realloc( col2cblk_bcast, nr * sizeof(pastix_int_t) );
                 /* Protection for static analysis */
                if ( pastix_unlikely( tmp == NULL ) ) {
                    pastix_print_error( "Error reallocating col2cblk_bcast\n" );
                }
                col2cblk_bcast = tmp;
            }

            /* Receives the col2cblk_bcast from c. */
            MPI_Bcast( col2cblk_bcast, n, PASTIX_MPI_INT, c, solvmtx->solv_comm );
            /*
             * Goes through the columns in col2cblk_bcast and adds the processor
             * number in col2cblk.
             */
            for ( k = 0; k < n; k += 2 ) {
                fcolnum = col2cblk_bcast[k];
                lcolnum = col2cblk_bcast[k+1];
                for ( j = fcolnum; j <= lcolnum; j++ ) {
                    col2cblk[j] = - c - 1;
                }
            }
        }
    }

    free( col2cblk_bcast );

    return col2cblk;
}
#endif

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Creates the array which represents the repartition of each column
 *        in the block structure. This routine calls bcsc_init_col2cblk_shm or
 *        bcsc_init_col2cblk_dst according to the way the matrix is stored in the
 *        memory.
 *
 *******************************************************************************
 *
 * @param[in] solvmtx
 *          The solvmtx structure associated to the problem.
 *
 * @param[in] bcsc
 *           The internal block CSC structure.
 *           The number of local columns is updated.
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 *******************************************************************************
 *
 * @return The col2cblk array which gives the repartition of the solvmtx columns
 *         into the block structure.
 *
 *******************************************************************************/
pastix_int_t *
bcsc_init_col2cblk( const SolverMatrix  *solvmtx,
                    const pastix_bcsc_t *bcsc,
                    const spmatrix_t    *spm )
{
    pastix_int_t *col2cblk;

    /* Tests if the spm is in shared or distributed memory. */
#if defined(PASTIX_WITH_MPI)
    if ( spm->loc2glob != NULL ) {
        col2cblk = bcsc_init_col2cblk_dst( solvmtx, bcsc );
    }
    else
#endif
    {
        col2cblk = bcsc_init_col2cblk_shm( solvmtx, bcsc );
    }

    (void)spm;
    return col2cblk;
}

/**
 *******************************************************************************
 *
 * @brief Initializes the dofshit array of size gNexp which gives
 *        dofshift[index_permuted] = index. This corresponds to the inverse of
 *        the permutation given in ord->permtab.
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
 *******************************************************************************
 *
 * @return The dofshift array.
 *
 *******************************************************************************/
static inline pastix_int_t*
bcsc_init_dofshift( const spmatrix_t     *spm,
                    const pastix_order_t *ord )
{
    pastix_int_t *dofshift, *ptr;
    pastix_int_t *dofs;
    pastix_int_t  dof;
    pastix_int_t  idof, dofj, dofidx;
    pastix_int_t  jg, jgp;

    /* Allocates the dofshift array. */
    MALLOC_INTERN( dofshift, spm->gNexp, pastix_int_t );

    dofs = spm->dofs;
    dof  = spm->dof;
    ptr  = dofshift;
    for ( jg = 0; jg < spm->gN; jg++ ) {
        jgp    = ord->permtab[jg];
        dofidx = (dof > 0) ? jgp * dof : dofs[jg];
        ptr    = dofshift + dofidx;
        dofj   = (dof > 0) ? dof : dofs[jg+1] - dofs[jg];
        for ( idof = 0; idof < dofj; idof++, ptr++ ) {
            *ptr = jgp;
        }
    }
    return dofshift;
}

/**
 *******************************************************************************
 *
 * @brief Initializes the coltab of a block csc matrix. The coltab corresponds to
 *        the number of rows (expended) per column (non expended). This rountine
 *        is called when the matrix is stored in shared memory or the matrix is
 *        replicated on the processors and the matrix's degree of liberty is
 *        constant.
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
 * @param[out] globcol
 *          The array which contains, for each column, its beginning in the
 *          smp->colptr.
 *
 *******************************************************************************/
static inline void
bcsc_init_global_coltab_shm_cdof( const spmatrix_t     *spm,
                                  const pastix_order_t *ord,
                                  pastix_int_t         *globcol )
{
    pastix_int_t *colptr   = spm->colptr;
    pastix_int_t *rowptr   = spm->rowptr;
    pastix_int_t  dof      = spm->dof;
    pastix_int_t  baseval  = spm->baseval;
    pastix_int_t  frow, lrow;
    pastix_int_t  k, j, ig, jg, igp, jgp;
    int           sym = (spm->mtxtype == SpmSymmetric) || (spm->mtxtype == SpmHermitian);

    assert( dof > 0 );
    assert( spm->loc2glob == NULL );

    /* Goes through the column of the spm. */
    for ( j = 0; j < spm->n; j++, colptr++ ) {
        jg   = j;
        jgp  = ord->permtab[jg];
        frow = colptr[0] - baseval;
        lrow = colptr[1] - baseval;
        assert( (lrow - frow) >= 0 );
        /* Adds the number of values in the column jg. */
        globcol[jgp] += (lrow - frow) * dof;

        /*
         * Adds for At the number of values in the row ig and column jg. This
         * is not required for the general case as the spm has a symmetric
         * pattern.
         */
        if ( !sym ) {
            continue;
        }

        for ( k = frow; k < lrow; k++ ) {
            ig = rowptr[k] - baseval;
            if ( ig != jg ) {
                igp = ord->permtab[ig];
                globcol[igp] += dof;
            }
        }
    }

    return;
}

/**
 *******************************************************************************
 *
 * @brief Initializes the coltab of a block csc matrix. The coltab corresponds to
 *        the number of rows (expended) per column (non expended). This rountine
 *        is called when the matrix is stored in shared memory or the matrix is
 *        replicated on the processors and the matrix's degree of liberty is
 *        variadic.
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
 * @param[out] globcol
 *          The array which contains, for each column, its begining in the
 *          smp->colptr.
 *
 *******************************************************************************/
static inline void
bcsc_init_global_coltab_shm_vdof( const spmatrix_t     *spm,
                                  const pastix_order_t *ord,
                                  pastix_int_t         *globcol )
{
    pastix_int_t *colptr   = spm->colptr;
    pastix_int_t *rowptr   = spm->rowptr;
    pastix_int_t *dofs     = spm->dofs;
    pastix_int_t  baseval  = spm->baseval;
    pastix_int_t  frow, lrow;
    pastix_int_t  k, j, ig, jg, igp, jgp;
    pastix_int_t  dofj, dofi;
    int           sym = (spm->mtxtype == SpmSymmetric) || (spm->mtxtype == SpmHermitian);

    assert( spm->dof <= 0 );
    assert( spm->loc2glob == NULL );

    /* Goes through the column of the spm. */
    for ( j=0; j<spm->n; j++, colptr++ ) {
        jg   = j;
        dofj = dofs[jg+1] - dofs[jg];
        jgp  = ord->permtab[jg];
        frow = colptr[0] - baseval;
        lrow = colptr[1] - baseval;
        assert( (lrow - frow) >= 0 );

        for ( k=frow; k<lrow; k++ ) {
            ig   = rowptr[k] - baseval;
            dofi = dofs[ig+1] - dofs[ig];
            /* Adds the number of values in the row ig and column jg. */
            globcol[jgp] += dofi;

            /* Adds for At the number of values in the row ig and column jg. */
            if ( sym && (ig != jg) ) {
                igp= ord->permtab[ig];
                globcol[igp] += dofj;
            }
        }
    }

    return;
}

#if defined(PASTIX_WITH_MPI)
/**
 *******************************************************************************
 *
 * @brief Initializes the coltab of a block csc matrix. The coltab corresponds to
 *        the number of rows (expended) per column (non expended). This rountine
 *        is called when the matrix is distributed in the memory and the matrix's
 *        degree of liberty is constant.
 *
 * There are two cases:
 *
 * If the matrix is general: the full columns and rows of the blocks are stored
 *      in Lvalues and Uvalues.
 *      - The local data of the current process which are in remote blocks after
 *        the permutation need be to sent to the owner process. The data is stored
 *        in sendA if it is sent for the column only, in sendAt if it is sent for
 *        the row only and in sendAAt if it is sent for the row and column.
 *      - The local data of the current process which are in local column blocks
 *        after the permutation need to be added in globcol.
 *
 * If the matrix is Symmetric or Hermitian: only the full columns of the blocks
 *      are stored in Lvalues (and Uvalues = Lvalues). Only half of the spm is
 *      stored lower (or upper) triangular half, therefore we need to duplicate
 *      the lower (or upper) data to fill the upper (or lower half of the matrix
 *      in the blocks.
 *      - The local data of the current process which are in remote blocks after
 *        the permutation need be to sent to the owner process. The data is stored
 *        in sendA if it is sent for the lower (or upper) half or the column, in
 *        sendAt if it is sent for the upper (or lower) half of the column and in
 *        sendAAt if it is sent for both the lower and upper half of the column.
 *        The diagonal values are stored in sendA only.
 *      - The local data of the current process which are in column blocks after
 *        the permutation need to be added in globcol twice: once for the lower
 *        half and once for the upper half. The diagonal values need to be added
 *        only once.
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
 * @param[in] col2cblk
 *          The array which contains the repartition of the matrix columns
 *          into the block structure.
 *
 * @param[out] globcol
 *          The array which contains, for each column, its begining in the
 *          smp->colptr.
 *
 * @param[in,out] bcsc_comm
 *          On entry, the initialised bcsc_comm structure. On exit, the
 *          bcsc_handle_comm structure which contains the amount of data to
 *          send to the other processors.
 *
 *******************************************************************************/
static inline void
bcsc_init_global_coltab_dst_cdof( const spmatrix_t     *spm,
                                  const pastix_order_t *ord,
                                  const pastix_int_t   *col2cblk,
                                  pastix_int_t         *globcol,
                                  bcsc_handle_comm_t   *bcsc_comm )
{
    pastix_int_t     *colptr    = spm->colptr;
    pastix_int_t     *rowptr    = spm->rowptr;
    pastix_int_t     *loc2glob  = spm->loc2glob;
    pastix_int_t      dof       = spm->dof;
    pastix_int_t      baseval   = spm->baseval;
    bcsc_proc_comm_t *data_comm = bcsc_comm->data_comm;
    bcsc_exch_comm_t *data_sendA, *data_sendAt, *data_sendAAt;
    pastix_int_t      frow, lrow;
    pastix_int_t      il, jl, ig, jg, igp, jgp;
    int               sym = (spm->mtxtype == SpmSymmetric) || (spm->mtxtype == SpmHermitian);
    pastix_int_t      ownerj, owneri;

    assert( dof > 0 );

    /* Goes through the columns of spm. */
    for ( jl = 0; jl < spm->n; jl++, colptr++, loc2glob++ ) {
        jg  = *loc2glob - baseval;
        jgp = ord->permtab[jg];

        frow = colptr[0] - baseval;
        lrow = colptr[1] - baseval;
        assert( (lrow - frow) >= 0 );

        ownerj = col2cblk[jgp * dof];

        /* The column jp belongs to another process. */
        if ( ownerj < 0 ) {
            ownerj     = - ownerj - 1;
            data_comm  = bcsc_comm->data_comm + ownerj;
            data_sendA = &( data_comm->sendA );

            /* Goes through the rows of jl. */
            for ( il = frow; il < lrow; il++ ) {
                ig = rowptr[il] - baseval;

                /*
                 * The diagonal values (ip, jp) belong to the same process.
                 * They are sent to owneri in the sym case for A only.
                 */
                if ( sym && ( ig == jg ) ) {
                    data_sendA->size.idxcnt += 2;
                    data_sendA->size.valcnt += dof * dof;
                    continue;
                }

                igp    = ord->permtab[ig];
                owneri = col2cblk[igp* dof];

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

                        data_sendAAt->size.idxcnt += 2;
                        data_sendAAt->size.valcnt += dof * dof;
                    }
                    /*
                     * The values (ip, jp) belong to different processes.
                     * They are sent to owneri for At and to ownerj for A.
                     */
                    else {
                        data_sendAt = &( data_comm->sendAt );

                        data_sendAt->size.idxcnt += 2;
                        data_sendAt->size.valcnt += dof * dof;

                        data_sendA->size.idxcnt += 2;
                        data_sendA->size.valcnt += dof * dof;
                    }
                }
                /* The row ip is local. */
                else {
                    /*
                     * The values (ip, jp) belong to ownerj.
                     * They are sent to ownerj for A.
                     */
                    data_sendA->size.idxcnt += 2;
                    data_sendA->size.valcnt += dof * dof;
                    /*
                     * The values (ip, jp) are local.
                     * In the sym case ther are added to globcol.
                     */
                    if ( sym ) {
                        globcol[igp] += dof;
                    }
                }
            }
        }
        /* The column jp is local. */
        else {
            /* The column is added to globcol. */
            globcol[jgp] += dof * ( lrow - frow );

            /* Goes through the rows of j. */
            for ( il = frow; il < lrow; il++ ) {
                ig = rowptr[il] - baseval;

                /*
                 * The diagonal values (ip, jp) have already been
                 * added to globcol in the sym case.
                 */
                if ( sym && ( ig == jg ) ) {
                    continue;
                }

                igp    = ord->permtab[ig];
                owneri = col2cblk[igp* dof];

                /* The row ip belongs to another process. */
                if ( owneri < 0 ) {
                    owneri    = - owneri - 1;
                    data_comm = bcsc_comm->data_comm + owneri;

                    /*
                     * The values (ip, jp) belong to owneri.
                     * They are sent to ownerj for At.
                     */
                    data_sendAt = &( data_comm->sendAt );

                    data_sendAt->size.idxcnt += 2;
                    data_sendAt->size.valcnt += dof * dof;
                }
                else {
                    /*
                     * The values (ip, jp) are local.
                     * In the sym case they are added to globcol.
                     */
                    if ( sym ) {
                        globcol[igp] += dof;
                    }
                }
            }
        }
    }

    return;
}

/**
 *******************************************************************************
 *
 * @brief Initializes the coltab of a block csc matrix. The coltab corresponds to
 *        the number of rows (expended) per column (non expended). This rountine
 *        is called when the matrix is distributed in the memory and the matrix's
 *        degree of liberty is variadic.
 *
 * DO NOT CURRENTLY WORKS
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
 * @param[in] col2cblk
 *          The array which contains the repartition of the matrix columns
 *          into the block structure.
 *
 * @param[out] globcol
 *          The array which contains, for each column, its begining in the
 *          smp->colptr.
 *
 * @param[out] bcsc_comm
 *          The bcsc_handle_comm structure which contains the amount of
 *          data to send to the other processors.
 *
 *******************************************************************************/
static inline void
bcsc_init_global_coltab_dst_vdof( __attribute__((unused)) const spmatrix_t     *spm,
                                  __attribute__((unused)) const pastix_order_t *ord,
                                  __attribute__((unused)) const pastix_int_t   *col2cblk,
                                  __attribute__((unused)) pastix_int_t         *globcol,
                                  __attribute__((unused)) bcsc_handle_comm_t   *bcsc_comm )
{
    // pastix_int_t *colptr   = spm->colptr;
    // pastix_int_t *rowptr   = spm->rowptr;
    // pastix_int_t *loc2glob = spm->loc2glob;
    // pastix_int_t *dofs     = spm->dofs;
    // pastix_int_t  dof      = spm->dof;
    // pastix_int_t  baseval  = spm->baseval;
    // pastix_int_t  frow, lrow;
    // pastix_int_t  k, j, ig, jg, igp, jgp;
    // pastix_int_t  dofj, dofi;
    // int           sym = (spm->mtxtype == SpmSymmetric) || (spm->mtxtype == SpmHermitian);

    // assert( dof <= 0 );

    // for ( j=0; j<spm->n; j++, colptr++, loc2glob++ )
    // {
    //     jg   = *loc2glob - baseval;
    //     jgp  = ord->permtab[jg];
    //     dofj = dofs[jg+1] - dofs[jg];

    //     frow = colptr[0] - baseval;
    //     lrow = colptr[1] - baseval;
    //     assert( (lrow - frow) >= 0 );

    //     jgpe    = ...;
    //     ownerj = col2cblk[jgpe]; // FAUX
    //     localj = ( ownerj >= 0 );
    //     ownerj = - ownerj - 1;

    //     for ( k=frow; k<lrow; k++ )
    //     {
    //         ig   = rowptr[k] - baseval;
    //         dofi = dofs[ig+1] - dofs[ig];

    //         if ( localj ) {
    //             /* The column is local */
    //             globcol[jgp] += dofi;
    //         }
    //         else {
    //             /* The column is remote */
    //             //update_counter_tosend( ownerj, 1 /* Nbr Elt */, dofi /* Nbr values */ );
    //         }

    //         if ( sym && (ig != jg) ) {
    //             igp    = ord->permtab[ig];
    //             igpe    = ...;
    //             owneri = col2cblk[igpe]; // FAUX

    //             if ( owneri >= 0 ) {
    //                 globcol[igp] += dofj;
    //             }
    //             else {
    //                 owneri = - owneri - 1;
    //                 //update_counter_tosend( owneri, 1 /* Nbr Elt */, dofj /* Nbr values */ );
    //             }
    //         }
    //     }
    // }

    return;
}

/**
 *******************************************************************************
 *
 * @brief Exchanges the indexes with the other processors.
 *
 *******************************************************************************
 *
 * @param[in,out] bcsc_comm
 *          The bcsc_handle_comm structure which contains the data the current
 *          processor has to send to the other processors on entry. On exit,
 *          the structure is updated with the received data from the other
 *          processors.
 *
 *******************************************************************************/
void
bcsc_exchange_indexes( bcsc_handle_comm_t *bcsc_comm )
{
    pastix_int_t        clustnbr      = bcsc_comm->clustnbr;
    pastix_int_t        clustnum      = bcsc_comm->clustnum;
    bcsc_proc_comm_t   *data_comm     = bcsc_comm->data_comm;
    bcsc_proc_comm_t   *data_local    = bcsc_comm->data_comm + clustnum;
    bcsc_exch_comm_t   *sendA_local   = &( data_local->sendA );
    bcsc_exch_comm_t   *sendAt_local  = &( data_local->sendAt );
    bcsc_exch_comm_t   *sendAAt_local = &( data_local->sendAAt );
    pastix_int_t        counter_req   = 0;
    pastix_int_t        cntA          = 0;
    pastix_int_t        cntAt         = 0;
    pastix_int_t        cntAAt        = 0;
    pastix_int_t        idx_cnt_A[clustnbr];
    pastix_int_t        idx_cnt_At[clustnbr];
    pastix_int_t        idx_cnt_AAt[clustnbr];
    MPI_Status          statuses[(clustnbr-1)*BCSC_COMM_NBR];
    MPI_Request         requests[(clustnbr-1)*BCSC_COMM_NBR];
    bcsc_proc_comm_t   *data_send, *data_recv;
    bcsc_exch_comm_t   *send;
    bcsc_data_amount_t *recv;
    pastix_int_t        c_send, c_recv, k;

    bcsc_allocate_buf( bcsc_comm, PastixTagMemRecvIdx );

    for ( k = 0; k < clustnbr; k++ ) {
        if ( k == clustnum ) {
            idx_cnt_A[k]  = 0;
            idx_cnt_At[k] = 0;
            idx_cnt_AAt[k] = 0;
            continue;
        }
        idx_cnt_A[ k ]  = cntA;
        cntA += data_comm[k].recvA.idxcnt;
        idx_cnt_At[ k ] = cntAt;
        cntAt += data_comm[k].recvAt.idxcnt;
        idx_cnt_AAt[ k ] = cntAAt;
        cntAAt += data_comm[k].recvAAt.size.idxcnt;
    }

    c_send = (clustnum+1) % clustnbr;
    c_recv = (clustnum-1+clustnbr) % clustnbr;
    for ( k = 0; k < clustnbr-1; k++ ) {
        data_send = data_comm + c_send;
        data_recv = data_comm + c_recv;
        if ( c_send == clustnum ) {
            continue;
        }

        /* Posts the receptions of the indexes. */
        recv = &( data_recv->recvA );
        if ( recv->idxcnt != 0 ) {
            MPI_Irecv( sendA_local->idxbuf + idx_cnt_A[c_recv], recv->idxcnt,
                       PASTIX_MPI_INT, c_recv, PastixTagIndexesA, bcsc_comm->comm,
                       &requests[counter_req++] );
        }
        recv = &( data_recv->recvAt );
        if ( recv->idxcnt != 0 ) {
            MPI_Irecv( sendAt_local->idxbuf + idx_cnt_At[c_recv], recv->idxcnt,
                       PASTIX_MPI_INT, c_recv, PastixTagIndexesAt, bcsc_comm->comm,
                       &requests[counter_req++] );
        }
        recv = &( data_recv->recvAAt.size );
        if ( recv->idxcnt != 0 ) {
            MPI_Irecv( sendAAt_local->idxbuf + idx_cnt_AAt[c_recv], recv->idxcnt,
                       PASTIX_MPI_INT, c_recv, PastixTagIndexesAAt, bcsc_comm->comm,
                       &requests[counter_req++] );
        }

        /* Posts the emissions of the indexes. */
        send = &( data_send->sendA );
        if ( send->size.idxcnt != 0 ) {
            MPI_Isend( send->idxbuf, send->size.idxcnt, PASTIX_MPI_INT, c_send,
                       PastixTagIndexesA, bcsc_comm->comm, &requests[counter_req++] );
        }
        send = &( data_send->sendAt );
        if ( send->size.idxcnt != 0 ) {
            MPI_Isend( send->idxbuf, send->size.idxcnt, PASTIX_MPI_INT, c_send,
                       PastixTagIndexesAt, bcsc_comm->comm, &requests[counter_req++] );
        }
        send = &( data_send->sendAAt );
        if ( send->size.idxcnt != 0 ) {
            MPI_Isend( send->idxbuf, send->size.idxcnt, PASTIX_MPI_INT, c_send,
                       PastixTagIndexesAAt, bcsc_comm->comm, &requests[counter_req++] );
        }
        c_send = (c_send+1) % clustnbr;
        c_recv = (c_recv-1+clustnbr) % clustnbr;
    }

    MPI_Waitall( counter_req, requests, statuses );
}

/**
 *******************************************************************************
 *
 * @brief Updates globcol with the received indexes.
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
 * @param[out] globcol
 *          The array which contains, for each column, its begining in the
 *          smp->colptr. This array is updated with the data received from the
 *          other processors.
 *
 * @param[in] bcsc_comm
 *          The bcsc_handle_comm structure which contains the received data
 *          from the other processors.
 *
 *******************************************************************************/
static inline void
bcsc_update_globcol( const spmatrix_t     *spm,
                     const pastix_order_t *ord,
                     pastix_int_t         *globcol,
                     bcsc_handle_comm_t   *bcsc_comm )
{
    pastix_int_t     *dofs          = spm->dofs;
    pastix_int_t      dof           = spm->dof;
    pastix_int_t      clustnum      = bcsc_comm->clustnum;
    bcsc_proc_comm_t *data_local    = bcsc_comm->data_comm + clustnum;
    bcsc_exch_comm_t *sendA_local   = &( data_local->sendA );
    bcsc_exch_comm_t *sendAt_local  = &( data_local->sendAt );
    bcsc_exch_comm_t *sendAAt_local = &( data_local->sendAAt );
    pastix_int_t      k, igp, jgp, jg, ig, baseval;
    pastix_int_t     *indexes_A;
    pastix_int_t     *indexes_At;
    pastix_int_t     *indexes_AAt;

    assert( ord->baseval == 0 );
    baseval = ord->baseval;

    /* Updates globcol. */
    indexes_A   = sendA_local->idxbuf;
    indexes_At  = sendAt_local->idxbuf;
    indexes_AAt = sendAAt_local->idxbuf;

    /* Goes through indexes_A. */
    for ( k = 0; k < data_local->recvA.idxcnt; k += 2, indexes_A += 2 ) {
        igp = indexes_A[0];
        jgp = indexes_A[1];
        ig  = ord->peritab[igp] - baseval;

        /* Adds the values (igp, jgp) to globcol. */
        globcol[jgp] += ( dof < 0 ) ? dofs[ ig+1 ] - dofs[ig] : dof;
    }

    /* Goes through indexes_At. */
    if ( spm->mtxtype != SpmGeneral ) {
        for ( k = 0; k < data_local->recvAt.idxcnt; k += 2, indexes_At += 2 ) {
            igp = indexes_At[0];
            jgp = indexes_At[1];
            jg  = ord->peritab[jgp] - baseval;

            /* Adds the values (igp, jgp) to globcol. */
            globcol[igp] += ( dof < 0 ) ? dofs[ jg+1 ] - dofs[jg] : dof;
        }
    }

    /* Goes through indexes_AAt. */
    for ( k = 0; k < data_local->recvAAt.size.idxcnt; k += 2, indexes_AAt += 2 ) {
        igp = indexes_AAt[0];
        jgp = indexes_AAt[1];
        ig  = ord->peritab[igp] - baseval;
        jg  = ord->peritab[jgp] - baseval;

        /* Adds the values (igp, jgp) to globcol. */
        globcol[jgp] += ( dof < 0 ) ? dofs[ ig+1 ] - dofs[ig] : dof;

        if ( spm->mtxtype != SpmGeneral ) {
            /* Adds the values (igp, jgp) twice to globcol if sym. */
            globcol[igp] += ( dof < 0 ) ? dofs[ jg+1 ] - dofs[jg] : dof;
        }
    }
}
#endif

/**
 *******************************************************************************
 *
 * @brief Initializes the coltab of a block csc matrix. The coltab corresponds to
 *        the number of rows (expended) per column (non expended). This routine
 *        is calls bcsc_init_global_coltab_[shm,dst]_[c,v]dof according to the way
 *        the matrix is stored and if the degree of liberty of the matrix is
 *        constant or variadic. If the matrix is distributed in the memory, this
 *        function also calls the routines which exchange the amount of data for
 *        the communication, store the indexes and values to send and exchange
 *        the indexes.
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
 * @param[in] col2cblk
 *          The array which contains the repartition of the matrix columns
 *          into the block structure.
 *
 * @param[in,out] bcsc_comm
 *          The handle_comm_structure updated with the amount of data the current
 *          processor has to send to the other processor if PASTIX_WITH_MPI = ON
 *          and the matrix is distributed in memory. If it is not the case,
 *          bcsc_comm = NULL.
 *
 *******************************************************************************
 *
 * @returns The array which contains, for each column, its begining in the
 *          smp->colptr.
 *
 *******************************************************************************/
static inline pastix_int_t*
bcsc_init_global_coltab( const spmatrix_t     *spm,
                         const pastix_order_t *ord,
                         const SolverMatrix   *solvmtx,
                         const pastix_int_t   *col2cblk,
                         bcsc_handle_comm_t   *bcsc_comm )
{
    spm_int_t *globcol;

    /*
     * Allocates and initializes globcol which contains the number of elements in
     * each column of the input matrix.
     * Globcol is equivalent to the classic colptr for the internal blocked
     * csc. The blocked csc integrates the permutation computed within order
     * structure.
     */
    MALLOC_INTERN( globcol, spm->gN+1, pastix_int_t );
    memset( globcol, 0, (spm->gN+1) * sizeof(pastix_int_t) );

    if ( bcsc_comm == NULL ) {
        if ( spm->dof > 0  ) {
            bcsc_init_global_coltab_shm_cdof( spm, ord, globcol );
        }
        else {
            bcsc_init_global_coltab_shm_vdof( spm, ord, globcol );
        }
    }
#if defined(PASTIX_WITH_MPI)
    else {
        if ( spm->dof > 0 ) {
            bcsc_init_global_coltab_dst_cdof( spm, ord, col2cblk, globcol, bcsc_comm );
        }
        else {
            bcsc_init_global_coltab_dst_vdof( spm, ord, col2cblk, globcol, bcsc_comm );
        }

        /* Exchanges the amount of data which will be sent and received. */
        bcsc_exchange_amount_of_data( bcsc_comm );

        /* Stores the indexes and values the current processor has to send to the others. */
        switch( spm->flttype ) {
            case SpmFloat:
                bcsc_sstore_data( spm, ord, col2cblk, bcsc_comm );
                break;
            case SpmDouble:
                bcsc_dstore_data( spm, ord, col2cblk, bcsc_comm );
                break;
            case SpmComplex32:
                bcsc_cstore_data( spm, ord, col2cblk, bcsc_comm );
                break;
            case SpmComplex64:
                bcsc_zstore_data( spm, ord, col2cblk, bcsc_comm );
                break;
            case SpmPattern:
            default:
                fprintf(stderr, "bcsc_init: Error unknown floating type for input spm\n");
        }

        /* Exchanges the indexes and updates globcol with the received indexes. */
        bcsc_exchange_indexes( bcsc_comm );
        bcsc_update_globcol( spm, ord, globcol, bcsc_comm );

#if !defined(NDEBUG)
        /* Check that globcol contains the right information. */
        if ( spm->dof > 0 ) {
            pastix_int_t ig, ip, ipe, dofi;
            pastix_int_t nnzl = 0;
            pastix_int_t nnzg = 0;
            pastix_int_t nnz;
            for( ig=0; ig<spm->gN; ig++ ) {
                ip  = ord->permtab[ig];
                ipe = ( spm->dof > 0 ) ? ip * spm->dof : spm->dofs[ ig ] - spm->baseval;
                if ( col2cblk[ipe] < 0 ) {
                    continue;
                }

                dofi = ( spm->dof > 0 ) ? spm->dof: spm->dofs[ig+1] - spm->dofs[ig];
                nnzl += globcol[ip] * dofi;
            }
            MPI_Allreduce( &nnzl, &nnzg, 1, PASTIX_MPI_INT, MPI_SUM, spm->comm );

            if ( spm->mtxtype != SpmGeneral ) {
                /*
                * We can't check the exact number of elements if some diagonal
                * values are missing (=0).
                */
                nnz = spm->gnnzexp * 2;
                assert( nnzg <= nnz );
                nnz = nnz - (spm->gN * spm->dof * spm->dof);
                assert( nnzg >= nnz );
            }
            else {
                nnz = spm->gnnzexp;
                assert( nnzg == nnz );
            }
        }
#endif
    }

#endif

    (void)solvmtx;
    (void)col2cblk;
    return globcol;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Initializes the coltab of a block csc matrix. The coltab corresponds
 *        to the number of rows (expended) per column (non expended). If the
 *        matrix is distributed in the memory, this function also calls the
 *        routines which exchange the amount of data for the communication,
 *        store the indexes and values to send and exchange the indexes.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The spm structure that stores the dofs.
 *
 * @param[in] ord
 *          The ordering which needs to be applied on the spm to generate the
 *          block csc.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.

 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc stores the initialized coltab split per block
 *          corresponding to the input spm with the permutation applied
 *          and grouped accordingly to the distribution described in solvmtx.
 *
 *******************************************************************************
 *
 * @return The number of non zero unknowns in the matrix.
 *
 *******************************************************************************/
pastix_int_t
bcsc_init_coltab( const spmatrix_t     *spm,
                  const pastix_order_t *ord,
                  const SolverMatrix   *solvmtx,
                  pastix_bcsc_t        *bcsc )
{
    SolverCblk   *cblk;
    bcsc_cblk_t  *blockcol;
    pastix_int_t *dofshift = NULL;
    pastix_int_t *globcol  = NULL;
    pastix_int_t  cblknum, bcscnum, iter, idxcol, nodeidx, colsize;

    bcsc->cscfnbr = solvmtx->cblknbr - solvmtx->faninnbr - solvmtx->recvnbr;
    MALLOC_INTERN( bcsc->cscftab, bcsc->cscfnbr, bcsc_cblk_t );

    /* Creates an array to convert expanded indexes to not expanded indexes. */
    dofshift = bcsc_init_dofshift( spm, ord );

    /* Computes the number of rows (expanded) per column (not expanded). */
    globcol  = bcsc_init_global_coltab( spm, ord, solvmtx, bcsc->col2cblk, bcsc->bcsc_comm );

    idxcol   = 0;
    bcscnum  = 0;
    cblk     = solvmtx->cblktab;
    blockcol = bcsc->cscftab;
    for ( cblknum = 0; cblknum < solvmtx->cblknbr; cblknum++, cblk++ ) {
        if ( cblk->cblktype & (CBLK_FANIN|CBLK_RECV) ) {
            continue;
        }

        blockcol->cblknum = cblknum;
        blockcol->colnbr  = cblk_colnbr( cblk );
        assert( cblk->bcscnum == bcscnum );
        MALLOC_INTERN( blockcol->coltab, blockcol->colnbr + 1, pastix_int_t );

        blockcol->coltab[0] = idxcol;
        for ( iter = 0; iter < blockcol->colnbr; iter++ ) {
            nodeidx = dofshift[ cblk->fcolnum + iter ];
            colsize = globcol[nodeidx];
            //jgpe = cblk->fcolnum + iter;
            //jgp  = dofshift[ jgpe ];
            //colsize = globcol[jgp];
            blockcol->coltab[iter+1] = blockcol->coltab[iter] + colsize;
        }
        idxcol = blockcol->coltab[blockcol->colnbr];

        blockcol++;
        bcscnum++;
    }
    assert( (blockcol - bcsc->cscftab) == bcsc->cscfnbr );
    assert( bcscnum == bcsc->cscfnbr );

    memFree_null( globcol );
    memFree_null( dofshift );

    if ( idxcol > 0 ) {
        MALLOC_INTERN( bcsc->rowtab,  idxcol, pastix_int_t);
        MALLOC_INTERN( bcsc->Lvalues, idxcol * pastix_size_of( bcsc->flttype ), char );
    }
    else {
        bcsc->rowtab  = NULL;
        bcsc->Lvalues = NULL;
    }
    bcsc->Uvalues = NULL;

    return idxcol;
}

/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Restores the coltab array when it has been modified to initialize
 *        the row and values arrays.
 *
 *******************************************************************************
 *
 * @param[inout] bcsc
 *          On entry, the bcsc to restore.
 *          On exit, the coltab array of the bcsc is restored to the correct
 *          indexes.
 *
 *******************************************************************************/
void
bcsc_restore_coltab( pastix_bcsc_t *bcsc )
{
    bcsc_cblk_t *blockcol;
    pastix_int_t index, iter, idxcol, idxcoltmp;

    idxcol   = 0;
    blockcol = bcsc->cscftab;
    for ( index=0; index<bcsc->cscfnbr; index++, blockcol++ )
    {
        for ( iter=0; iter <= blockcol->colnbr; iter++ )
        {
            idxcoltmp = blockcol->coltab[iter];
            blockcol->coltab[iter] = idxcol;
            idxcol = idxcoltmp;
        }
    }
    return;
}

/**
 *******************************************************************************
 *
 * @brief Initializes a block csc.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The initial sparse matrix in the spm format.
 *
 * @param[in] solvmtx
 *          The solver matrix structure which describes the data distribution.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc stores the input spm with the permutation applied
 *          and grouped accordingly to the distribution described in solvmtx.
 *
 *******************************************************************************/
void
bcsc_init_struct( const spmatrix_t   *spm,
                  const SolverMatrix *solvmtx,
                  pastix_bcsc_t      *bcsc )
{
    pastix_int_t       *col2cblk  = NULL;

    bcsc->mtxtype     = spm->mtxtype;
    bcsc->flttype     = spm->flttype;
    bcsc->gN          = spm->gNexp;
    bcsc->n           = solvmtx->nodenbr;
    bcsc->Urowtab     = NULL;
    bcsc->spm2bcsc_L  = NULL;
    bcsc->spm2bcsc_Lt = NULL;
    bcsc->spm2bcsc_U  = NULL;

    /*
     * Creates the col2cblk array which associates each column to a cblk
     * (expanded).
     */
    col2cblk = bcsc_init_col2cblk( solvmtx, bcsc, spm );
    bcsc->col2cblk = col2cblk;

    /*
     * Initializes the coltab array of the bcsc and allocates the rowtab and
     * Lvalues arrays.
     */
    bcsc->bcsc_comm = NULL;
    if ( spm->loc2glob != NULL ) {
        bcsc_handle_comm_init( solvmtx, bcsc );
    }
}

/**
 *******************************************************************************
 *
 * @brief Cleanup the bcsc struct. (symmetric of bcsc_init_struct)
 *
 *******************************************************************************
 *
 * @param[inout] bcsc
 *          On entry, the pointer to the initialized bcsc.
 *          On exit, the bcsc freed from the informations initialized by
 *          bcsc_init_struct().
 *
 *******************************************************************************/
void
bcsc_exit_struct( pastix_bcsc_t *bcsc )
{
    if ( bcsc->col2cblk != NULL ) {
        memFree_null( bcsc->col2cblk );
    }

    if ( bcsc->bcsc_comm != NULL ) {
        bcsc_handle_comm_exit( bcsc->bcsc_comm );
        memFree_null( bcsc->bcsc_comm );
    }
}

/**
 *******************************************************************************
 *
 * @brief Initializes a block csc.
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
 * @param[in] initAt
 *          The test to know if At has to be initialized:
 *          - if initAt = 0 then the matrix is symmetric of hermitian which
 *            means that Lvalues = Uvalues so At does not need to be
 *            initialised.
 *          - if initAt = 1 then the matrix is general and which means that
 *            At needs to be initialised and computed.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc stores the input spm with the permutation applied
 *          and grouped accordingly to the distribution described in solvmtx.
 *
 *******************************************************************************/
static inline void
bcsc_init( const spmatrix_t     *spm,
           const pastix_order_t *ord,
           const SolverMatrix   *solvmtx,
           pastix_int_t          initAt,
           pastix_bcsc_t        *bcsc )
{
    pastix_int_t valuesize;

    bcsc_init_struct( spm, solvmtx, bcsc );
    valuesize = bcsc_init_coltab( spm, ord, solvmtx, bcsc );

    /*
     * Fills in the lower triangular part of the blocked csc with values and
     * rows. The upper triangular part is done later if required through LU
     * factorization.
     */
    switch( spm->flttype ) {
    case SpmFloat:
        bcsc_sinit( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmDouble:
        bcsc_dinit( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmComplex32:
        bcsc_cinit( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmComplex64:
        bcsc_zinit( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmPattern:
    default:
        fprintf(stderr, "bcsc_init: Error unknown floating type for input spm\n");
    }
}

/**
 *******************************************************************************
 *
 * @brief Initializes the block csc matrix.
 *
 * The block csc matrix is used to initialize the factorized matrix, and to
 * perform the matvec operations in refinement.
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
 * @param[in] initAt
 *          The test to know if At has to be initialized:
 *          - if initAt = 0 then the matrix is symmetric of hermitian which
 *            means that Lvalues = Uvalues so At does not need to be
 *            initialised.
 *          - if initAt = 1 then the matrix is general which means that
 *            At needs to be initialised and computed.
 *
 * @param[inout] bcsc
 *          On entry, the pointer to an allocated bcsc.
 *          On exit, the bcsc stores the input spm with the permutation applied
 *          and grouped accordingly to the distribution described in solvmtx.
 *
 *******************************************************************************
 *
 * @return The time spent to initialize the bcsc structure.
 *
 *******************************************************************************/
/**
 *******************************************************************************
 *
 * @ingroup bcsc_internal
 *
 * @brief Binary search for @p target in the sorted array rowtab[lo..hi).
 *
 * @return The index p such that rowtab[p] == target.
 *
 *******************************************************************************/
static inline pastix_int_t
bcsc_find_pos( const pastix_int_t *rowtab, pastix_int_t lo, pastix_int_t hi,
               pastix_int_t target )
{
    while ( lo < hi ) {
        pastix_int_t mid = lo + ( (hi - lo) >> 1 );
        if ( rowtab[mid] < target ) lo = mid + 1;
        else                         hi = mid;
    }
    assert( rowtab[lo] == target );
    return lo;
}

/**
 *******************************************************************************
 *
 * @brief Builds the spm->values[k] → bcsc position scatter maps.
 *
 * For each scalar index k into spm->values (expanded by degrees of freedom),
 * this function records the corresponding position in Lvalues (spm2bcsc_L),
 * in Lvalues for the transposed entry (spm2bcsc_Lt, symmetric/hermitian only),
 * and in Uvalues for the transposed entry (spm2bcsc_U, general LU only).
 *
 * The maps are built via binary search in the already-sorted rowtab / Urowtab
 * arrays produced by bcsc_init().  Entries belonging to a remote MPI rank
 * receive the sentinel value -1; they are handled by the normal exchange path
 * in bcscUpdateValues().  Diagonal entries that are skipped by bcsc_zinit_At()
 * for sym/herm matrices also receive -1 in spm2bcsc_Lt.
 *
 * Must be called after bcsc_init() has completed (rowtab sorted, coltab in
 * column-start state).
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The sparse matrix used for the initial bcscInit() call.
 *
 * @param[in] ord
 *          The ordering (permtab must still be valid).
 *
 * @param[in] solvmtx
 *          The solver matrix (cblktab must still be valid).
 *
 * @param[inout] bcsc
 *          On entry, a fully initialised bcsc with sorted rowtab/Urowtab.
 *          On exit, spm2bcsc_L, spm2bcsc_Lt, and spm2bcsc_U are populated.
 *
 *******************************************************************************/
void
bcsc_build_spm2bcsc( const spmatrix_t     *spm,
                     const pastix_order_t *ord,
                     const SolverMatrix   *solvmtx,
                     pastix_bcsc_t        *bcsc )
{
    pastix_int_t *col2cblk = bcsc->col2cblk;
    pastix_int_t *rowtab   = bcsc->rowtab;
    pastix_int_t *Urowtab  = bcsc->Urowtab;
    pastix_int_t *loc2glob = spm->loc2glob;
    pastix_int_t *colptr   = spm->colptr;
    pastix_int_t *rowptr   = spm->rowptr;
    pastix_int_t *dofs     = spm->dofs;
    pastix_int_t  dof      = spm->dof;
    pastix_int_t  baseval  = spm->baseval;
    pastix_int_t  nnzexp   = spm->nnzexp;
    pastix_int_t  j, kk, k;
    int doLt = ( spm->mtxtype != SpmGeneral );
    int doU  = ( spm->mtxtype == SpmGeneral ) && ( Urowtab != NULL );

    MALLOC_INTERN( bcsc->spm2bcsc_L, nnzexp, pastix_int_t );
    bcsc->spm2bcsc_Lt = NULL;
    bcsc->spm2bcsc_U  = NULL;
    if ( doLt ) { MALLOC_INTERN( bcsc->spm2bcsc_Lt, nnzexp, pastix_int_t ); }
    if ( doU  ) { MALLOC_INTERN( bcsc->spm2bcsc_U,  nnzexp, pastix_int_t ); }

    k = 0;
    for ( j = 0; j < spm->n; j++, colptr++, loc2glob++ ) {
        pastix_int_t jg   = ( spm->loc2glob == NULL ) ? j : *loc2glob - baseval;
        pastix_int_t jgp  = ord->permtab[jg];
        pastix_int_t jgpe = ( dof > 0 ) ? jgp * dof : dofs[jg] - baseval;
        pastix_int_t dofj = ( dof > 0 ) ? dof : dofs[jg+1] - dofs[jg];

        pastix_int_t itercblk_L = col2cblk[jgpe];
        SolverCblk  *cblk_L     = ( itercblk_L >= 0 ) ? solvmtx->cblktab + itercblk_L : NULL;
        pastix_int_t *coltab_L  = ( cblk_L != NULL  ) ? bcsc->cscftab[cblk_L->bcscnum].coltab : NULL;

        for ( kk = colptr[0]; kk < colptr[1]; kk++, rowptr++ ) {
            pastix_int_t ig   = *rowptr - baseval;
            pastix_int_t igp  = ord->permtab[ig];
            pastix_int_t igpe = ( dof > 0 ) ? igp * dof : dofs[ig] - baseval;
            pastix_int_t dofi = ( dof > 0 ) ? dof : dofs[ig+1] - dofs[ig];
            pastix_int_t n_scalars = dofi * dofj;
            pastix_int_t k_base    = k;
            pastix_int_t idofj, idofi;

            /* --- spm2bcsc_L: direct (A) scatter positions --- */
            if ( cblk_L != NULL ) {
                pastix_int_t colidx0 = jgpe - cblk_L->fcolnum;
                for ( idofj = 0; idofj < dofj; idofj++ ) {
                    pastix_int_t colidx = colidx0 + idofj;
                    pastix_int_t lo     = coltab_L[colidx];
                    pastix_int_t hi     = coltab_L[colidx + 1];
                    for ( idofi = 0; idofi < dofi; idofi++, k++ ) {
                        pastix_int_t p = bcsc_find_pos( rowtab, lo, hi, igpe + idofi );
                        bcsc->spm2bcsc_L[k] = p;
                    }
                }
            }
            else {
                pastix_int_t s;
                for ( s = 0; s < n_scalars; s++, k++ ) {
                    bcsc->spm2bcsc_L[k] = -1;  /* remote entry */
                }
            }

            /* --- spm2bcsc_Lt: transposed (At) scatter positions (sym/herm) --- */
            if ( doLt ) {
                if ( ig == jg ) {
                    /* diagonal block: bcsc_zinit_At() skips these */
                    pastix_int_t s;
                    for ( s = 0; s < n_scalars; s++ ) {
                        bcsc->spm2bcsc_Lt[k_base + s] = -1;
                    }
                }
                else {
                    pastix_int_t itercblk_t = col2cblk[igpe];
                    if ( itercblk_t < 0 ) {
                        pastix_int_t s;
                        for ( s = 0; s < n_scalars; s++ ) {
                            bcsc->spm2bcsc_Lt[k_base + s] = -1;
                        }
                    }
                    else {
                        SolverCblk   *cblk_t   = solvmtx->cblktab + itercblk_t;
                        pastix_int_t *coltab_t  = bcsc->cscftab[cblk_t->bcscnum].coltab;
                        pastix_int_t  colidx0_t = igpe - cblk_t->fcolnum;
                        for ( idofj = 0; idofj < dofj; idofj++ ) {
                            pastix_int_t row_t = jgpe + idofj;
                            for ( idofi = 0; idofi < dofi; idofi++ ) {
                                pastix_int_t colidx_t = colidx0_t + idofi;
                                pastix_int_t lo = coltab_t[colidx_t];
                                pastix_int_t hi = coltab_t[colidx_t + 1];
                                pastix_int_t p  = bcsc_find_pos( rowtab, lo, hi, row_t );
                                bcsc->spm2bcsc_Lt[k_base + idofj * dofi + idofi] = p;
                            }
                        }
                    }
                }
            }

            /* --- spm2bcsc_U: transposed (At) scatter positions (general LU) --- */
            if ( doU ) {
                pastix_int_t itercblk_t = col2cblk[igpe];
                if ( itercblk_t < 0 ) {
                    pastix_int_t s;
                    for ( s = 0; s < n_scalars; s++ ) {
                        bcsc->spm2bcsc_U[k_base + s] = -1;
                    }
                }
                else {
                    SolverCblk   *cblk_t   = solvmtx->cblktab + itercblk_t;
                    pastix_int_t *coltab_t  = bcsc->cscftab[cblk_t->bcscnum].coltab;
                    pastix_int_t  colidx0_t = igpe - cblk_t->fcolnum;
                    for ( idofj = 0; idofj < dofj; idofj++ ) {
                        pastix_int_t row_t = jgpe + idofj;
                        for ( idofi = 0; idofi < dofi; idofi++ ) {
                            pastix_int_t colidx_t = colidx0_t + idofi;
                            pastix_int_t lo = coltab_t[colidx_t];
                            pastix_int_t hi = coltab_t[colidx_t + 1];
                            pastix_int_t p  = bcsc_find_pos( Urowtab, lo, hi, row_t );
                            bcsc->spm2bcsc_U[k_base + idofj * dofi + idofi] = p;
                        }
                    }
                }
            }
        }
    }
    assert( k == nnzexp );
    (void)solvmtx;
}

double
bcscInit( const spmatrix_t     *spm,
          const pastix_order_t *ord,
          const SolverMatrix   *solvmtx,
          pastix_int_t          initAt,
          pastix_bcsc_t        *bcsc )
{
    double time = 0.;

    assert( ord->baseval == 0 );
    assert( ord->vertnbr == spm->gN );

    clockStart(time);
    bcsc_init( spm, ord, solvmtx, initAt, bcsc );
    bcsc_build_spm2bcsc( spm, ord, solvmtx, bcsc );
    clockStop(time);

    return time;
}

/**
 *******************************************************************************
 *
 * @brief Updates only the values arrays of an already initialised block csc.
 *
 * This function is used to re-populate bcsc->Lvalues and bcsc->Uvalues from
 * a new spm->values array when the sparsity pattern has not changed since the
 * last bcscInit() call.  All structural data (col2cblk, coltab, rowtab) is
 * reused without modification.
 *
 *******************************************************************************
 *
 * @param[in] spm
 *          The sparse matrix with updated values.  Must have the same sparsity
 *          pattern as the spm used in the previous bcscInit() call.
 *
 * @param[in] ord
 *          The ordering (unchanged from the original factorization).
 *
 * @param[in] solvmtx
 *          The solver matrix structure (unchanged).
 *
 * @param[in] initAt
 *          Non-zero if Uvalues must be updated (PastixFactLU case).
 *
 * @param[inout] bcsc
 *          On entry, the fully initialised bcsc (coltab in column-start state).
 *          On exit, Lvalues and Uvalues contain the updated values.
 *
 *******************************************************************************
 *
 * @return The time spent updating the bcsc values.
 *
 *******************************************************************************/
double
bcscUpdateValues( const spmatrix_t     *spm,
                  const pastix_order_t *ord,
                  const SolverMatrix   *solvmtx,
                  pastix_int_t          initAt,
                  pastix_bcsc_t        *bcsc )
{
    double        time = 0.;
    pastix_int_t  valuesize;
    bcsc_cblk_t  *last;

    assert( ord->baseval == 0 );
    assert( ord->vertnbr == spm->gN );
    assert( bcsc != NULL );
    assert( bcsc->cscftab != NULL );

    /* Recover total entry count from the existing coltab (column-start state). */
    last      = bcsc->cscftab + bcsc->cscfnbr - 1;
    valuesize = last->coltab[ last->colnbr ];

    clockStart(time);
    switch( spm->flttype ) {
    case SpmFloat:
        bcsc_supdatevalues( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmDouble:
        bcsc_dupdatevalues( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmComplex32:
        bcsc_cupdatevalues( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmComplex64:
        bcsc_zupdatevalues( spm, ord, solvmtx, initAt, bcsc, valuesize );
        break;
    case SpmPattern:
    default:
        fprintf(stderr, "bcscUpdateValues: Error unknown floating type for input spm\n");
    }
    clockStop(time);

    return time;
}

/**
 *******************************************************************************
 *
 * @brief Frees the block csc structure but do not free the bcsc pointer.
 *
 *******************************************************************************
 *
 * @param[inout] bcsc
 *          The block csc matrix to free.
 *
 *******************************************************************************/
void
bcscExit( pastix_bcsc_t *bcsc )
{
    bcsc_cblk_t *cblk;
    pastix_int_t i;

    if ( bcsc->cscftab == NULL ) {
        return;
    }

    for ( i=0, cblk=bcsc->cscftab; i < bcsc->cscfnbr; i++, cblk++ ) {
        memFree_null( cblk->coltab );
    }

    memFree_null( bcsc->cscftab );
    memFree_null( bcsc->rowtab );
    memFree_null( bcsc->Urowtab );

    if ( (bcsc->Uvalues != NULL) &&
         (bcsc->Uvalues != bcsc->Lvalues) ) {
        memFree_null( bcsc->Uvalues );
    }

    memFree_null( bcsc->Lvalues );
    memFree_null( bcsc->spm2bcsc_L );
    memFree_null( bcsc->spm2bcsc_Lt );
    memFree_null( bcsc->spm2bcsc_U );

    bcsc_exit_struct( bcsc );
}
