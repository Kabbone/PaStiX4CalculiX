/**
 * @file pastix_f2c.c
 *
 * PaStiX Fortran to C bindings module
 *
 * @copyright 2017-2025 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @author Tony Delarue
 * @date 2025-02-24
 *
 * This file has been automatically generated with gen_wrappers.py
 *
 * @ingroup wrap_fortran
 *
 */
#include "common.h"

static inline PASTIX_Comm
_pastix_comm_f2c( int pastix_comm )
{
    int flag = 0;
    MPI_Initialized(&flag);
    if ( !flag ) {
        return MPI_COMM_WORLD;
    }
    else {
        return MPI_Comm_f2c( pastix_comm );
    }
}

int
pastixOrderInit_f2c( pastix_order_t *ordeptr,
                     pastix_int_t    baseval,
                     pastix_int_t    vertnbr,
                     pastix_int_t    cblknbr,
                     pastix_int_t   *perm,
                     pastix_int_t   *invp,
                     pastix_int_t   *rang,
                     pastix_int_t   *tree )
{
    return pastixOrderInit( ordeptr, baseval, vertnbr, cblknbr, perm, invp, rang,
                            tree );
}

int
pastixOrderAlloc_f2c( pastix_order_t *ordeptr,
                      pastix_int_t    vertnbr,
                      pastix_int_t    cblknbr )
{
    return pastixOrderAlloc( ordeptr, vertnbr, cblknbr );
}

int
pastixOrderAllocId_f2c( pastix_order_t *ordeptr,
                        pastix_int_t    vertnbr )
{
    return pastixOrderAllocId( ordeptr, vertnbr );
}

void
pastixOrderExit_f2c( pastix_order_t *ordeptr )
{
    pastixOrderExit( ordeptr );
}

void
pastixOrderBase_f2c( pastix_order_t *ordeptr,
                     pastix_int_t    baseval )
{
    pastixOrderBase( ordeptr, baseval );
}

int
pastixOrderCheck_f2c( const pastix_order_t *ordeptr )
{
    return pastixOrderCheck( ordeptr );
}

int
pastixOrderCopy_f2c( pastix_order_t       *ordedst,
                     const pastix_order_t *ordesrc )
{
    return pastixOrderCopy( ordedst, ordesrc );
}

pastix_order_t *
pastixOrderGet_f2c( const pastix_data_t *pastix_data )
{
    return pastixOrderGet( pastix_data );
}

void
pastixOrderBcast_f2c( pastix_order_t *ordemesh,
                      int             root,
                      int             pastix_comm )
{
    pastixOrderBcast( ordemesh, root, _pastix_comm_f2c( pastix_comm ) );
}

int
pastixOrderGrid_f2c( pastix_order_t **myorder,
                     pastix_int_t     nx,
                     pastix_int_t     ny,
                     pastix_int_t     nz )
{
    return pastixOrderGrid( myorder, nx, ny, nz );
}

int
pastixOrderLoad_f2c( const pastix_data_t *pastix_data,
                     pastix_order_t      *ordeptr )
{
    return pastixOrderLoad( pastix_data, ordeptr );
}

int
pastixOrderSave_f2c( pastix_data_t        *pastix_data,
                     const pastix_order_t *ordeptr )
{
    return pastixOrderSave( pastix_data, ordeptr );
}

int
pastix_f2c( pastix_data_t **pastix_data,
            int             pastix_comm,
            pastix_int_t    n,
            pastix_int_t   *colptr,
            pastix_int_t   *rowptr,
            void           *values,
            pastix_int_t   *perm,
            pastix_int_t   *invp,
            void           *B,
            pastix_int_t    nrhs,
            pastix_int_t   *iparm,
            double         *dparm )
{
    return pastix( pastix_data, _pastix_comm_f2c( pastix_comm ), n, colptr,
                   rowptr, values, perm, invp, B, nrhs, iparm, dparm );
}

void
pastixInitParam_f2c( pastix_int_t *iparm,
                     double       *dparm )
{
    pastixInitParam( iparm, dparm );
}

