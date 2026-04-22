/**
 *
 * @file pastix.h
 *
 * PaStiX main header file.
 *
 * @copyright 2004-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author David Goudin
 * @author Francois Pellegrini
 * @author Gregoire Pichon
 * @author Mathieu Faverge
 * @author Pascal Henon
 * @author Pierre Ramet
 * @author Xavier Lacoste
 * @author Theophile Terraz
 * @author Tony Delarue
 * @date 2024-07-05
 *
 **/
#ifndef _pastix_h_
#define _pastix_h_

#include "pastix/config.h"
#include <spm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if defined(PASTIX_WITH_MPI)
#include <mpi.h>
typedef MPI_Comm PASTIX_Comm;
#else
typedef uintptr_t PASTIX_Comm;
#ifndef MPI_COMM_WORLD
#define MPI_COMM_WORLD 0
#endif
#endif
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "pastix/api.h"
#include "pastix/datatypes.h"
#include "pastix/order.h"

BEGIN_C_DECLS

/*
 * Main function for compatibility with former versions
 */
int pastix( pastix_data_t **pastix_data,
            PASTIX_Comm     pastix_comm,
            pastix_int_t    n,
            pastix_int_t   *colptr,
            pastix_int_t   *rowptr,
            void           *values,
            pastix_int_t   *perm,
            pastix_int_t   *invp,
            void           *B,
            pastix_int_t    nrhs,
            pastix_int_t   *iparm,
            double         *dparm );

/*
 * Solver initialization
 */
void pastixInitParam( pastix_int_t   *iparm,
                      double         *dparm );
void pastixInit     ( pastix_data_t **pastix_data,
                      PASTIX_Comm     pastix_comm,
                      pastix_int_t   *iparm,
                      double         *dparm );
void pastixInitWithAffinity( pastix_data_t **pastix_data,
                             PASTIX_Comm     pastix_comm,
                             pastix_int_t   *iparm,
                             double         *dparm,
                             const int      *bindtab );
void pastixFinalize ( pastix_data_t **pastix_data );

/*
 * Main steps of the solver
 */
int pastix_task_analyze( pastix_data_t    *pastix_data,
                         const spmatrix_t *spm );
int pastix_task_numfact( pastix_data_t    *pastix_data,
                         spmatrix_t       *spm );
int pastix_task_solve  ( pastix_data_t    *pastix_data,
                         pastix_int_t      m,
                         pastix_int_t      nrhs,
                         void             *B,
                         pastix_int_t      ldb );
int pastix_task_refine ( pastix_data_t    *pastix_data,
                         pastix_int_t      n,
                         pastix_int_t      nrhs,
                         void             *B,
                         pastix_int_t      ldb,
                         void             *X,
                         pastix_int_t      ldx );
int pastix_task_solve_and_refine ( pastix_data_t *pastix_data,
                                   pastix_int_t   n,
                                   pastix_int_t   nrhs,
                                   void          *B,
                                   pastix_int_t   ldb,
                                   void          *X,
                                   pastix_int_t   ldx );

/*
 * Analyze subtasks
 */
int pastix_subtask_order     ( pastix_data_t    *pastix_data,
                               const spmatrix_t *spm,
                               pastix_order_t   *myorder );
int pastix_subtask_symbfact  ( pastix_data_t    *pastix_data );
int pastix_subtask_reordering( pastix_data_t    *pastix_data );
int pastix_subtask_blend     ( pastix_data_t    *pastix_data );

/*
 * Numerical factorization subtasks
 */
int pastix_subtask_spm2bcsc        ( pastix_data_t *pastix_data,
                                     spmatrix_t    *spm );
int pastix_subtask_spm2bcsc_values ( pastix_data_t *pastix_data,
                                     spmatrix_t    *spm );
int pastix_subtask_bcsc2ctab ( pastix_data_t *pastix_data );
int pastix_subtask_sopalin   ( pastix_data_t *pastix_data );

/*
 * Numerical solve and refinement subtasks
 */
int pastix_subtask_applyorder( pastix_data_t *pastix_data,
                               pastix_dir_t   dir,
                               pastix_int_t   m,
                               pastix_int_t   n,
                               void          *B,
                               pastix_int_t   ldb,
                               pastix_rhs_t   Bp );
int pastix_subtask_trsm( pastix_data_t    *pastix_data,
                         pastix_side_t     side,
                         pastix_uplo_t     uplo,
                         pastix_trans_t    trans,
                         pastix_diag_t     diag,
                         pastix_rhs_t      b );
int pastix_subtask_diag( pastix_data_t    *pastix_data,
                         pastix_rhs_t      b );
int pastix_subtask_solve( pastix_data_t *pastix_data,
                          pastix_rhs_t   b );
int pastix_subtask_refine( pastix_data_t *pastix_data,
                           pastix_rhs_t   b,
                           pastix_rhs_t   x );
int pastix_subtask_solve_adv( pastix_data_t *pastix_data,
                              pastix_trans_t transA,
                              pastix_rhs_t   b );

/*
 * Separate set of unknowns to avoid numerical errors
 */
void pastixIsolateUnknowns( pastix_data_t      *pastix_data,
                            pastix_int_t        n,
                            const pastix_int_t *list );

/*
 * Schur complement manipulation routines.
 */
void pastixSetSchurUnknownList( pastix_data_t       *pastix_data,
                                pastix_int_t         n,
                                const pastix_int_t  *list );
int  pastixGetSchur           ( const pastix_data_t *pastix_data,
                                void                *S,
                                pastix_int_t         lds );

/*
 * Right hand side routines.
 */
int pastixRhsInit( pastix_rhs_t *rhs );
int pastixRhsFinalize( pastix_rhs_t rhs );
int pastixRhsDoubletoSingle( const pastix_rhs_t dB,
                             pastix_rhs_t       sB );
int pastixRhsSingleToDouble( const pastix_rhs_t sB,
                             pastix_rhs_t       dB );

int pastixRhsSchurGet( const pastix_data_t *pastix_data,
                       pastix_int_t         m,
                       pastix_int_t         n,
                       pastix_rhs_t         rhsB,
                       void                *B,
                       pastix_int_t         ldb );
int pastixRhsSchurSet( const pastix_data_t *pastix_data,
                       pastix_int_t         m,
                       pastix_int_t         n,
                       void                *B,
                       pastix_int_t         ldb,
                       pastix_rhs_t         rhsB );

/*
 * DoF subroutine to expand the problem.
 */
void pastixExpand( const pastix_data_t *pastix_data,
                   spmatrix_t          *spm );

/*
 * Function to provide access to the diagonal
 */
int pastixGetDiag( const pastix_data_t *pastix_data,
                   void                *x,
                   pastix_int_t         incx );

/*
 * Function to provide a common way to read binary options in examples/testings
 */
void pastixGetOptions( int            argc,
                       char         **argv,
                       pastix_int_t  *iparm,
                       double        *dparm,
                       int           *check,
                       int           *scatter,
                       spm_driver_t  *driver,
                       char         **filename );

/*
 * Function to provide a common way to output
 * the iparm/dparm parameters in a CSV file.
 */
void pastixDumpParam ( const pastix_data_t *pastix_data );
int  pastixCheckParam( const pastix_int_t *iparm,
                       const double       *dparm );

/*
 * Functions to handle number of Blas threads.
 */
int pastixBlasGetNumThreads(void);
int pastixBlasSetNumThreads( int nt );
int pastixBlasSetNumThreadsOne(void);

END_C_DECLS

#endif /* _pastix_h_ */
