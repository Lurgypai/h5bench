#include <hdf5.h>
#include <mpi.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

int MY_RANK, NUM_RANKS;

void
print_usage(char *name)
{
    if (MY_RANK == 0) {
        printf("=============== Usage: %s /path_to_config_file /path_to_output_data_file [CSV "
               "csv_file_path]=============== \n",
               name);
    }
}

int
main(int argc, char *argv[])
{
    int mpi_thread_lvl_provided = -1;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_thread_lvl_provided);
    assert(MPI_THREAD_MULTIPLE == mpi_thread_lvl_provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &MY_RANK);
    MPI_Comm_size(MPI_COMM_WORLD, &NUM_RANKS);
    MPI_Comm           comm    = MPI_COMM_WORLD;
    MPI_Info           info    = MPI_INFO_NULL;
    char *             num_str = "1024 Ks";
    unsigned long long num     = 0;

    char buffer[200];

    if (MY_RANK == 0) {
        if (argc != 3) {
            print_usage(argv[0]);
            return 0;
        }
    }

    /* READ CONFIGURATION */
    char *       output_file;
    bench_params params;

    char *cfg_file_path = argv[1];
    output_file         = argv[2];
    if (MY_RANK == 0) {
        printf("Configuration file: %s\n", argv[1]);
        printf("Output data file: %s\n", argv[2]);
    }
    int do_write = 1;
    if (read_config(cfg_file_path, &params, do_write) < 0) {
        if (MY_RANK == 0)
            printf("Configuration file read failed. Please, check %s\n", cfg_file_path);
        return 0;
    }

    if (params.io_op != IO_WRITE) {
        if (MY_RANK == 0)
            printf("Make sure the configuration file has IO_OPERATION=WRITE defined\n");
        return 0;
    }

    if (params.useCompress)
        params.data_coll = 1;

    if (params.subfiling)
        subfiling = 1;

#if H5_VERSION_GE(1, 13, 0)
    if (H5VLis_connector_registered_by_name("async")) {
        if (MY_RANK == 0) {
            printf("Using 'async' VOL connector\n");
        }
    }