void
pastixInit_f2c( pastix_data_t **pastix_data,
                int             pastix_comm,
                pastix_int_t   *iparm,
                double         *dparm )
{
    pastixInit( pastix_data, _pastix_comm_f2c( pastix_comm ), iparm, dparm );
}

void
pastixInitWithAffinity_f2c( pastix_data_t **pastix_data,
                            int             pastix_comm,
                            pastix_int_t   *iparm,
                            double         *dparm,
                            const int      *bindtab )
{
    pastixInitWithAffinity( pastix_data, _pastix_comm_f2c( pastix_comm ), iparm,
                            dparm, bindtab );
}

void
pastixFinalize_f2c( pastix_data_t **pastix_data )
{
    pastixFinalize( pastix_data );
}

int
pastix_task_analyze_f2c( pastix_data_t    *pastix_data,
                         const spmatrix_t *spm )
{
    return pastix_task_analyze( pastix_data, spm );
}

int
pastix_task_numfact_f2c( pastix_data_t *pastix_data,
                         spmatrix_t    *spm )
{
    return pastix_task_numfact( pastix_data, spm );
}

int
pastix_task_solve_f2c( pastix_data_t *pastix_data,
                       pastix_int_t   m,
                       pastix_int_t   nrhs,
                       void          *B,
                       pastix_int_t   ldb )
{
    return pastix_task_solve( pastix_data, m, nrhs, B, ldb );
}

int
pastix_task_refine_f2c( pastix_data_t *pastix_data,
                        pastix_int_t   n,
                        pastix_int_t   nrhs,
                        void          *B,
                        pastix_int_t   ldb,
                        void          *X,
                        pastix_int_t   ldx )
{
    return pastix_task_refine( pastix_data, n, nrhs, B, ldb, X, ldx );
}

int
pastix_task_solve_and_refine_f2c( pastix_data_t *pastix_data,
                                  pastix_int_t   n,
                                  pastix_int_t   nrhs,
                                  void          *B,
                                  pastix_int_t   ldb,
                                  void          *X,
                                  pastix_int_t   ldx )
{
    return pastix_task_solve_and_refine( pastix_data, n, nrhs, B, ldb, X, ldx );
}

int
pastix_subtask_order_f2c( pastix_data_t    *pastix_data,
                          const spmatrix_t *spm,
                          pastix_order_t   *myorder )
{
    return pastix_subtask_order( pastix_data, spm, myorder );
}

int
pastix_subtask_symbfact_f2c( pastix_data_t *pastix_data )
{
    return pastix_subtask_symbfact( pastix_data );
}

int
pastix_subtask_reordering_f2c( pastix_data_t *pastix_data )
{
    return pastix_subtask_reordering( pastix_data );
}

int
pastix_subtask_blend_f2c( pastix_data_t *pastix_data )
{
    return pastix_subtask_blend( pastix_data );
}

int
pastix_subtask_spm2bcsc_f2c( pastix_data_t *pastix_data,
                             spmatrix_t    *spm )
{
    return pastix_subtask_spm2bcsc( pastix_data, spm );
}

int
pastix_subtask_bcsc2ctab_f2c( pastix_data_t *pastix_data )
{
    return pastix_subtask_bcsc2ctab( pastix_data );
}

int
pastix_subtask_sopalin_f2c( pastix_data_t *pastix_data )
{
    return pastix_subtask_sopalin( pastix_data );
}

int
pastix_subtask_applyorder_f2c( pastix_data_t *pastix_data,
                               pastix_dir_t   dir,
                               pastix_int_t   m,
                               pastix_int_t   n,
                               void          *B,
                               pastix_int_t   ldb,
                               pastix_rhs_t   Bp )
{
    return pastix_subtask_applyorder( pastix_data, dir, m, n, B, ldb, Bp );
}

int
pastix_subtask_trsm_f2c( pastix_data_t *pastix_data,
                         pastix_side_t  side,
                         pastix_uplo_t  uplo,
                         pastix_trans_t trans,
                         pastix_diag_t  diag,
                         pastix_rhs_t   b )
{
    return pastix_subtask_trsm( pastix_data, side, uplo, trans, diag, b );
}

