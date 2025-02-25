/**
 *
 * @file solver_matrix_gen_utils.c
 *
 * PaStiX solver structure generation functions to factorize
 * solver_matric_gen.c .
 *
 * @copyright 1998-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Tony Delarue
 * @author Pascal Henon
 * @author Pierre Ramet
 * @author Xavier Lacoste
 * @author Mathieu Faverge
 * @author Alycia Lisito
 * @author Nolan Bredel
 * @date 2024-07-05
 *
 * @addtogroup blend_dev_solver
 * @{
 *
 **/
#include "common.h"
#include "symbol/symbol.h"
#include "blend/solver.h"
#include "elimintree.h"
#include "cost.h"
#include "cand.h"
#include "pastix/order.h"
#include "extendVector.h"
#include "simu.h"
#include "solver_matrix_gen_utils.h"

/**
 *******************************************************************************
 *
 * @brief Fill the local numbering arrays to compress the symbol information
 *        into solver.
 *
 *******************************************************************************
 *
 * @param[in] symbmtx
 *          The pointer to the symbol matrix structure.
 *
 * @param[in] simuctrl
 *          The pointer to the simuctrl structure.
 *
 *  @param[inout] solvmtx
 *          Pointer to the solver matrix.
 *
 * @param[inout] cblklocalnum
 *          Local cblk infos.
 *
 * @param[inout] bloklocalnum
 *          Local blok infos.
 *
 * @param[inout] tasklocalnum
 *          Local tasks infos.
 *
 * @param[inout] ftgttab
 *          Array of fan-in to store the lists of recv/fanin cblk per local cblk.
 *
 *******************************************************************************/
