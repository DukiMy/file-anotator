/**
 * @file file_annotator
 * @brief Main source file for annotating files with metadata.
 * @author Durim Miziraj
 * @date 2025-07-26
 *
 * This file implements the main logic for reading, modifying,
 * and writing annotated text files.
 */

/* Imports */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/* Pre-defined constants */
#define NEW_LINE '\n'
#define NULL_TERM '\0'

typedef struct IO_file_path {
  char *input_file_path;
  char *output_file_path;
} IO_file_path;

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

/* Declarting methods */
int get_dims(const char *str, size_t *width, size_t *rows);
int horizontal_border(size_t width, char **write_head);
int body(size_t width, const char **input_buffer, char **write_head);
char *boxed_text(const char *input_buffer, size_t *fsize);
int prepend(const char *filepath, const char *prepend_text);
int box_from_file(IO_file_path *file_path);
void assign_file_path(int argc,  char *argv[], IO_file_path *file_path);
int main(int argc, char *argv[]);



/* The programs main entry point */
int main(int argc, char *argv[]) {
  IO_file_path file_path = {
    .input_file_path = "null",
    .output_file_path = "null"
  };

  assign_file_path(argc, argv, &file_path);
  box_from_file(&file_path);

  return EXIT_SUCCESS;
}

/*
 * Checks for valid arguments, exits the program if it fails.
 */
void assign_file_path(const int argc, char *argv[], IO_file_path *file_path) {
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
        file_path->input_file_path = optarg;
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
        file_path->output_file_path = optarg;
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

int box_from_file(IO_file_path *file_path) {
  FILE *input_stream = fopen(file_path->input_file_path, "rb");
  fseek(input_stream, 0, SEEK_END);
  size_t fsize = ftell(input_stream);
  rewind(input_stream);

  char *input_buffer = malloc(fsize + 1);
  fread(input_buffer, 1, fsize, input_stream);
  fclose(input_stream);

  input_buffer[fsize] = NULL_TERM;
  char *boxed_text_buffer = boxed_text(input_buffer, &fsize);
  free(input_buffer);

  FILE *output_stream = fopen(file_path->output_file_path, "wb");
  fwrite(boxed_text_buffer, 1, fsize, output_stream);
  fclose(output_stream);
  free(boxed_text_buffer);

  return EXIT_SUCCESS;
}

char *boxed_text(const char *input_buffer, size_t *fsize) {
  size_t width, rows;

  get_dims(input_buffer, &width, &rows);
  *fsize = (width + 3) * (rows + 2) + 1;

  char *buffer = malloc(*fsize);
  char *write_head = buffer;

  horizontal_border(width, &write_head);
  body(width, &input_buffer, &write_head);
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

int body(size_t width, const char **input_buffer, char **write_head) {
  const char *reader = *input_buffer;
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

