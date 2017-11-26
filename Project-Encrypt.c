/******************************************************************************
 *                                                                            *
 *  MPI IO Example - Reading from MPI Files                                   *
 *                                                                            *
 *  Each of the processes read a specified number of blocks from a            *
 *  single input file.                                                        *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  The original code was written by Gustav at University of Indiana in 2003. *
 *                                                                            *
 *  The current version has been tested/updated by the HPC department at      *
 *  the Norwegian University of Science and Technology in 2011.               *
 *                                                                            *
 ******************************************************************************/
#include <stdio.h>   /* all IO stuff lives here */
#include <stdlib.h>  /* exit lives here */
#include <unistd.h>  /* getopt lives here */
#include <string.h>  /* strcpy lives here */
#include <limits.h>  /* INT_MAX lives here */
#include <mpi.h>     /* MPI and MPI-IO live here */
 
#define DEBUG 1
#define MASTER_RANK 0
#define TRUE 1
#define FALSE 0
#define BOOLEAN int
#define MBYTE 1048576
#define COLUMN_SIZE 64
#define SYNOPSIS printf ("synopsis: %s -f <file>\n", argv[0])

// Functions prototype
void transpose(unsigned char *read_buffer, unsigned char **write_buffer, int number_of_bytes);

void transpose(unsigned char *read_buffer, unsigned char **write_buffer, int number_of_bytes) {
  int i;
  for (i=0 ; i<number_of_bytes ; i++) {
    (*write_buffer)[(i*COLUMN_SIZE)%number_of_bytes] = read_buffer[i];
  }
}
 