#endif

    if (MY_RANK == 0) {
        print_params(&params);
    }

    set_globals(&params);

    NUM_TIMESTEPS = params.cnt_time_step;
    /* END READ CONFIGURATION */

    if (MY_RANK == 0) {
        printf("Start benchmark: h5bench_write\n");
        printf("Number of particles per rank: %llu M\n", NUM_PARTICLES / (1024 * 1024));
    }

    unsigned long total_write_size =
        NUM_RANKS * NUM_TIMESTEPS * NUM_PARTICLES * (7 * sizeof(float) + sizeof(int));
    hid_t         filespace = 0, memspace = 0;
    unsigned long data_size             = 0;
    unsigned long data_preparation_time = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Allreduce(&NUM_PARTICLES, &TOTAL_PARTICLES, 1, MPI_LONG_LONG, MPI_SUM, comm);
    MPI_Scan(&NUM_PARTICLES, &FILE_OFFSET, 1, MPI_LONG_LONG, MPI_SUM, comm);

    FILE_OFFSET -= NUM_PARTICLES;

    if (MY_RANK == 0)
        printf("Total number of particles: %lldM\n", TOTAL_PARTICLES / (M_VAL));

    hid_t fapl      = set_fapl();
    ALIGN           = params.align;
    ALIGN_THRESHOLD = params.align_threshold;
    ALIGN_LEN       = params.align_len;

    if (params.file_per_proc) {
    }
    else {
#ifdef HAVE_SUBFILING
        if (params.subfiling == 1)
            H5Pset_fapl_subfiling(fapl, NULL);
        else
#endif
            H5Pset_fapl_mpio(fapl, comm, info);
        set_metadata(fapl, ALIGN, ALIGN_THRESHOLD, ALIGN_LEN, params.meta_coll);
    }

    void *data = _prepare_data(params, &filespace, &memspace, &data_preparation_time, &data_size);

    unsigned long t1 = get_time_usec();

    hid_t file_id;

    unsigned long tfopen_start = get_time_usec();
    if (params.file_per_proc) {
        char mpi_rank_output_file_path[4096];
        sprintf(mpi_rank_output_file_path, "%s/rank_%d_%s", get_dir_from_path(output_file), MY_RANK,
                get_file_name_from_path(output_file));

        file_id = H5Fcreate_async(mpi_rank_output_file_path, H5F_ACC_TRUNC, H5P_DEFAULT, fapl, 0);
    }
    else {
        file_id = H5Fcreate_async(output_file, H5F_ACC_TRUNC, H5P_DEFAULT, fapl, 0);
    }
    unsigned long tfopen_end = get_time_usec();

    if (MY_RANK == 0)
        printf("Opened HDF5 file... \n");

    MPI_Barrier(MPI_COMM_WORLD);
    unsigned long t2 = get_time_usec(); // t2 - t1: metadata: creating/opening

    unsigned long raw_write_time, inner_metadata_time, local_data_size;
    int           stat = _run_benchmark_write(params, file_id, fapl, filespace, memspace, data, data_size,
                                    &local_data_size, &raw_write_time, &inner_metadata_time);

    if (stat < 0) {
        if (MY_RANK == 0)
            printf("\n==================== Benchmark Failed ====================\n");
        assert(0);
    }

    unsigned long t3 = get_time_usec(); // t3 - t2: writting data, including metadata

    H5Pclose(fapl);
    unsigned long tflush_start = get_time_usec();
    H5Fflush(file_id, H5F_SCOPE_LOCAL);
    MPI_Barrier(MPI_COMM_WORLD);
    unsigned long tflush_end = get_time_usec();

    unsigned long tfclose_start = get_time_usec();

    H5Fclose_async(file_id, 0);

    unsigned long tfclose_end = get_time_usec();
    MPI_Barrier(MPI_COMM_WORLD);
    unsigned long t4 = get_time_usec();

    if (MY_RANK == 0) {
        human_readable value;
        char *         mode_str = NULL;

        if (has_vol_async) {
            mode_str = "ASYNC";
        }
        else {
            mode_str = "SYNC";
        }
        printf("\n=================== Performance Results ==================\n");

        printf("Total number of ranks: %d\n", NUM_RANKS);

        unsigned long long total_sleep_time_us =
            read_time_val(params.compute_time, TIME_US) * (params.cnt_time_step - 1);
        printf("Total emulated compute time: %.3lf s\n", total_sleep_time_us / (1000.0 * 1000.0));

        double total_size_bytes = NUM_RANKS * local_data_size;
        value                   = format_human_readable(total_size_bytes);
        printf("Total write size: %.3lf %cB\n", value.value, value.unit);

        float rwt_s    = (float)raw_write_time / (1000.0 * 1000.0);
        float raw_rate = (float)total_size_bytes / rwt_s;
        printf("Raw write time: %.3f s\n", rwt_s);

        float meta_time_s = (float)inner_metadata_time / (1000.0 * 1000.0);
        printf("Metadata time: %.3f s\n", meta_time_s);

        float fcreate_time_s = (float)(tfopen_end - tfopen_start) / (1000.0 * 1000.0);
        printf("H5Fcreate() time: %.3f s\n", fcreate_time_s);

        float flush_time_s = (float)(tflush_end - tflush_start) / (1000.0 * 1000.0);
        printf("H5Fflush() time: %.3f s\n", flush_time_s);

        float fclose_time_s = (float)(tfclose_end - tfclose_start) / (1000.0 * 1000.0);
        printf("H5Fclose() time: %.3f s\n", fclose_time_s);

        float oct_s = (float)(t4 - t1) / (1000.0 * 1000.0);
        printf("Observed completion time: %.3f s\n", oct_s);

        value = format_human_readable(raw_rate);
        printf("%s Raw write rate: %.3f %cB/s \n", mode_str, value.value, value.unit);

        float or_bs = (float)total_size_bytes / ((float)(t4 - t1 - total_sleep_time_us) / (1000.0 * 1000.0));
        value       = format_human_readable(or_bs);
        printf("%s Observed write rate: %.3f %cB/s\n", mode_str, value.value, value.unit);

        printf("===========================================================\n");

        if (params.useCSV) {
            fprintf(params.csv_fs, "metric, value, unit\n");
            fprintf(params.csv_fs, "operation, %s, %s\n", "write", "");
            fprintf(params.csv_fs, "ranks, %d, %s\n", NUM_RANKS, "");
            fprintf(params.csv_fs, "collective data, %s, %s\n", params.data_coll == 1 ? "YES" : "NO", "");
            fprintf(params.csv_fs, "collective meta, %s, %s\n", params.meta_coll == 1 ? "YES" : "NO", "");
            fprintf(params.csv_fs, "subfiling, %s, %s\n", params.subfiling == 1 ? "YES" : "NO", "");
            fprintf(params.csv_fs, "total compute time, %.3lf, %s\n", total_sleep_time_us / (1000.0 * 1000.0),
                    "seconds");
            value = format_human_readable(total_size_bytes);
            fprintf(params.csv_fs, "total size, %.3lf, %cB\n", value.value, value.unit);
            fprintf(params.csv_fs, "raw time, %.3f, %s\n", rwt_s, "seconds");
            value = format_human_readable(raw_rate);
            fprintf(params.csv_fs, "raw rate, %.3lf, %cB/s\n", value.value, value.unit);
            fprintf(params.csv_fs, "metadata time, %.3f, %s\n", meta_time_s, "seconds");
            value = format_human_readable(or_bs);
            fprintf(params.csv_fs, "observed rate, %.3f, %cB/s\n", value.value, value.unit);
            fprintf(params.csv_fs, "observed time, %.3f, %s\n", oct_s, "seconds");
            fclose(params.csv_fs);
        }
    }

    MPI_Finalize();
    return 0;
}