void
solvMatGen_fill_localnums( const symbol_matrix_t *symbmtx,
                           const SimuCtrl        *simuctrl,
                           SolverMatrix          *solvmtx,
                           pastix_int_t          *cblklocalnum,
                           pastix_int_t          *bloklocalnum,
                           pastix_int_t          *tasklocalnum,
                           solver_cblk_recv_t   **ftgttab,
                           pastix_int_t          *faninnbr_tab )
{
    pastix_int_t  *localindex;
    symbol_cblk_t *symbcblk;
    symbol_blok_t *symbblok;
    pastix_int_t   cblknum, brownum, brownbr;
    pastix_int_t   faninnbr, recvnbr, faninbloknbr;
    pastix_int_t   i, j, k, c, fc;
    pastix_int_t   flaglocal;
    pastix_int_t   clustnum = solvmtx->clustnum;
    pastix_int_t   clustnbr = solvmtx->clustnbr;

    /* Initialize the set of cluster candidates for each cblk */
    MALLOC_INTERN( localindex, clustnbr, pastix_int_t );

    /*
     * Compute local number of tasks on each cluster
     */
    memset( localindex, 0, clustnbr * sizeof(pastix_int_t) );
    for ( i = 0; i < simuctrl->tasknbr; i++ ) {
        c = simuctrl->bloktab[ simuctrl->tasktab[i].bloknum ].ownerclust;

        tasklocalnum[i] = localindex[c];
        localindex[c]++;
    }
    solvmtx->tasknbr = localindex[clustnum];

    /*
     * Compute the local numbering of the fan-in and recv cblks on each cluster
     */
    /* Reset the array to compute local informations */
    memset( localindex, 0, clustnbr * sizeof( pastix_int_t ) );

    cblknum      = 0;
    brownum      = 0;
    recvnbr      = 0;
    faninnbr     = 0;
    faninbloknbr = 0;
    symbcblk = symbmtx->cblktab;
    for ( i = 0; i < symbmtx->cblknbr; i++, symbcblk++ ) {
        brownbr = symbcblk[1].brownum - symbcblk[0].brownum;

        /*
         * The cblk is considered local if data are local, or if we store a
         * compressed copy for fanin
         */
        flaglocal = ( simuctrl->cblktab[i].owned ) || ( ftgttab[i] != NULL );
        if ( !flaglocal ) {
            cblklocalnum[i] = -i - 1;
#if !defined(NDEBUG)
            for ( j=symbcblk[0].bloknum; j<symbcblk[1].bloknum; j++ ) {
                bloklocalnum[j] = -1;
                assert( simuctrl->bloktab[j].ownerclust != clustnum );
            }
#endif
            continue;
        }

        /*
         * The cblk is local.
         */
        if ( simuctrl->cblktab[i].owned ) {
            solver_cblk_recv_t *ftgtcblk;

            /*
             * The cblk is local. We may receive remote information, let's:
             *    - compute the size of the compressed browtab
             *    - compute the number of remote fanin to be received for the update
             *    - compute the set of remote nodes sending a fanin
             *
             * To do that, we work on the incoming edges.
             */
            for ( j = symbcblk[0].brownum; j < symbcblk[1].brownum; j++ ) {
                k = symbmtx->browtab[j];
                symbblok = symbmtx->bloktab + k;
                c = simuctrl->bloktab[k].ownerclust;

                assert( i == symbblok->fcblknm );

                /* This is a remote contribution we add it to the ftgt and update the counters */
                if ( c != clustnum ) {
                    solver_recv_update_recv( ftgttab + i,
                                             symbmtx,
                                             symbmtx->cblktab + symbblok->lcblknm,
                                             symbblok, symbcblk, c );
                    brownbr--;
                }
                assert( brownbr >= 0 );
            }

            /*
             * Now that all remote contributions have been computed and summarized in ftgttab[i],
             * let's compute the local information for the indices
             */
            ftgtcblk = ftgttab[i];
            while( ftgtcblk != NULL ) {
                assert( (ftgtcblk->ownerid != -1) &&
                        (ftgtcblk->ownerid != clustnum) );

                /* Book some space for the reception blocks */
                localindex[clustnum] += solver_recv_get_bloknbr( ftgtcblk, symbcblk,
                                                                 symbmtx->bloktab + symbcblk->bloknum );

                brownbr++; /* One more blok will be in the browtab */
                cblknum++; /* Add one cblk                         */
                recvnbr++; /* Add one reception count              */

                ftgtcblk = ftgtcblk->next;
            }

            /*
             * Now, we need to get through the outgoing dependencies to generate
             * the fanin informations if it needs to be added, and to compute
             * local block indices.
             */
            symbblok = symbmtx->bloktab + symbcblk->bloknum;
            for ( j=symbcblk[0].bloknum; j<symbcblk[1].bloknum; j++, symbblok++ ) {
                symbol_cblk_t *symbfcbk;
                pastix_int_t fcblknum, fbloknum;

                bloklocalnum[j] = localindex[clustnum];
                localindex[clustnum]++;

                assert( simuctrl->bloktab[j].ownerclust == clustnum );

                fcblknum = symbblok->fcblknm;
                symbfcbk = symbmtx->cblktab + fcblknum;
                fbloknum = symbfcbk->bloknum;
                fc = simuctrl->bloktab[fbloknum].ownerclust;

                /*
                 * If the facing cblk isn't local, we need to have a local copy
                 * of it to store the fan-in
                 */
                if ( fc != clustnum ) {
                    solver_recv_update_fanin( ftgttab + fcblknum,
                                              symbmtx, symbcblk, symbblok, symbfcbk, fc );
                }
            }
        }
        else {
            /* If the cblk is not local, it is a fanin */

            /*
             * First, let's look at incoming dependencies to reduce the brownbr
             */
            {
                for ( j = symbcblk[0].brownum; j < symbcblk[1].brownum; j++ ) {
                    k = symbmtx->browtab[j];
                    c = simuctrl->bloktab[k].ownerclust;
                    if ( c != clustnum ) {
                        brownbr--;
                    }
                }
            }

            /*
             * Second, let's update the localindex counter based on the number of local blocks
             */
            {
                /* Check we have one and only one solver_cblk_recv associated to it */
                solver_cblk_recv_t *ftgtcblk = ftgttab[i];
                solver_blok_recv_t *ftgtblok;
                assert( ftgtcblk       != NULL );
                assert( ftgtcblk->next == NULL );

                symbblok = symbmtx->bloktab + symbcblk->bloknum;
                faninnbr++;
                /* Adds a fanin cblk to ftgtcblk->ownerid cblk fanin counter. */
                faninnbr_tab[2 * ftgtcblk->ownerid]++;
                ftgtblok = ftgtcblk->bloktab;
                for ( j=symbcblk[0].bloknum; j<symbcblk[1].bloknum; j++, symbblok++, ftgtblok++ )
                {
                    assert( simuctrl->bloktab[j].ownerclust != clustnum );

                    if ( (ftgtblok->frownum <= ftgtblok->lrownum) &&
                         (ftgtblok->frownum >= symbblok->frownum) &&
                         (ftgtblok->lrownum <= symbblok->lrownum) )
                    {
                        bloklocalnum[j] = localindex[clustnum];
                        localindex[clustnum]++;
                        faninbloknbr++;
                        /* Adds a fanin blok to ftgtcblk->ownerid blok fanin counter. */
                        faninnbr_tab[2 * ftgtcblk->ownerid + 1]++;
                    }
                    else {
                        bloklocalnum[j] = -1;
                    }
                }
            }
        }

        /* Store index of the current cblk */
        cblklocalnum[i] = cblknum;
        cblknum++;

        /* Update the brownum index */
        brownum += brownbr;
        assert( brownum <= symbcblk[1].brownum );
    }

    solvmtx->cblknbr = cblknum;
    solvmtx->bloknbr = localindex[clustnum];
    solvmtx->brownbr = brownum;

    /* Reallocate recv_sources tab to diminish it's size */
    solvmtx->recvnbr  = recvnbr;
    solvmtx->faninnbr = faninnbr;

#if defined(PASTIX_WITH_MPI)
    if ( clustnbr > 1 ) {
        pastix_int_t fanincnt, fanin_tmp;
        pastix_int_t faninblokcnt, faninblok_tmp;
        pastix_int_t gfaninnbr[2];
        pastix_int_t fanin_tab[4];

        fanin_tab[0] = faninnbr;
        fanin_tab[1] = faninbloknbr;
        fanin_tab[2] = 0;
        fanin_tab[3] = 0;

        if ( clustnum != 0 ) {
            /* Receives global number of cblk fanin: fanin_tab[2] and blok fanin: fanin_tab[3]. */
            MPI_Recv( &fanin_tab[2], 2, PASTIX_MPI_INT, clustnum-1, clustnum-1,
                      solvmtx->solv_comm, MPI_STATUS_IGNORE);
            fanin_tab[0] += fanin_tab[2];
            fanin_tab[1] += fanin_tab[3];
        }
        if ( clustnum != clustnbr - 1 ) {
            /* Sends global number of cblk fanin: fanin_tab[1] and blok fanin: fanin_tab[2]. */
            MPI_Send( &fanin_tab[0], 2, PASTIX_MPI_INT, clustnum+1, clustnum,
                      solvmtx->solv_comm );
        }

        /* Shares the global number of fanin. */
        gfaninnbr[0] = fanin_tab[0];
        gfaninnbr[1] = fanin_tab[1];
        MPI_Bcast( &gfaninnbr, 2, PASTIX_MPI_INT, clustnbr-1, solvmtx->solv_comm );
        solvmtx->gfanincblknbr = gfaninnbr[0];
        solvmtx->gfaninbloknbr = gfaninnbr[1];

        fanincnt     = fanin_tab[2];
        faninblokcnt = fanin_tab[3];
        for ( k = 0; k < clustnbr; k++ ) {
            if ( faninnbr_tab[2 * k] == 0 ) {
                /* No fanin cblk for k. */
                faninnbr_tab[2 * k]     = -1;
                /* No fanin blok for k. */
                faninnbr_tab[2 * k + 1] = -1;
            }
            /* Adds fanin cblk (and blok) indexes for k. */
            if ( faninnbr_tab[2 * k] != -1 ) {
                /* Adds fanin cblk index for k. */
                fanin_tmp = faninnbr_tab[2 * k];
                faninnbr_tab[ 2 * k] = fanincnt;
                fanincnt += fanin_tmp;
                /* Adds fanin blok index for k. */
                faninblok_tmp = faninnbr_tab[2 * k + 1];
                faninnbr_tab[ 2 * k + 1] = faninblokcnt;
                faninblokcnt += faninblok_tmp;
            }
        }

        /* Exchanges fanin cblk and blok indexes. */
        MPI_Alltoall( faninnbr_tab, 2, PASTIX_MPI_INT, faninnbr_tab+2*clustnbr,
                      2, PASTIX_MPI_INT, solvmtx->solv_comm );
    }
#endif

    memFree_null( localindex );
    (void)faninbloknbr;
}