int
pastix_subtask_diag_f2c( pastix_data_t *pastix_data,
                         pastix_rhs_t   b )
{
    return pastix_subtask_diag( pastix_data, b );
}

int
pastix_subtask_solve_f2c( pastix_data_t *pastix_data,
                          pastix_rhs_t   b )
{
    return pastix_subtask_solve( pastix_data, b );
}

int
pastix_subtask_refine_f2c( pastix_data_t *pastix_data,
                           pastix_rhs_t   b,
                           pastix_rhs_t   x )
{
    return pastix_subtask_refine( pastix_data, b, x );
}

int
pastix_subtask_solve_adv_f2c( pastix_data_t *pastix_data,
                              pastix_trans_t transA,
                              pastix_rhs_t   b )
{
    return pastix_subtask_solve_adv( pastix_data, transA, b );
}

void
pastixIsolateUnknowns_f2c( pastix_data_t      *pastix_data,
                           pastix_int_t        n,
                           const pastix_int_t *list )
{
    pastixIsolateUnknowns( pastix_data, n, list );
}

void
pastixSetSchurUnknownList_f2c( pastix_data_t      *pastix_data,
                               pastix_int_t        n,
                               const pastix_int_t *list )
{
    pastixSetSchurUnknownList( pastix_data, n, list );
}

int
pastixGetSchur_f2c( const pastix_data_t *pastix_data,
                    void                *S,
                    pastix_int_t         lds )
{
    return pastixGetSchur( pastix_data, S, lds );
}

int
pastixRhsInit_f2c( pastix_rhs_t *rhs )
{
    return pastixRhsInit( rhs );
}

int
pastixRhsFinalize_f2c( pastix_rhs_t rhs )
{
    return pastixRhsFinalize( rhs );
}

int
pastixRhsDoubletoSingle_f2c( const pastix_rhs_t dB,
                             pastix_rhs_t       sB )
{
    return pastixRhsDoubletoSingle( dB, sB );
}

int
pastixRhsSingleToDouble_f2c( const pastix_rhs_t sB,
                             pastix_rhs_t       dB )
{
    return pastixRhsSingleToDouble( sB, dB );
}

int
pastixRhsSchurGet_f2c( const pastix_data_t *pastix_data,
                       pastix_int_t         m,
                       pastix_int_t         n,
                       pastix_rhs_t         rhsB,
                       void                *B,
                       pastix_int_t         ldb )
{
    return pastixRhsSchurGet( pastix_data, m, n, rhsB, B, ldb );
}

int
pastixRhsSchurSet_f2c( const pastix_data_t *pastix_data,
                       pastix_int_t         m,
                       pastix_int_t         n,
                       void                *B,
                       pastix_int_t         ldb,
                       pastix_rhs_t         rhsB )
{
    return pastixRhsSchurSet( pastix_data, m, n, B, ldb, rhsB );
}

void
pastixExpand_f2c( const pastix_data_t *pastix_data,
                  spmatrix_t          *spm )
{
    pastixExpand( pastix_data, spm );
}

int
pastixGetDiag_f2c( const pastix_data_t *pastix_data,
                   void                *x,
                   pastix_int_t         incx )
{
    return pastixGetDiag( pastix_data, x, incx );
}

void
pastixGetOptions_f2c( int           argc,
                      char        **argv,
                      pastix_int_t *iparm,
                      double       *dparm,
                      int          *check,
                      int          *scatter,
                      spm_driver_t *driver,
                      char        **filename )
{
    pastixGetOptions( argc, argv, iparm, dparm, check, scatter, driver, filename );
}

void
pastixDumpParam_f2c( const pastix_data_t *pastix_data )
{
    pastixDumpParam( pastix_data );
}

int
pastixCheckParam_f2c( const pastix_int_t *iparm,
                      const double       *dparm )
{
    return pastixCheckParam( iparm, dparm );
}

int
pastixBlasGetNumThreads_f2c( void )
{
    return pastixBlasGetNumThreads( );
}

int
pastixBlasSetNumThreads_f2c( int nt )
{
    return pastixBlasSetNumThreads( nt );
}

int
pastixBlasSetNumThreadsOne_f2c( void )
{
    return pastixBlasSetNumThreadsOne( );
}
