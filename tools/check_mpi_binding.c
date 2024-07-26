/**
 *
 *  @file check_mpi_binding.c
 *  @copyright 2013-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                       Univ. Bordeaux. All rights reserved.
 *
 *  @version 6.4.0
 *  @author Pierre Ramet
 *  @author Mathieu Faverge
 *  @date 2024-07-05
 *
 * @brief Small test to check that the processes are correctly bound.
 * This test can be compiled with:
 *     mpicc check_mpi_binding.c -o check_mpi_binding -Wall -lpthread -lhwloc
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <string.h>
#include <hwloc.h>

/**
 * Extract of PaStiX internal scheduler for simpler compilation of this small test
 * DO NOT use these function in your code, but call directly the ones from PaStiX.
 */
static hwloc_topology_t topology;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
int isched_topo_init(void)
{
    /* Load the global config */
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);

    return 0;
}

int isched_topo_restrict(void)
{
    /* Restrict to the MPI binding */
    hwloc_bitmap_t cpuset = hwloc_bitmap_alloc();
    int rc;
    rc = hwloc_get_cpubind( topology, cpuset, HWLOC_CPUBIND_PROCESS );
    if ( rc == 0 ) {
#if HWLOC_API_VERSION >= 0x20000
        rc = hwloc_topology_restrict( topology, cpuset, HWLOC_RESTRICT_FLAG_REMOVE_CPULESS );
#else
        rc = hwloc_topology_restrict( topology, cpuset, 0 );
#endif
        if ( rc != 0 ) {
            fprintf(stderr,
                    "Failed to restrict the topology to the correct cpuset\n"
                    "This may generate incorrect bindings\n");
        }
    }
    hwloc_bitmap_free(cpuset);
    return 0;
}

int isched_topo_world_size()
{
    hwloc_obj_t obj = hwloc_get_obj_by_type( topology, HWLOC_OBJ_MACHINE, 0 );
    return hwloc_get_nbobjs_inside_cpuset_by_type(topology, obj->cpuset, HWLOC_OBJ_CORE);
}

int isched_topo_fini(void)
{
    /* Destroy topology object. */
    hwloc_topology_destroy(topology);
    return 0;
}

/**
 * End of the extract
 */

#define BUF_MAX 256

int main( int argc, char *argv[] )
{
    int world_size, world_rank;
    int nbcores_per_node, nbcores_per_process;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &world_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

    /**
     * Initialize hwloc topology
     */
    isched_topo_init();
    nbcores_per_node = isched_topo_world_size();

    /**
     * Restrict the topology to the MPI process binding as done within PaStiX
     */
    isched_topo_restrict();
    nbcores_per_process = isched_topo_world_size();
    isched_topo_fini();

    /**
     * Print the global information
     */
    if ( world_rank == 0 ) {
        printf( " Number of MPI processes:                %d\n"
                " Number of physical cores per node:      %d\n"
                " Number of attributed cores per process: %d\n",
                world_size, nbcores_per_node, nbcores_per_process );
    }

    /**
     * Print the bounded cpuset (be careful returns the PU and not the cores.
     */
    {
        int namelen;
        FILE *fp;
        char path[PATH_MAX];
        char processor_name[MPI_MAX_PROCESSOR_NAME];    /* Print binding */
        MPI_Get_processor_name(processor_name, &namelen);

        fp = popen("grep Cpus_allowed_list /proc/$$/status", "r");
        while (fgets(path, PATH_MAX, fp) != NULL) {
            printf("[%2d] Runs on %s: %s", world_rank, processor_name, path);
        }
        pclose(fp);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