/**
 *******************************************************************************
 *
 * @brief Reorder the browtab from the symbol structure in a distributed way.
 *        First stock the 1D blocks and then the 2D blocks.
 *
 *******************************************************************************
 *
 * @param[in] symbmtx
 *          The pointer to the symbol matrix structure.
 *
 * @param[in] symbcblk
 *          The pointer to the current symbol cblk.
 *
 *  @param[inout] solvmtx
 *          Pointer to the solver matrix.
 *
 * @param[inout] solvcblk
 *          The pointer to the current solver cblk.
 *
 * @param[inout] browtmp
 *          Workspace array used to reorder the local brow information.
 *          Must be of size at least (symbcblk[1].brownum - symbcblk[0].brownum)
 *
 * @param[in] cblklocalnum
 *          Local cblk indices.
 *
 * @param[in] bloklocalnum
 *          Local blok indices.
 *
 * @param[in] brownum
 *         Current brownum.
 *
 *******************************************************************************
 *
 * @retval TODO
 *
 *******************************************************************************/
pastix_int_t
solvMatGen_reorder_browtab( const symbol_matrix_t *symbmtx,
                            const symbol_cblk_t   *symbcblk,
                            SolverMatrix          *solvmtx,
                            SolverCblk            *solvcblk,
                            pastix_int_t          *browtmp,
                            const pastix_int_t    *cblklocalnum,
                            const pastix_int_t    *bloklocalnum,
                            pastix_int_t           brownum )
{
    pastix_int_t   brownbr;
    symbol_blok_t *symbblok;
    SolverBlok    *solvblok;
    SolverCblk    *browcblk;
    pastix_int_t   lcblknm, lbloknm;
    pastix_int_t   j2d, j1d, j, jmax;
    pastix_int_t   *b;

    brownbr = symbcblk[1].brownum - symbcblk[0].brownum;
    solvcblk->brown2d = solvcblk->brownum + brownbr;

    /* Nothing to do here */
    if ( !brownbr ) {
        return 0;
    }

    assert( brownbr <= symbmtx->browmax );
    memcpy( browtmp,
            symbmtx->browtab + symbcblk->brownum,
            brownbr * sizeof(pastix_int_t) );

    /*
     * j   is the index in the local browtab (~ postition of b)
     * j1d is the number of discovered 1d block in the browtab
     * j2d if the index of the first 2d block in the original tab
     * jmax is equal to brownbr
     * brownbr is updated to store the real number of brow (minus fanin)
     *
     * b is a pointer to the temporary copy of the subsection of the browtab
     * At the end of the first pass, if b[i] is negative, it has already been treated
     * with if equal to:
     *       -1, the block was a 1D block
     *       -2, the block belonged to a remote cblk
     *       -3, the block belonged to a local fanin (should not happen)
     * It is is positive, it's a 2D block that need to be pushed to the end of
     * the browtab in the second pass.
     */
    b = browtmp;
    j2d = -1;
    jmax = brownbr;

    /* First pass to copy 1D updates */
    for ( j=0, j1d=0; j < jmax; j++, b++ ) {
        /* Get the contributing block in the symbol */
        symbblok = symbmtx->bloktab + (*b);

        lcblknm = ( cblklocalnum == NULL ) ? symbblok->lcblknm : cblklocalnum[ symbblok->lcblknm ];

        /* If distant blok */
        if ( lcblknm < 0 ) {
            *b = -2;
            brownbr--;
            continue;
        }

        /* Get the local cblk which owns the block */
        browcblk = solvmtx->cblktab + lcblknm;

        /* Recv should never appear through cblklocalnum */
        assert( !(browcblk->cblktype & CBLK_RECV) );

        /* Fanin should not contribute to local data */
        if( browcblk->cblktype & CBLK_FANIN ) {
            *b = -3;
            brownbr--;
            continue;
        }

        /* Store the first non 1D index to not rediscover the begining, and skip 2d for now */
        if ( browcblk->cblktype & CBLK_TASKS_2D ) {
            j2d = ( j2d == -1 ) ? j : j2d;
            continue;
        }

        /* Find the SolvBlok corresponding to the SymbBlok */
        lbloknm = ( bloklocalnum == NULL ) ? *b : bloklocalnum[ *b ];
        solvblok = solvmtx->bloktab + lbloknm;

        assert( solvblok->lcblknm == lcblknm );
#if !defined(NDEBUG)
        {
            pastix_int_t frownum, lrownum;
            symbol_blok_get_rownum( symbmtx, symbblok, &frownum, &lrownum );
            assert( ( frownum == solvblok->frownum ) &&
                    ( lrownum == solvblok->lrownum ) );
        }
#endif

        assert( brownum + j1d < solvmtx->brownbr );
        solvmtx->browtab[brownum + j1d] = lbloknm;
        solvblok->browind = brownum + j1d;
        *b = -1;
        j1d++;
    }

    /* Store the index of the first 2D contribution in the array */
    assert( j1d <= brownbr );
    solvcblk->brown2d = solvcblk->brownum + j1d;

    /* Second pass to copy 2D updates to the end */
    if ( j2d != -1 ) {
        b = browtmp + j2d;
        for ( j = j2d; j < jmax; j++, b++ ) {
            symbblok = symbmtx->bloktab + ( *b );

            if ( *b < 0 ) {
                continue;
            }
            lcblknm = ( cblklocalnum == NULL ) ? symbblok->lcblknm : cblklocalnum[ symbblok->lcblknm ];
            assert( lcblknm >= 0 );

            /* Get the local cblk which owns the block */
            browcblk = solvmtx->cblktab + lcblknm;
            assert( (cblklocalnum == NULL) ||
                    (browcblk->ownerid == solvmtx->clustnum) );

            /* Find the SolvBlok corresponding to the SymbBlok */
            lbloknm = ( bloklocalnum == NULL ) ? *b : bloklocalnum[ *b ];
            solvblok = solvmtx->bloktab + lbloknm;

            assert( solvblok->lcblknm == lcblknm );
            assert( ( symbblok->frownum == solvblok->frownum ) &&
                    ( symbblok->lrownum == solvblok->lrownum ) );

            assert( brownum + j1d < solvmtx->brownbr );
            solvmtx->browtab[brownum + j1d] = lbloknm;
            solvblok->browind = brownum + j1d;
            j1d++;
        }
    }
    assert( j1d == brownbr );

    return brownbr;
}

