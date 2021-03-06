/******************************************************************************
 *                                                                            *
 *  Distributed Systems Project of the Andrés Bello Catholic University       *
 *                                                                            *
 *  Each of the processes read a specified number of blocks from a single     *
 *  input file, then encrypt and transpose the data and write the same number *
 *  of ciphered blocks in a new file.                                         *
 *                                                                            *
 *  Atahualpa Silva F <atahualpasilva@gmail.com>                              *
 *  Andrea L. Contreras D. <andre.contdi@gmail.com>                           *
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
 
/*#define DEBUG 1*/
#define MASTER_RANK 0
#define TRUE 1
#define FALSE 0
#define BOOLEAN int
#define MBYTE 1048576
#define COLUMN_SIZE 16
#define PRIME_NUMBER 17
#define CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~`!@#$%^&*()_+-=<>,.?/':{}|[\\]\""
#define SYNOPSIS printf ("synopsis: %s -f <inputfile> -o <outputfile> -e <boolean(1 or 0)> -k <key> \n", argv[0])

// Functions prototype
void encrypt(BOOLEAN wanna_encrypt, unsigned char *key_original, unsigned char *content, unsigned char **content_encrypt, int key_original_len, int vid_title_len, int content_len, int my_rank);
void transpose(unsigned char *read_buffer, unsigned char **write_buffer, int number_of_bytes, BOOLEAN wanna_encrypt, int my_rank);

void encrypt(BOOLEAN wanna_encrypt, unsigned char *key_original, unsigned char *content, unsigned char **content_encrypt, int key_original_len, int vid_title_len, int content_len, int my_rank) {
  int i, j, caesar = 0, characters_len = strlen(CHARACTERS), encrypt_iterator; /*Para recorridos y substitución.*/

  /*Se obtiene el valor de Cesar a utilizar.*/
  for (i = 0 ; i < key_original_len; i++) {    
    for (j = 0 ; j < characters_len; j++) {
      if (key_original[i] == CHARACTERS[j]) {
        caesar += j;
        break;
      }
    }
  }
  caesar += key_original_len;
  caesar = (caesar * PRIME_NUMBER)%(characters_len-1);
  #ifdef DEBUG
    printf("%d: Estoy en el método encrypt", my_rank);
    if (wanna_encrypt) printf("%d:Encrypt with Cesar + %d\n", my_rank, caesar);
    else printf("%d: Decrypt with Cesar - %d\n", my_rank, caesar);
  #endif

  /*Se hace el cifrado de la frase o contenido del archivo con Cesar.*/
  for (i = 0 ; i < content_len; i++) {
    if (wanna_encrypt == TRUE) {
      (*content_encrypt)[i] = content[i] + caesar;
    } else {
      if (wanna_encrypt == FALSE) {
        (*content_encrypt)[i] = content[i] - caesar;
      }
    }
  }
}