int main(argc, argv)
     int argc;
     char *argv[];
{
  /* my variables */
 
  int my_rank, pool_size, last_guy, i, count;
  BOOLEAN i_am_the_master = FALSE, input_error = FALSE;
  unsigned char *read_filename = NULL, *read_buffer; // Read file variables
  unsigned char *write_filename = "c2.mp4", *write_buffer; // Write file variables
  int filename_length;
  int *junk;
  int file_open_error, number_of_bytes;
 
  /* MPI_Offset is long long */
 
  MPI_Offset my_offset, my_current_offset, total_number_of_bytes, number_of_bytes_ll, max_number_of_bytes_ll;
  MPI_File read_fh, write_fh;
  MPI_Status status;
  double start, finish, io_time, longest_io_time;
 
  /* getopt variables */
 
  extern char *optarg;
  int c;
 
  /* ACTION */
 
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &pool_size);
  last_guy = pool_size - 1;
  if (my_rank == MASTER_RANK) i_am_the_master = TRUE;
 
  if (i_am_the_master) {
 
    /* read the command line */
 
    while ((c = getopt(argc, argv, "f:h")) != EOF)
      switch(c) {
      case 'f':
    read_filename = optarg;
#ifdef DEBUG
    printf("input file: %s\n", read_filename);
#endif
    break;
      case 'h':
    SYNOPSIS;
    input_error = TRUE;
    break;
      case '?':
    SYNOPSIS;
    input_error = TRUE;
    break;
      } /* end of switch(c) */
 
    /* Check if the command line has initialized read_filename and
     * number_of_blocks.
     */
 
    if (read_filename == NULL) {
      SYNOPSIS;
      input_error = TRUE;
    }
 
    if (input_error) MPI_Abort(MPI_COMM_WORLD, 1);
 
    filename_length = strlen(read_filename) + 1;
 
    /* This is another way of exiting, but it can be done only
       if no files have been opened yet. */
 
  } /* end of "if (i_am_the_master)"; reading the command line */
 
    /* If we got this far, the data read from the command line
       should be OK. */
 
  MPI_Bcast(&filename_length, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
  if (! i_am_the_master) {
    read_filename = (unsigned char *) malloc(filename_length);
    //write_filename = (unsigned char *) malloc(filename_length);
  }
#ifdef DEBUG
  printf("%3d: allocated space for read_filename\n", my_rank);
#endif
  MPI_Bcast(read_filename, filename_length, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
#ifdef DEBUG
  printf("%3d: received broadcast\n", my_rank);
  printf("%3d: read_filename = %s\n", my_rank, read_filename);
#endif
 
  MPI_Barrier(MPI_COMM_WORLD);
 
  /* Default I/O error handling is MPI_ERRORS_RETURN */
 
  file_open_error = MPI_File_open(MPI_COMM_WORLD, read_filename,
                          MPI_MODE_RDONLY, MPI_INFO_NULL, &read_fh);

  file_open_error = MPI_File_open(MPI_COMM_WORLD, write_filename,
                MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &write_fh);
 
  if (file_open_error != MPI_SUCCESS) {
 
    char error_string[BUFSIZ];
    int length_of_error_string, error_class;
 
    MPI_Error_class(file_open_error, &error_class);
    MPI_Error_string(error_class, error_string, &length_of_error_string);
    printf("%3d: %s\n", my_rank, error_string);
 
    MPI_Error_string(file_open_error, error_string, &length_of_error_string);
    printf("%3d: %s\n", my_rank, error_string);
 
    MPI_Abort(MPI_COMM_WORLD, file_open_error);
  }
 
  MPI_File_get_size(read_fh, &total_number_of_bytes);
#ifdef DEBUG
  printf("%3d: total_number_of_bytes = %lld\n", my_rank, total_number_of_bytes);
#endif
 
  number_of_bytes_ll = total_number_of_bytes / pool_size;
 
  /* If pool_size does not divide total_number_of_bytes evenly,
     the last process will have to read more data, i.e., to the
     end of the file. */
 
  max_number_of_bytes_ll =
    number_of_bytes_ll + total_number_of_bytes % pool_size;
 
  if (max_number_of_bytes_ll < INT_MAX) {
 
    if (my_rank == last_guy)
      number_of_bytes = (int) max_number_of_bytes_ll;
    else
      number_of_bytes = (int) number_of_bytes_ll;
 
    read_buffer = (unsigned char *) malloc(number_of_bytes);
    write_buffer = (unsigned char *) malloc(number_of_bytes);
#ifdef DEBUG
    printf("%3d: allocated %d bytes\n", my_rank, number_of_bytes);
#endif
 
    my_offset = (MPI_Offset) my_rank * number_of_bytes_ll;
#ifdef DEBUG
    printf("%3d: my offset = %lld\n", my_rank, my_offset);
#endif
    MPI_File_seek(read_fh, my_offset, MPI_SEEK_SET);
 
    MPI_Barrier(MPI_COMM_WORLD);

    // Write file operarion
    MPI_File_seek(write_fh, my_offset, MPI_SEEK_SET);
 
    start = MPI_Wtime();
    MPI_File_read(read_fh, read_buffer, number_of_bytes, MPI_BYTE, &status);
    // Write file operation
    transpose(read_buffer, &write_buffer, number_of_bytes);
    MPI_File_write(write_fh, write_buffer, number_of_bytes, MPI_BYTE, &status);
    #ifdef DEBUG
      int m;
      for (m = 0; m < number_of_bytes ; m++) {
        if (m<5)
          printf("SOY RANK%d %02X %d %d\n", my_rank, read_buffer[m], m + (int) my_offset, m);
      }
    #endif
    finish = MPI_Wtime();
    MPI_Get_count(&status, MPI_BYTE, &count);
#ifdef DEBUG
    printf("%3d: read %d bytes\n", my_rank, count);
#endif
    MPI_File_get_position(read_fh, &my_offset);
#ifdef DEBUG
    printf("%3d: my offset = %lld\n", my_rank, my_offset);
#endif
 
    io_time = finish - start;
    MPI_Allreduce(&io_time, &longest_io_time, 1, MPI_DOUBLE, MPI_MAX,
          MPI_COMM_WORLD);
    if (i_am_the_master) {
      printf("longest_io_time       = %f seconds\n", longest_io_time);
      printf("total_number_of_bytes = %lld\n", total_number_of_bytes);
      printf("transfer rate         = %f MB/s\n",
         total_number_of_bytes / longest_io_time / MBYTE);
    }
  }
  else {
    if (i_am_the_master) {
      printf("Not enough memory to read the file.\n");
      printf("Consider running on more nodes.\n");
    }
  } /* of if(max_number_of_bytes_ll < INT_MAX) */
 
  MPI_File_close(&read_fh);
  MPI_File_close(&write_fh);
 
  MPI_Finalize();
  exit(0);
}