/**
 * @brief Structure to pass information to the muti-threaded ttsktab
 *        initialization function.
 */
struct args_ttsktab
{
    SolverMatrix       *solvmtx;      /**< Pointer to the solver matrix            */
    const SimuCtrl     *simuctrl;     /**< Pointer to simulation control structure */
    const pastix_int_t *tasklocalnum; /**< Array of the local indices of the tasks */
    pastix_int_t        clustnum;     /**< Index of the local cluster              */
};

/**
 *******************************************************************************
 *
 * @brief Fill the ttsktab for it's own thread.
 *
 *******************************************************************************
 *
 * @param[in] ctx
 *          The context of the current thread
 *
 * @param[inout] args
 *          The pointer to the args_ttsktab structure that parameterize the
 *          function call.
 *
 *******************************************************************************/
void
solvMatGen_fill_ttsktab( isched_thread_t *ctx, void *args )
{
    struct args_ttsktab *arg          = (struct args_ttsktab*)args;
    SolverMatrix        *solvmtx      = arg->solvmtx;
    const SimuCtrl      *simuctrl     = arg->simuctrl;
    const pastix_int_t  *tasklocalnum = arg->tasklocalnum;
    pastix_int_t         clustnum     = arg->clustnum;
    int                  rank         = ctx->rank;
    SimuProc            *simuproc     = simuctrl->proctab
        + ( simuctrl->clustab[clustnum].fprocnum + rank );
    pastix_int_t i;
    pastix_int_t priomin = PASTIX_INT_MAX;
    pastix_int_t priomax = 0;
    pastix_int_t ttsknbr = extendint_Size( simuproc->tasktab );
    pastix_int_t j, jloc;

    solvmtx->ttsknbr[rank] = ttsknbr;
    if(ttsknbr > 0) {
        MALLOC_INTERN(solvmtx->ttsktab[rank], ttsknbr, pastix_int_t);
    }
    else {
        solvmtx->ttsktab[rank] = NULL;
    }

    for(i=0; i<ttsknbr; i++)
    {
        j = extendint_Read(simuproc->tasktab, i);
        if( tasklocalnum != NULL ){
            jloc = tasklocalnum[j];
        }
        else {
            jloc = j;
        }
        /* Only local cblks should appear in the tasktab */
        assert( !(solvmtx->cblktab[ solvmtx->tasktab[jloc].cblknum ].cblktype & (CBLK_FANIN|CBLK_RECV)) );
        solvmtx->ttsktab[rank][i] = jloc;
        solvmtx->cblktab[jloc].threadid = rank;

#if defined(PASTIX_DYNSCHED)
        solvmtx->tasktab[jloc].threadid = rank;
#endif
        priomax = pastix_imax( solvmtx->tasktab[jloc].prionum, priomax );
        priomin = pastix_imin( solvmtx->tasktab[jloc].prionum, priomin );
    }

#if defined(PASTIX_DYNSCHED)
    solvmtx->btree->nodetab[rank].priomin = priomin;
    solvmtx->btree->nodetab[rank].priomax = priomax;
#endif
}