void transpose(unsigned char *read_buffer, unsigned char **write_buffer, int number_of_bytes, BOOLEAN wanna_encrypt, int my_rank) {
  int i, rows_number, extra_data, iRow, jCol, cont_aux, columns_number;
  if (number_of_bytes<(COLUMN_SIZE*2))
    columns_number = number_of_bytes/4;
  else
    columns_number = COLUMN_SIZE;
  rows_number = number_of_bytes / columns_number;
  extra_data = number_of_bytes % columns_number;

  #ifdef DEBUG
    printf("%d: Estoy en el método transpose\n", my_rank);
    printf("%d: filas=%d columnas=%d con number_of_bytes=%d\n", my_rank, rows_number, COLUMN_SIZE, number_of_bytes);
  #endif

  if (wanna_encrypt) {
    #ifdef DEBUG
      if (my_rank == 0) {
        printf("\nMATRIZ ORIGINAL\n");
        for (iRow =0 ; iRow < rows_number ; iRow++) {
          printf("\n");
          for (jCol =0 ; jCol < columns_number ; jCol++) {
            cont_aux = iRow * columns_number + jCol;
            printf("%02X\t", read_buffer[cont_aux]);
          }
        }
        printf("\n");
        while (cont_aux < number_of_bytes - 1) {
          cont_aux++;
          printf("%02X\t", read_buffer[cont_aux]);
        }
      }
    #endif

    for (jCol =0 ; jCol < columns_number ; jCol++) {
      for (iRow =0 ; iRow < rows_number ; iRow++) {
        cont_aux = jCol * rows_number + iRow;
        (*write_buffer)[cont_aux] = read_buffer[iRow * columns_number + jCol];
      }
    }

    while (cont_aux < number_of_bytes) {
      cont_aux++;
      (*write_buffer)[cont_aux] = read_buffer[cont_aux];
    }

    #ifdef DEBUG
      if (my_rank == 0) {
        printf("\n\nMATRIZ MODIFICADA\n");
        for (jCol =0 ; jCol < columns_number ; jCol++) {
          printf("\n");
          for (iRow =0 ; iRow < rows_number ; iRow++) {
            cont_aux = jCol * rows_number + iRow;
            printf("%02X\t", (*write_buffer)[cont_aux]);
          }
        }
        printf("\n");
        while (cont_aux < number_of_bytes - 1) {
          cont_aux++;
          printf("%02X\t", read_buffer[cont_aux]);
          if ((cont_aux+1) % rows_number == 0) printf("\n");
        }
      }
    #endif

  } else {
    #ifdef DEBUG
      if (my_rank == 0) {
        printf("\nMATRIZ ORIGINAL\n");
        for (jCol =0 ; jCol < columns_number ; jCol++) {
          printf("\n");
          for (iRow =0 ; iRow < rows_number ; iRow++) {
            cont_aux = jCol * rows_number + iRow;
            printf("%02X\t", read_buffer[cont_aux]);
          }
        }
        printf("\n");
        while (cont_aux < number_of_bytes - 1) {
          cont_aux++;
          printf("%02X\t", read_buffer[cont_aux]);
          if ((cont_aux+1) % rows_number == 0) printf("\n");
        }
      }
    #endif

    for (iRow =0 ; iRow < rows_number ; iRow++) {
      for (jCol =0 ; jCol < columns_number ; jCol++) {
        cont_aux = iRow * columns_number + jCol;
        (*write_buffer)[cont_aux] = read_buffer[jCol * rows_number + iRow];
      }
    }

    while (cont_aux < number_of_bytes) {
      cont_aux++;
      (*write_buffer)[cont_aux] = read_buffer[cont_aux];
    }

    #ifdef DEBUG
      if (my_rank == 0) {
        printf("\n\nMATRIZ MODIFICADA\n");
        for (iRow =0 ; iRow < rows_number ; iRow++) {
          printf("\n");
          for (jCol =0 ; jCol < columns_number ; jCol++) {
            cont_aux = iRow * columns_number + jCol;
            printf("%02X\t", (*write_buffer)[cont_aux]);
          }
        }
        printf("\n");
        while (cont_aux < number_of_bytes - 1) {
          cont_aux++;
          printf("%02X\t", read_buffer[cont_aux]);
        }
      }
    #endif
  }
}
 
