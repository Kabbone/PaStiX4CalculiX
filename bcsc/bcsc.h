/**
 *
 * @file bcsc.h
 *
 * @copyright 2004-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 6.4.0
 * @author Mathieu Faverge
 * @author Pierre Ramet
 * @author Xavier Lacoste
 * @author Alycia Lisito
 * @date 2024-07-05
 *
 * @addtogroup bcsc
 * @{
 *   @brief Describe all the internals routines to manipulate the internal block csc.
 *
 *   These functions provide a set of subroutines to manipulate the permuted
 *   sparse matrix stored in block of columns following the partition.
 *
 **/
#ifndef _bcsc_h_
#define _bcsc_h_

/**
 * @brief Tags used in MPI communications.
 */
typedef enum bcsc_tag_ {
    PastixTagCountA,
    PastixTagCountAt,
    PastixTagCountAAt,
    PastixTagIndexesA,
    PastixTagIndexesAt,
    PastixTagIndexesAAt,
    PastixTagValuesA,
    PastixTagValuesAt,
    PastixTagValuesAAt,
    PastixTagMemSendIdx,
    PastixTagMemRecvIdx,
    PastixTagMemSend,
    PastixTagMemRecvAAt,
    PastixTagMemSendValA,
    PastixTagMemSendValAt,
    PastixTagMemSendValAAt,
    PastixTagMemRecvIdxA,
    PastixTagMemRecvIdxAt,
    PastixTagMemRecvIdxAAt,
    PastixTagMemRecvValAAt,
} bcsc_tag_e;

/**
 * @brief Information about the amount of data.
 */
typedef struct bcsc_data_amount_s
{
    pastix_int_t idxcnt; /**< Amount of indexes of A or At which will be exchanged. */
    pastix_int_t valcnt; /**< Amount of values of A or At which will be exchanged.  */
} bcsc_data_amount_t;

/**
 * @brief Information about the sending data.
 */
typedef struct bcsc_exch_comm_s
{
    bcsc_data_amount_t  size;   /**< Amount of indexes of A which will be send to clustnum.             */
    pastix_int_t       *idxbuf; /**< Array of indexes of A to send to clustnum.                         */
                                /*   indexes_A[2*k]   = kth column index to send to proc clustnum.      */
                                /*   indexes_A[2*k+1] = kth row index to send to proc clustnum.         */
    void               *valbuf; /**< Array of values of A to send to clustnum.                          */
                                /*   values_A is sorted the same way as the indexes_A, for each indexes */
                                /*   there are dofi*dofj values.                                        */
} bcsc_exch_comm_t;

/**
 * @brief Informations of the data exchanged with other processors.
 */
typedef struct bcsc_proc_comm_s
{
    bcsc_exch_comm_t   sendA;   /**< Sending data of A.         */
    bcsc_exch_comm_t   sendAt;  /**< Sending data of At.        */
    bcsc_exch_comm_t   sendAAt; /**< Sending data of A and At.  */
    bcsc_data_amount_t recvA;   /**< Receving data of A.        */
    bcsc_data_amount_t recvAt;  /**< Receving data of At.       */
    bcsc_exch_comm_t   recvAAt; /**< Receving data of A and At. */
} bcsc_proc_comm_t;

/**
 * @brief Structure to manage communications with distributed spm.
 */
typedef struct bcsc_handle_comm_s
{
    pastix_int_t      clustnbr;     /**< Number of processes in the cluster.                         */
    pastix_int_t      clustnum;     /**< ID of the current process in the cluster.                   */
    PASTIX_Comm       comm;         /**< PaStiX MPI communicator used for the ordering step.         */
    pastix_coeftype_t flttype;      /**< valtab datatype: PastixFloat, PastixDouble, PastixComplex32 or PastixComplex64 */
    pastix_int_t      max_idx;      /**< Maximum amount of indexes received, used to allocate the receiving buffer.     */
    pastix_int_t      max_val;      /**< Maximum amount of values received, used to allocate the receiving buffer.      */
    bcsc_proc_comm_t  data_comm[1]; /**< Array of size clustnbr.                                     */
                                    /* data_comm[c]: contains the data clustnum has to send to c     */
                                    /*               and the amount of data clustnum will receive    */
                                    /*               from c.                                         */
                                    /* data_comm[clustnum]: contains the data clustnum will recevied */
                                    /*                     from the other processors.                */
} bcsc_handle_comm_t;

/**
 * @brief Compressed colptr format for the bcsc
 */
typedef struct bcsc_cblk_s {
    pastix_int_t  colnbr;  /**< Number of columns in the block column.                                    */
    pastix_int_t  cblknum; /**< Index of the corresponding cblk in the local solver matrix                */
    pastix_int_t *coltab;  /**< Array of indexes of the start of each column in the row and value arrays. */
} bcsc_cblk_t;

/**
 * @brief Internal column block distributed CSC matrix.
 */