/**
 *******************************************************************************
 *
 * @brief Fill in ttsktab for it's own thread. Only for debugging factorization.
 *
 *******************************************************************************
 *
 * @param[in] ctx
 *          the context of the current thread
 *
 * @param[inout] args
 *          The pointer to the args_ttsktab structure that parameterize the
 *          function call.
 *
 *******************************************************************************/
void
solvMatGen_fill_ttsktab_dbg( isched_thread_t *ctx, void *args )
{
    struct args_ttsktab *arg = (struct args_ttsktab*)args;

    pastix_int_t  i, j, size;
    SolverMatrix *solvmtx = arg->solvmtx;
    int           rank    = ctx->rank;
    int           nthread = ctx->global_ctx->world_size;
    pastix_int_t  tasknbr = solvmtx->tasknbr / nthread;
    pastix_int_t  priomin = PASTIX_INT_MAX;
    pastix_int_t  priomax = 0;

    size = (rank == nthread-1) ? (solvmtx->tasknbr - (nthread-1) * tasknbr) : tasknbr;
    solvmtx->ttsknbr[rank] = size;

    if(size > 0) {
        MALLOC_INTERN(solvmtx->ttsktab[rank], size, pastix_int_t);
    }
    else {
        solvmtx->ttsktab[rank] = NULL;
    }

    j = ((solvmtx->tasknbr - (nthread-1) * tasknbr) * rank);
    for(i=0; i < size; i++)
    {
        solvmtx->ttsktab[rank][i] = j;

#if defined(PASTIX_DYNSCHED)
        solvmtx->tasktab[j].threadid = rank;
#endif
        priomax = pastix_imax( solvmtx->tasktab[j].prionum, priomax );
        priomin = pastix_imin( solvmtx->tasktab[j].prionum, priomin );
        j++;
    }

#if defined(PASTIX_DYNSCHED)
    solvmtx->btree->nodetab[rank].priomin = priomin;
    solvmtx->btree->nodetab[rank].priomax = priomax;
#endif
}

/**
 *******************************************************************************
 *
 * @brief Fill the global tasktab array, as well as the thread ttsktab arrays.
 *
 *******************************************************************************
 *
 * @param[inout] solvmtx
 *          Pointer to the solver matrix.
 *
 * @param[in] isched
 *          The internal context to run multi-threaded functions.
 *
 * @param[in] simuctrl
 *          The pointer to the simulation control structure.
 *
 * @param[in] tasklocalnum
 *          Array of the local indices of the tasks.
 *
 * @param[in] cblklocalnum
 *          Array of the local indices of the cblk.
 *
 * @param[in] bloklocalnum
 *          Array of the local indices of the blocks.
 *
 * @param[in] clustnum
 *          Rank of the MPI instance.
 *
 * @param[in] is_dbg
 *          Enable/disable the ttsktab debug generation.
 *
 *******************************************************************************/