int main(argc, argv)
  int argc;
  char *argv[];
{
  /* my variables */ 
  int my_rank, pool_size, last_guy, i, count, read_filename_length, write_filename_length, file_open_error, number_of_bytes, key_original_len;
  BOOLEAN i_am_the_master = FALSE, input_error = FALSE, wanna_encrypt = TRUE;
  unsigned char *read_filename = NULL, *read_buffer, *write_filename = NULL, *write_buffer, *key_original = NULL; // Read and write file variables
  int *junk;
  
  /* MPI_Offset is long long */ 
  MPI_Offset my_offset, my_current_offset, total_number_of_bytes, number_of_bytes_ll, max_number_of_bytes_ll;
  MPI_File read_fh, write_fh;
  MPI_Status status;
  double start, finish, io_time, longest_io_time, start_program, finish_program, ejecution_time, total_ejecution_time;
 
  /* getopt variables */ 
  extern char *optarg;
  int c;
 
  /* ACTION */ 
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &pool_size);
  start_program = MPI_Wtime();
  last_guy = pool_size - 1;
  if (my_rank == MASTER_RANK) i_am_the_master = TRUE;
 
  if (i_am_the_master) {
 
    /* read the command line */
 
    while ((c = getopt(argc, argv, "f:o:e:k:h")) != EOF)
      switch(c) {
        case 'f':
          read_filename = optarg;
          #ifdef DEBUG
            printf("input file: %s\n", read_filename);
          #endif
          break;
        case 'o':
          write_filename = optarg;
          #ifdef DEBUG
            printf("output file: %s\n", write_filename);
          #endif
          break;
        case 'e':
          if ((sscanf (optarg, "%d", &wanna_encrypt) != 1) || (wanna_encrypt != 0 && wanna_encrypt != 1)) {
            SYNOPSIS;
            input_error = TRUE;
          }
          #ifdef DEBUG
            printf("encrypt: %d\n", wanna_encrypt);
          #endif
          break;
        case 'k':
          key_original = optarg;
          #ifdef DEBUG
            printf("key: %s\n", key_original);
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
 
    if (read_filename == NULL || write_filename == NULL || key_original == NULL) {
      SYNOPSIS;
      input_error = TRUE;
    }
 
    if (input_error) MPI_Abort(MPI_COMM_WORLD, 1);
 
    read_filename_length = strlen(read_filename) + 1;
    write_filename_length = strlen(write_filename) + 1;
    key_original_len = strlen(key_original);
 
    /* This is another way of exiting, but it can be done only
       if no files have been opened yet. */
 
  } /* end of "if (i_am_the_master)"; reading the command line */
 
    /* If we got this far, the data read from the command line
       should be OK. */
 
  MPI_Bcast(&read_filename_length, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
  MPI_Bcast(&write_filename_length, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
  MPI_Bcast(&key_original_len, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
  MPI_Bcast(&wanna_encrypt, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
  if (! i_am_the_master) {
    read_filename = (unsigned char *) malloc(read_filename_length);
    write_filename = (unsigned char *) malloc(write_filename_length);
    key_original = (unsigned char *) malloc(key_original_len);
  }
#ifdef DEBUG
  printf("%3d: allocated space for read_filename\n", my_rank);
#endif
  MPI_Bcast(read_filename, read_filename_length, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
  MPI_Bcast(write_filename, write_filename_length, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
  MPI_Bcast(key_original, key_original_len, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
#ifdef DEBUG
  printf("%3d: received broadcast\n", my_rank);
  printf("%3d: read_filename = %s\n", my_rank, read_filename);
  printf("%3d: write_filename = %s\n", my_rank, write_filename);
#endif
 
  MPI_Barrier(MPI_COMM_WORLD);
 
  /* Default I/O error handling is MPI_ERRORS_RETURN */
 
  file_open_error = MPI_File_open(MPI_COMM_WORLD, read_filename,
                          MPI_MODE_RDONLY | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &read_fh);

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
    transpose(read_buffer, &write_buffer, number_of_bytes, wanna_encrypt, my_rank);
    //read_buffer = (unsigned char *) malloc(number_of_bytes);
    memcpy(read_buffer, write_buffer, number_of_bytes);
    encrypt(wanna_encrypt, key_original, read_buffer, &write_buffer, key_original_len, strlen(write_filename)-4, number_of_bytes, my_rank);
    MPI_File_write(write_fh, write_buffer, number_of_bytes, MPI_BYTE, &status);
    #ifdef DEBUG
      int m;
      for (m = 0; m < number_of_bytes ; m++) {
        if (m<5) {
          printf("READ: SOY RANK%d %02X %d %d\n", my_rank, read_buffer[m], m + (int) my_offset, m);
          printf("WRITE: SOY RANK%d %02X %d %d\n", my_rank, write_buffer[m], m + (int) my_offset, m);
        }
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

  finish_program = MPI_Wtime();
  ejecution_time = finish_program - start_program;
  printf("%d: Ejecution time: %f seconds\n", my_rank, ejecution_time);
  MPI_Allreduce(&ejecution_time, &total_ejecution_time, 1, MPI_DOUBLE, MPI_SUM,
          MPI_COMM_WORLD);
  if (i_am_the_master) {
    printf("%d: Avg ejecution time: %f seconds\n", my_rank, total_ejecution_time / pool_size); 
  }
 
  MPI_File_close(&read_fh);
  MPI_File_close(&write_fh);
 
  MPI_Finalize();
  exit(0);
}
