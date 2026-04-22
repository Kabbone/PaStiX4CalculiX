/**
 * @file bcsc_z.h
 *
 * @copyright 2004-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @author Pierre Ramet
 * @author Xavier Lacoste
 * @author Vincent Bridonneau
 * @date 2024-07-05
 *
 * @precisions normal z -> c d s
 *
 **/
#ifndef _bcsc_z_h_
#define _bcsc_z_h_

/**
 * @addtogroup bcsc_internal
 * @{
 *
 *    @name PastixComplex64 initialization functions
 *    @{
 */
void bcsc_zinit( const spmatrix_t     *spm,
                 const pastix_order_t *ord,
                 const SolverMatrix   *solvmtx,
                 int                   initAt,
                 pastix_bcsc_t        *bcsc,
                 pastix_int_t          valuesize );

void bcsc_zupdatevalues( const spmatrix_t     *spm,
                         const pastix_order_t *ord,
                         const SolverMatrix   *solvmtx,
                         int                   initAt,
                         pastix_bcsc_t        *bcsc,
                         pastix_int_t          valuesize );

#if defined(PASTIX_WITH_MPI)
void bcsc_zstore_data( const spmatrix_t     *spm,
                       const pastix_order_t *ord,
                       const pastix_int_t   *col2cblk,
                       bcsc_handle_comm_t   *bcsc_comm );
#endif

/**
 *   @}
 * @}
 *
 * @addtogroup bcsc
 * @{
 *
 *    @name PastixComplex64 vector(s) operations
 *    @{
 */
void bvec_zaxpy_seq( pastix_data_t            *pastix_data,
                     pastix_int_t              n,
                     pastix_complex64_t        alpha,
                     const pastix_complex64_t *x,
                     pastix_complex64_t       *y );
void bvec_zaxpy_smp( pastix_data_t            *pastix_data,
                     pastix_int_t              n,
                     pastix_complex64_t        alpha,
                     const pastix_complex64_t *x,
                     pastix_complex64_t       *y );

void bvec_zcopy_seq( pastix_data_t            *pastix_data,
                     pastix_int_t              n,
                     const pastix_complex64_t *x,
                     pastix_complex64_t       *y );
void bvec_zcopy_smp( pastix_data_t            *pastix_data,
                     pastix_int_t              n,
                     const pastix_complex64_t *x,
                     pastix_complex64_t       *y );

#if defined(PRECISION_z) || defined(PRECISION_c)
pastix_complex64_t bvec_zdotc_seq( pastix_data_t            *pastix_data,
                                   pastix_int_t              n,
                                   const pastix_complex64_t *x,
                                   const pastix_complex64_t *y );
pastix_complex64_t bvec_zdotc_smp( pastix_data_t            *pastix_data,
                                   pastix_int_t              n,
                                   const pastix_complex64_t *x,
                                   const pastix_complex64_t *y );
#endif

pastix_complex64_t bvec_zdotu_seq( pastix_data_t            *pastix_data,
                                   pastix_int_t              n,
                                   const pastix_complex64_t *x,
                                   const pastix_complex64_t *y );
pastix_complex64_t bvec_zdotu_smp( pastix_data_t            *pastix_data,
                                   pastix_int_t              n,
                                   const pastix_complex64_t *x,
                                   const pastix_complex64_t *y );

void bvec_zgemv_seq( pastix_data_t            *pastix_data,
                     pastix_int_t              m,
                     pastix_int_t              n,
                     pastix_complex64_t        alpha,
                     const pastix_complex64_t *A,
                     pastix_int_t              lda,
                     const pastix_complex64_t *x,
                     pastix_complex64_t        beta,
                     pastix_complex64_t       *y );
void bvec_zgemv_smp( pastix_data_t            *pastix_data,
                     pastix_int_t              m,
                     pastix_int_t              n,
                     pastix_complex64_t        alpha,
                     const pastix_complex64_t *A,
                     pastix_int_t              lda,
                     const pastix_complex64_t *x,
                     pastix_complex64_t        beta,
                     pastix_complex64_t       *y );

double bvec_znrm2_seq( pastix_data_t            *pastix_data,
                       pastix_int_t              n,
                       const pastix_complex64_t *x );
double bvec_znrm2_smp( pastix_data_t            *pastix_data,
                       pastix_int_t              n,
                       const pastix_complex64_t *x );

void bvec_zscal_seq( pastix_data_t      *pastix_data,
                     pastix_int_t        n,
                     pastix_complex64_t  alpha,
                     pastix_complex64_t *x );
void bvec_zscal_smp( pastix_data_t      *pastix_data,
                     pastix_int_t        n,
                     pastix_complex64_t  alpha,
                     pastix_complex64_t *x );

#if defined( PASTIX_WITH_MPI )
int bvec_zexchange_data_rep( pastix_data_t      *pastix_data,
                             pastix_int_t        nrhs,
                             pastix_complex64_t *b,
                             pastix_int_t        ldb,
                             pastix_rhs_t        Pb );
int bvec_zallocate_buf_dst( bvec_handle_comm_t *rhs_comm );
int bvec_zexchange_data_dst( pastix_data_t      *pastix_data,
                             pastix_dir_t        dir,
                             pastix_int_t        nrhs,
                             pastix_complex64_t *b,
                             pastix_int_t        ldb,
                             pastix_rhs_t        Pb,
                             const pastix_int_t *glob2loc );
#endif

int bvec_zlapmr( pastix_data_t      *pastix_data,
                 pastix_dir_t        dir,
                 pastix_int_t        m,
                 pastix_int_t        n,
                 pastix_complex64_t *A,
                 pastix_int_t        lda,
                 pastix_rhs_t        PA );

/**
 *    @}
 *
 *    @name PastixComplex64 matrix operations
 *    @{
 */
double bcsc_znorm( pastix_normtype_t    ntype,
                   const pastix_bcsc_t *bcsc );

void bcsc_zspsv( pastix_data_t      *pastix_data,
                 pastix_complex64_t *b,
                 pastix_complex32_t *work );

void bcsc_zspmv( const pastix_data_t      *pastix_data,
                 pastix_trans_t            trans,
                 pastix_complex64_t        alpha,
                 const pastix_complex64_t *x,
                 pastix_complex64_t        beta,
                 pastix_complex64_t       *y );

void bcsc_zspmv_seq( const pastix_data_t      *pastix_data,
                     pastix_trans_t            trans,
                     pastix_complex64_t        alpha,
                     const pastix_complex64_t *x,
                     pastix_complex64_t        beta,
                     pastix_complex64_t       *y );
void bcsc_zspmv_smp( const pastix_data_t      *pastix_data,
                     pastix_trans_t            trans,
                     pastix_complex64_t        alpha,
                     const pastix_complex64_t *x,
                     pastix_complex64_t        beta,
                     pastix_complex64_t       *y );

/**
 *    @}
 *
 *    @name PastixComplex64 MPI vector operations
 *    @{
 */
const pastix_complex64_t *bvec_zgather_remote( const pastix_data_t      *pastix_data,
                                               const pastix_complex64_t *y );
void bvec_znullify_remote( const pastix_data_t *pastix_data,
                           pastix_complex64_t  *y );
void bvec_zallreduce( const pastix_data_t *pastix_data,
                      pastix_complex64_t  *y );
/**
 *    @}
 * @}
 */
#endif /* _bcsc_z_h_ */