void
solvMatGen_fill_tasktab( SolverMatrix       *solvmtx,
                         isched_t           *isched,
                         const SimuCtrl     *simuctrl,
                         const pastix_int_t *tasklocalnum,
                         const pastix_int_t *cblklocalnum,
                         const pastix_int_t *bloklocalnum,
                         pastix_int_t        clustnum,
                         int                 is_dbg )
{
    Task         *solvtask;
    SolverBlok   *blok, *lblk;
    SimuTask     *simutask    = simuctrl->tasktab;
    SolverCblk   *solvcblk    = solvmtx->cblktab;
    pastix_int_t  tasknum     = 0;
    pastix_int_t  tasknbr_1dp = 0;
    pastix_int_t  i;

    MALLOC_INTERN( solvmtx->tasktab, solvmtx->tasknbr+1, Task );
    solvtask = solvmtx->tasktab;

    /* No local indices, this is a global solver */
    if ( tasklocalnum == NULL )
    {
        for(i=0; i<simuctrl->tasknbr; i++, simutask++)
        {
            solvtask->taskid  = COMP_1D;
            solvtask->prionum = simutask->prionum;
            solvtask->cblknum = simutask->cblknum;
            solvtask->bloknum = simutask->bloknum;
            solvtask->ctrbcnt = simutask->ctrbcnt;

            solvcblk = solvmtx->cblktab + solvtask->cblknum;
            solvcblk->priority = solvtask->prionum;

            /* Schur, fanin and recv are counted only in static */
            if ( solvcblk->cblktype & (CBLK_IN_SCHUR | CBLK_FANIN | CBLK_RECV ) ) {
                tasknum++; solvtask++;
                continue;
            }

            /* Count the number of additional tasks in 1D+ mode */
            if ( solvcblk->cblktype & CBLK_TASKS_2D )
            {
                blok = solvcblk[0].fblokptr + 1; /* first off-diagonal block */
                lblk = solvcblk[1].fblokptr;     /* the next diagonal block  */

                /* if there are off-diagonal supernodes in the column */
                for( ; blok < lblk; blok++ ) {
                    tasknbr_1dp++;
                    /* Skip blocks facing the same cblk */
                    while ( ( blok < lblk ) &&
                            ( blok[0].fcblknm == blok[1].fcblknm ) &&
                            ( blok[0].lcblknm == blok[1].lcblknm ) )
                    {
                        blok++;
                    }
                }
            }
            tasknbr_1dp++;
            tasknum++; solvtask++;
        }
    }
    else
    {
        for(i=0; i<simuctrl->tasknbr; i++, simutask++)
        {
            if( simuctrl->bloktab[ simutask->bloknum ].ownerclust != clustnum )
            {
                continue;
            }

            assert( tasknum == tasklocalnum[i] );

            solvtask->taskid  = COMP_1D;
            solvtask->prionum = simutask->prionum;
            solvtask->cblknum = cblklocalnum[ simutask->cblknum ];
            solvtask->bloknum = bloklocalnum[ simutask->bloknum ];
            solvtask->ctrbcnt = simutask->ctrbcnt;

            solvcblk = solvmtx->cblktab + solvtask->cblknum;
            solvcblk->priority = solvtask->prionum;

            /* Schur, fanin and recv are counted only in static */
            if ( solvcblk->cblktype & (CBLK_IN_SCHUR | CBLK_FANIN | CBLK_RECV ) ) {
                tasknum++; solvtask++;
                continue;
            }

            /* Count the number of additional tasks in 1D+ mode */
            if ( solvcblk->cblktype & CBLK_TASKS_2D )
            {
                blok = solvcblk[0].fblokptr + 1; /* first off-diagonal block */
                lblk = solvcblk[1].fblokptr;     /* the next diagonal block  */

                /* if there are off-diagonal supernodes in the column */
                for( ; blok < lblk; blok++ ) {
                    tasknbr_1dp++;
                    /* Skip blocks facing the same cblk */
                    while ( ( blok < lblk ) &&
                            ( blok[0].fcblknm == blok[1].fcblknm ) &&
                            ( blok[0].lcblknm == blok[1].lcblknm ) )
                    {
                        blok++;
                    }
                }
            }

            tasknbr_1dp++;
            tasknum++; solvtask++;
        }
    }
    assert(tasknum == solvmtx->tasknbr);
    solvmtx->tasknbr_1dp = tasknbr_1dp;

    /* One more task to avoid side effect */
    solvtask->taskid  = -1;
    solvtask->prionum = -1;
    solvtask->cblknum = solvmtx->cblknbr+1;
    solvtask->bloknum = solvmtx->bloknbr+1;
    solvtask->ctrbcnt = 0;

    /* Fill in the ttsktab arrays (one per thread) */
    MALLOC_INTERN(solvmtx->ttsknbr, solvmtx->bublnbr, pastix_int_t  );
    MALLOC_INTERN(solvmtx->ttsktab, solvmtx->bublnbr, pastix_int_t* );

    if( is_dbg ) {
        struct args_ttsktab args = { solvmtx, NULL, tasklocalnum, clustnum };
        isched_parallel_call( isched, solvMatGen_fill_ttsktab_dbg, &args );
    }
    else {
        struct args_ttsktab args = { solvmtx, simuctrl, tasklocalnum, clustnum };
        isched_parallel_call( isched, solvMatGen_fill_ttsktab, &args );
    }
}

/**
 *******************************************************************************
 *
 * @brief Compute the maximum area of the temporary buffers
 *        used during computation
 *
 * During this loop, we compute the maximum area that will be used as
 * temporary buffers, and statistics:
 *    - diagmax: Only for hetrf/sytrf factorization, this the maximum size
 *               of a panel of MAXSIZEOFBLOCKS width in a diagonal block
 *    - gemmmax: For all, this is the maximum area used to compute the
 *               compacted gemm on a CPU.
 *
 * Rk: This loop is not merged within the main block loop, since strides have
 * to be peviously computed.
 *
 *******************************************************************************
 *
 *  @param[inout] solvmtx
 *           Pointer to the solver matrix.
 *
 *******************************************************************************/
void
solvMatGen_max_buffers( SolverMatrix *solvmtx )
{
    SolverCblk  *solvcblk = solvmtx->cblktab;
    SolverBlok  *solvblok = solvmtx->bloktab;
    pastix_int_t gemmmax = 0;
    pastix_int_t offdmax = 0;
    pastix_int_t blokmax = 0;
    pastix_int_t gemmarea, offdarea, cblk_m, acc_m, i;

    for(i=0; i<solvmtx->cblknbr; i++, solvcblk++)
    {
        SolverBlok *lblok = solvcblk[1].fblokptr;
        pastix_int_t m = solvcblk->stride;
        pastix_int_t n = cblk_colnbr( solvcblk );
        pastix_int_t k = blok_rownbr( solvblok );

        m -= n;

        /*
         * Compute the surface of the off-diagonal block in a panel for
         * LDL^[th] factorizations
         */
        offdarea = m * n;
        offdmax = pastix_imax( offdmax, offdarea );

        /*
         * Compute the maximum area for 1d temporary workspace in GEMM
         */
        solvblok++;
        cblk_m = -1;
        acc_m  = 0;
        for( ; solvblok<lblok; solvblok++ ) {
            k = blok_rownbr( solvblok );

            /*
             * Temporary workspace for GEMM
             * m+1 to store the diagonal in case of GEMDM
             */
            if ( !(solvcblk->cblktype & CBLK_LAYOUT_2D) ) {
                gemmarea = (m+1) * k;
                gemmmax = pastix_imax( gemmmax, gemmarea );
            }

            /*
             * Max size for off-diagonal blocks for 2-terms version of the
             * 2D LDL
             */
            if ( solvcblk->cblktype & (CBLK_TASKS_2D | CBLK_COMPRESSED) ) {
                if ( solvblok->fcblknm == cblk_m ) {
                    acc_m += k;
                }
                else {
                    cblk_m = solvblok->fcblknm;
                    acc_m = k;
                }
                /*
                 * acc_m+1 to store the diagonal in case of GEMDM
                 */
                blokmax = pastix_imax( n * (acc_m+1), blokmax );
            }
            m -= k;
        }
    }

    solvmtx->offdmax = offdmax;
    solvmtx->gemmmax = gemmmax;
    solvmtx->blokmax = blokmax;
}

