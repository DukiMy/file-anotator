/**
 * @file file_annotator
 * @brief Main source file for annotating files with metadata.
 * @author Durim Miziraj
 * @date 2025-07-26
 *
 * This file implements the main logic for reading, modifying,
 * and writing.
 */

/* Imports */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/* Pre-defined constants */
#define NEW_LINE '\n'
#define NULL_TERM '\0'

typedef struct IO_filestream {
  FILE *infile;
  FILE *outfile;
  char *infile_path;
  char *outfile_path;
} IO_filestream;

typedef struct Infile {
  char *path;
  uint8_t exists;
  FILE stream;
} Infile;

typedef struct box_char {
  const char CORNER;
  const char VERT_WALL;
  const char HOR_WALL;
} box_char;

static const box_char BOX_CHAR = { 
  '+',  /* CORNER     */
  '|',  /* VERT_WALL  */
  '-'   /* HOR_WALL   */
};

/* Declaring methods */
int get_dims(const char *str, size_t *width, size_t *rows);
int horizontal_border(size_t width, char **write_head);
int body(size_t width, const char **working_buff, char **write_head);
char *boxed_text(const char *working_buff, size_t *fsize);
int prepend(const char *filepath, const char *prepend_text);
// TODO: Rename this into something more fitting
void assign_file_path(int argc,  char *argv[], IO_filestream *fs);
int main(int argc, char *argv[]);

/* The programs main entry point */
int main(int argc, char *argv[]) {
  IO_filestream fs = {
    .infile = NULL,
    .outfile = NULL,
    .infile_path = "null",
    .outfile_path = "null"
  };

  Infile input = {
    .path = "change this",
    .exists = 0x00,

  }

  assign_file_path(argc, argv, &fs);

  return EXIT_SUCCESS;
}

char *get_path() {

}

FILE set_infile(const int argc, char *argv[]) {
  char opt;
  while ((opt = getopt(argc, argv, "i")) != -1) {
    if (opt == 'i' && optarg && access(optarg, F_OK) == 0) {

      
      
    }
  }
}

/*
 * Checks for valid arguments, exits the program if it fails.
 */
void assign_file_path(const int argc, char *argv[], IO_filestream *fs) {
  char opt;

  while ((opt = getopt(argc, argv, "i:o:?:h")) != -1) {
    switch (opt) {

      case 'i':
        if (!optarg) {
          fprintf(
            stderr,
            "The invalid input argument provided was '%s'\n",
            optarg
          );
          exit(EXIT_FAILURE);
        }
        
        if (access(optarg, F_OK) != 0) {
          fprintf(
            stderr,
            "The input file '%s' could not be found.\n",
            optarg
          );
          exit(EXIT_FAILURE);
        }

        // TODO: Break out this operation
        // Open the read-only filestream with the optarg filepath
        fs->infile = fopen(optarg, "rb");
        // NOTE: fseek should only be used on files. data on random access, or use defined by stdin does not work with this.
        // Go to the end of the filestream
        fseek(fs->infile, 0, SEEK_END);
        // Tell the position of the pointer. Essentially tells the length of the filestream, because we traversed to its end previously. 
        size_t fsize = ftell(fs->infile);
        // Sets the pointer back to the beginning of the filestream. This making it ready to be used.
        rewind(fs->infile);
        // Allocates memory
        char *working_buff = malloc(fsize + 1);
        // Stores the the data of the input filestream to the input buffer
        fread(working_buff, 1, fsize, fs->infile);
        // Input filestream is no longer needed, and therefore closed
        fclose(fs->infile);
        // Adds a null terminator to the end of the buffer
        working_buff[fsize] = NULL_TERM;
        // Stores the boxed text at the input buffer.
        working_buff = boxed_text(working_buff, &fsize);
        // free(working_buff);
      break;

      case 'o':
        if (!optarg) {
          fprintf(
            stderr,
            "The invalid output argument provided was '%s'\n",
            optarg
          );
          exit(EXIT_FAILURE);
        }

        if (access(optarg, F_OK) != 0) {
          fprintf(
            stderr,
            "The output file '%s' could not be found.\n",
            optarg
          );
          exit(EXIT_FAILURE);
        }

        // TODO: Break out this operation
        // Open the filestream with the output path provided by optarg
        fs->outfile = fopen(optarg, "wb");
        // Write the contents of the working buffer to the output stream
        fwrite(working_buff, 1, fsize, fs->outfile);
        // Close the output stream
        fclose(fs->outfile);
        // Free the memory buffer
        free(working_buff);

      break;

      case '?': case 'h':
        printf(
          "Usage:\n  ./file_anotator.out -i <input_file_path> -o <output_file_path>\n"
        );
      exit(EXIT_SUCCESS);

      default:
        fprintf(
          stderr,
          "Argument '%s' was not recognized.\nUse flags '-h' or '-?'.",
          optarg
        );
      exit(EXIT_FAILURE);
    }
  }
}

char *boxed_text(const char *working_buff, size_t *fsize) {
  size_t width, rows;

  get_dims(working_buff, &width, &rows);
  *fsize = (width + 3) * (rows + 2) + 1;

  char *buffer = malloc(*fsize);
  char *write_head = buffer;

  horizontal_border(width, &write_head);
  body(width, &working_buff, &write_head);
  horizontal_border(width, &write_head);

  *write_head = NULL_TERM;
  *fsize = write_head - buffer;

  return buffer;
}

int get_dims(const char *str, size_t *width, size_t *rows) {
  *width = 0;
  *rows = 0;
  size_t current = 0;
  const char *p;

  for (p = str; *p; p++) {
    if (*p == NEW_LINE) {
      if (current > *width) *width = current;
      current = 0;
      (*rows)++;
    } else {
      current++;
    }
  }
  if (current > *width) *width = current;

  return EXIT_SUCCESS;
}

int horizontal_border(size_t width, char **write_head) {
  size_t i;

  *(*write_head)++ = BOX_CHAR.CORNER;
  for (i = 0; i < width; i++) *(*write_head)++ = BOX_CHAR.HOR_WALL;
  *(*write_head)++ = BOX_CHAR.CORNER;
  *(*write_head)++ = NEW_LINE;

  return EXIT_SUCCESS;
}

int body(size_t width, const char **working_buff, char **write_head) {
  const char *reader = *working_buff;
  size_t col_count = 0;

  while (*reader) {
    if (col_count == 0) {
      *(*write_head)++ = BOX_CHAR.VERT_WALL;
      *(*write_head)++ = ' ';
    }

    if (*reader == NEW_LINE) {
      // Fill remaining line
      for (; col_count < width; col_count++)
        *(*write_head)++ = ' ';

      *(*write_head)++ = BOX_CHAR.VERT_WALL;
      *(*write_head)++ = NEW_LINE;
      col_count = 0;
      reader++;
    } else {
      *(*write_head)++ = *reader++;
      col_count++;
    }
  }

  // Final line (if not newline-terminated)
  if (col_count > 0) {
    for (; col_count < width; col_count++)
      *(*write_head)++ = ' ';
    *(*write_head)++ = BOX_CHAR.VERT_WALL;
    *(*write_head)++ = NEW_LINE;
  }

  return EXIT_SUCCESS;
}