struct pastix_bcsc_s {
    pastix_int_t        gN;        /**< Global number of vertices                                                          */
    pastix_int_t        n;         /**< Local number of vertices                                                           */
    pastix_mtxtype_t    mtxtype;   /**< Matrix structure: PastixGeneral, PastixSymmetric or PastixHermitian.               */
    pastix_coeftype_t   flttype;   /**< valtab datatype: PastixFloat, PastixDouble, PastixComplex32 or PastixComplex64     */
    pastix_int_t        cscfnbr;   /**< Number of column blocks.                                                           */
    bcsc_cblk_t        *cscftab;   /**< Array of Block column structures of size cscfnbr. (pastix_bcscFormat_t)            */
    pastix_int_t       *rowtab;    /**< Array of rows in the matrix.                                                       */
    pastix_int_t       *Urowtab;   /**< Array of rows for Uvalues (general LU only, NULL otherwise).                       */
    void               *Lvalues;   /**< Array of values of the matrix A                                                    */
    void               *Uvalues;   /**< Array of values of the matrix A^t                                                  */
    pastix_int_t       *col2cblk;  /**< Array which gives the repartition of the solvmtx columns into the block structure. */
                                   /*   If the column k belongs to a remote block then col2cblk[k] = -(owner_proc + 1).    */
                                   /*   If the column k belongs to a local block then col2cblk[k] = block_num.             */
    bcsc_handle_comm_t *bcsc_comm; /**< Structure which handles the MPI communication (= NULL if PASTIX_WITH_MPI=OFF).     */
    /**
     * Scatter-index maps for O(nnz) value-only updates (bcscUpdateValues).
     * spm2bcsc_L[k]  = position in Lvalues for the k-th scalar of spm->values (direct fill).
     * spm2bcsc_Lt[k] = position in Lvalues for the transposed entry (sym/herm only; -1 for diagonal).
     * spm2bcsc_U[k]  = position in Uvalues for the transposed entry (general LU only).
     * Remote MPI entries carry the sentinel -1 and are handled by the normal exchange path.
     */
    pastix_int_t       *spm2bcsc_L;  /**< spm->values[k] → Lvalues position (direct contribution).    */
    pastix_int_t       *spm2bcsc_Lt; /**< spm->values[k] → Lvalues position (transposed, sym/herm).   */
    pastix_int_t       *spm2bcsc_U;  /**< spm->values[k] → Uvalues position (transposed, general LU). */
};

double bcscInit( const spmatrix_t     *spm,
                 const pastix_order_t *ord,
                 const SolverMatrix   *solvmtx,
                 pastix_int_t          initAt,
                 pastix_bcsc_t        *bcsc );

double bcscUpdateValues( const spmatrix_t     *spm,
                         const pastix_order_t *ord,
                         const SolverMatrix   *solvmtx,
                         pastix_int_t          initAt,
                         pastix_bcsc_t        *bcsc );

void bcsc_build_spm2bcsc( const spmatrix_t     *spm,
                           const pastix_order_t *ord,
                           const SolverMatrix   *solvmtx,
                           pastix_bcsc_t        *bcsc );

void   bcscExit( pastix_bcsc_t *bcsc );

/**
 * @}
 *
 * @addtogroup bcsc_internal
 * @{
 */
void bcsc_init_struct( const spmatrix_t   *spm,
                       const SolverMatrix *solvmtx,
                       pastix_bcsc_t      *bcsc );
void bcsc_exit_struct( pastix_bcsc_t *bcsc );
pastix_int_t bcsc_init_coltab( const spmatrix_t     *spm,
                               const pastix_order_t *ord,
                               const SolverMatrix   *solvmtx,
                               pastix_bcsc_t        *bcsc );
void bcsc_restore_coltab( pastix_bcsc_t *bcsc );
pastix_int_t * bcsc_init_col2cblk_shm( const SolverMatrix  *solvmtx,
                                       const pastix_bcsc_t *bcsc );
pastix_int_t * bcsc_init_col2cblk( const SolverMatrix  *solvmtx,
                                   const pastix_bcsc_t *bcsc,
                                   const spmatrix_t    *spm );

void bcsc_handle_comm_init( const SolverMatrix *solvmtx,
                            pastix_bcsc_t      *bcsc );
void bcsc_handle_comm_exit( bcsc_handle_comm_t *bcsc_comm );

#if defined(PASTIX_WITH_MPI)
int bcsc_allocate_buf( bcsc_handle_comm_t *bcsc_comm,
                       bcsc_tag_e          mode  );
int bcsc_free_buf( bcsc_handle_comm_t *bcsc_comm,
                   bcsc_tag_e          mode );
void bcsc_exchange_amount_of_data( bcsc_handle_comm_t *bcsc_comm );
void bcsc_exchange_indexes( bcsc_handle_comm_t   *bcsc_comm );
pastix_int_t * bcsc_init_col2cblk_dst( const SolverMatrix  *solvmtx,
                                       const pastix_bcsc_t *bcsc );
#endif

/**
 * @}
 */
#endif /* _bcsc_h_ */