/**
 *******************************************************************************
 *
 * @brief Mark blocks if they belong to the last supernode, or if they are
 * facing it for statistical purpose only.
 *
 * TODO : Should be improved by using the brow array in order to cover only the
 *        blocks in front of the last cblk
 *
 *******************************************************************************
 *
 *  @param[inout] solvmtx
 *           Pointer to the solver matrix.
 *
 *******************************************************************************/
void
solvMatGen_stats_last( SolverMatrix *solvmtx )
{
#if defined(PASTIX_SUPERNODE_STATS)
    pastix_int_t i;
    SolverBlok  *solvblok = solvmtx->bloktab;

    for(i=0; i<solvmtx->bloknbr; i++, solvblok++ ) {
        SolverCblk *fcblk = solvmtx->cblktab + solvblok->fcblknm;
        SolverCblk *lcblk = solvmtx->cblktab + solvblok->lcblknm;
        if ( fcblk->cblktype & CBLK_IN_LAST ) {
            if ( lcblk->cblktype & CBLK_IN_LAST ) {
                solvblok->inlast = 2;
            }
            else {
                solvblok->inlast = 1;
            }
        }
    }
#else
    (void)solvmtx;
#endif
}

/**
 *******************************************************************************
 *
 * @brief Register a local cblk from a symbol_cblk_t structure !(Fanin|Recv)
 *
 *******************************************************************************
 *
 * @param[in] symbmtx
 *          The pointer to the symbol matrix.
 *
 * @param[in] candcblk
 *          The cand structure associated to the current cblk to get the type of
 *          the cblk.
 *
 * @param[in] cblklocalnum
 *          Array of the local indices of the cblk.
 *
 * @param[inout] solvcblk
 *          Pointer to the current cblk to register.
 *
 * @param[inout] solvblok
 *          Pointer to the first block of the current cblk.
 *
 * @param[in] lcblknm
 *          The local index of the cblk.
 *
 * @param[in] brownum
 *          The current index in the browtab.
 *
 * @param[in] gcblknm
 *          The global index of the current cblk.
 *
 * @param[in] ownerid
 *          The index of the local MPI rank.
 *
 * @return The pointer to the next solver block to register.
 *
 *******************************************************************************/
SolverBlok*
solvMatGen_register_local_cblk( const symbol_matrix_t *symbmtx,
                                const Cand            *candcblk,
                                const pastix_int_t    *cblklocalnum,
                                SolverCblk            *solvcblk,
                                SolverBlok            *solvblok,
                                pastix_int_t           lcblknm,
                                pastix_int_t           brownum,
                                pastix_int_t           gcblknm,
                                pastix_int_t           ownerid )
{
    symbol_cblk_t *symbcblk = symbmtx->cblktab + gcblknm;
    symbol_blok_t *symbblok = symbmtx->bloktab + symbcblk->bloknum;
    SolverBlok    *fblokptr = solvblok;
    pastix_int_t   stride   = 0;
    pastix_int_t   layout2D = candcblk->cblktype & CBLK_LAYOUT_2D;
    pastix_int_t   fcolnum, lcolnum, nbcols, j;

    assert( solvblok != NULL );
    assert( brownum >= 0 );
    assert( symbblok->lcblknm == gcblknm );
    assert( (cblklocalnum == NULL) || (lcblknm == cblklocalnum[gcblknm]) );

    /*
     * Compute the number of columns of the fan-in
     */
    nbcols = symbol_cblk_get_colnum( symbmtx, symbcblk, &fcolnum, &lcolnum );

    /*
     * Register all the local blocks
     */
    for ( j=symbcblk[0].bloknum; j<symbcblk[1].bloknum; j++, symbblok++ )
    {
        pastix_int_t frownum, lrownum, nbrows;

        nbrows = symbol_blok_get_rownum( symbmtx, symbblok, &frownum, &lrownum );
        assert( nbrows >= 1 );

        /* Init the blok */
        solvMatGen_init_blok( solvblok, lcblknm,
                              cblklocalnum == NULL ? symbblok->fcblknm : cblklocalnum[symbblok->fcblknm],
                              frownum, lrownum, stride, nbcols,
                              layout2D );
        solvblok->gbloknm = j;
        stride += nbrows;
        solvblok++;
    }

    solvMatGen_init_cblk( solvcblk, fblokptr, candcblk, symbcblk,
                          fcolnum, lcolnum, brownum, stride,
                          gcblknm, ownerid );

    solvcblk->lcolidx = fcolnum;

    return solvblok;
}

/**
 *******************************************************************************
 *
 * @brief Register a remote cblk from a solver_recv_cblk_t structure (Fanin|Recv)
 *
 *******************************************************************************
 *
 * @param[inout] solvmtx
 *          The solver matrix.
 *
 * @param[in] symbmtx
 *          The pointer to the symbol matrix.
 *
 * @param[in] recvcblk
 *          The associated solver_recv_cblk_t structure used to initialize the
 *          remote cblk.
 *
 * @param[in] candcblk
 *          The cand structure associated to the current cblk to get the type of
 *          the cblk.
 *
 * @param[in] cblklocalnum
 *          Array of the local indices of the cblk.
 *
 * @param[inout] solvcblk
 *          Pointer to the current cblk to register.
 *
 * @param[inout] solvblok
 *          Pointer to the first block of the current cblk.
 *
 * @param[in] lcblknm
 *          The local index of the cblk.
 *
 * @param[in] brownum
 *          The current index in the browtab.
 *
 * @param[in] gcblknm
 *          The global index of the current cblk.
 *
 * @param[in] faninnbr_tab
 *          The fanin indexes array.
 *
 *******************************************************************************
 *
 * @return The pointer to the next solver block to register.
 *
 *******************************************************************************/
SolverBlok*
solvMatGen_register_remote_cblk( const SolverMatrix       *solvmtx,
                                 const symbol_matrix_t    *symbmtx,
                                 const solver_cblk_recv_t *recvcblk,
                                 const Cand               *candcblk,
                                 const pastix_int_t       *cblklocalnum,
                                 SolverCblk               *solvcblk,
                                 SolverBlok               *solvblok,
                                 pastix_int_t              lcblknm,
                                 pastix_int_t              brownum,
                                 pastix_int_t              gcblknm,
                                 pastix_int_t             *faninnbr_tab )
{
    const solver_blok_recv_t *recvblok = recvcblk->bloktab;
    symbol_cblk_t            *symbcblk = symbmtx->cblktab + gcblknm;
    symbol_blok_t            *symbblok = symbmtx->bloktab + symbcblk->bloknum;
    SolverBlok               *fblokptr = solvblok;
    pastix_int_t              stride   = 0;
    pastix_int_t              layout2D = candcblk->cblktype & CBLK_LAYOUT_2D;
    pastix_int_t              clustnbr = solvmtx->clustnbr;
    pastix_int_t              ownerid  = recvcblk->ownerid;
    pastix_int_t              fcolnum, lcolnum, nbcols, j;

    assert( solvblok != NULL );
    assert( brownum >= 0 );
    assert( symbblok->lcblknm == gcblknm );

    /*
     * Compute the number of columns of the fan-in
     */
    if ( symbmtx->dof < 0 ) {
        fcolnum = symbmtx->dofs[recvcblk->fcolnum];
        lcolnum = symbmtx->dofs[recvcblk->lcolnum + 1] - 1;
    }
    else {
        fcolnum = symbmtx->dof *   recvcblk->fcolnum;
        lcolnum = symbmtx->dof * ( recvcblk->lcolnum + 1 ) - 1;
    }
    nbcols = lcolnum - fcolnum + 1;

    /*
     * Register all the local blocks
     */
    for ( j=symbcblk[0].bloknum; j<symbcblk[1].bloknum; j++, recvblok++, symbblok++ )
    {
        pastix_int_t frownum, lrownum, nbrows;

        if ( symbmtx->dof < 0 ) {
            frownum = symbmtx->dofs[recvblok->frownum];
            lrownum = symbmtx->dofs[recvblok->lrownum + 1] - 1;
        }
        else {
            frownum = symbmtx->dof *   recvblok->frownum;
            lrownum = symbmtx->dof * ( recvblok->lrownum + 1 ) - 1;
        }
        nbrows = lrownum - frownum + 1;

        if ( nbrows < 1 ) {
            continue;
        }

        /* Init the blok */
        solvMatGen_init_blok( solvblok,
                              lcblknm, cblklocalnum[symbblok->fcblknm],
                              frownum, lrownum, stride, nbcols,
                              layout2D );
        solvblok->gbloknm = j;
        stride += nbrows;

        if ( solvcblk->cblktype & CBLK_FANIN ) {
            assert( faninnbr_tab );
            solvblok->gfaninnm = faninnbr_tab[2 * ownerid + 1] + solvmtx->gbloknbr;
            faninnbr_tab[2 * ownerid + 1]++;
        }
        if ( solvcblk->cblktype & CBLK_RECV ) {
            assert( faninnbr_tab );
            solvblok->gfaninnm = faninnbr_tab[2 * (clustnbr + ownerid) + 1] + solvmtx->gbloknbr;
            faninnbr_tab[2 * (clustnbr + ownerid) + 1]++;
        }
        solvblok++;
    }

    solvMatGen_init_cblk( solvcblk, fblokptr, candcblk, symbcblk,
                          fcolnum, lcolnum, brownum, stride,
                          gcblknm, ownerid );

    solvcblk->lcolidx = -1;

#if defined(PASTIX_BLEND_FANIN_FR)
    if( solvcblk->cblktype & CBLK_COMPRESSED ) {
        solvcblk->cblktype &= (~CBLK_COMPRESSED);
    }
#endif
    /* No Schur complement in distributed for the moment */
    if( solvcblk->cblktype & CBLK_IN_SCHUR ) {
        solvcblk->cblktype &= (~CBLK_IN_SCHUR);
    }

    return solvblok;
}

/**
 *@}
 */
