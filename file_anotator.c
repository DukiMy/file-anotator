/* This program annotates files with basic info regarding its creator, date of creation
 * and the time it was last changed.
 *
 * Created by Durim Miziraj
 */

/* Imports */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/* Pre-defined constants */
#define CORNERS '+'
#define HORIZONTAL_WALLS '-'
#define VERTICAL_WALLS '|'
#define NEW_LINE '\n'
#define NULL_TERM '\0'
#define TAB '\t'
#define INPUT_FLAG "-i"
#define OUTPUT_FLAG "-o"
#define SUCCESS 0

/* Declarting methods */
void get_dims(const char *str, size_t *width, size_t *rows);
void horizontal_border(size_t width, char symb, char **write_head);
void body(size_t width, const char **input_buffer, char **write_head);
char *boxed_text(char symb, const char *input_buffer, size_t *fsize);
int prepend(const char *filepath, const char *prepend_text);
int box_from_file(char *input_path, char * output_path);
int check_args(int argc,  char *argv[], char **input_path, char **output_path);
int main(int argc, char *argv[]);

/* The programs main entry point */
int main(int argc, char *argv[]) {
  char *input_path;
  char *output_path;

  check_args(argc, argv, &input_path, &output_path);
  box_from_file(input_path, output_path);

  return 0;
}

/*
 * Checks for valid arguments, exits the program if none are provided.
 */
int check_args(const int argc, char *argv[], char **input_path, char **output_path) {
  
  /* Hinders any further execution if no args are provided. */
  if (argc == 1) {
    fprintf(stderr, "Missing input, output or both.\n");
    printf("You need to provide arguments with the flags '-i' and '-o'.\n");
    exit(EXIT_FAILURE);
  }

  /*
   * Searches for the option flags, exits if none are provided.
   */
  int opt;
  while ((opt = getopt(argc, argv, "i:o:?")) != -1) {
    switch (opt) {
      case 'i':
        if (!optarg) {
          fprintf(stderr, "The invalid input argument provided was '%s'\n", optarg);
          exit(EXIT_FAILURE);
        }
        if (access(optarg, F_OK) != 0) {
          fprintf(stderr, "The input file '%s' could not be found.\n", optarg);
          exit(EXIT_FAILURE);
        }
        *input_path = optarg;
        break;
      case 'o':
        if (!optarg) {
          fprintf(stderr, "The invalid output argument provided was '%s'\n", optarg);
          exit(EXIT_FAILURE);
        }
        if (access(optarg, F_OK) != 0) {
          fprintf(stderr, "The output file '%s' could not be found.\n", optarg);
          exit(EXIT_FAILURE);
        }
        *output_path = optarg;
        break;
      case '?':
        printf("Usage:\n  ./file_anotator.out -i <input_file_path> -o <output_file_path>\n");
        exit(EXIT_SUCCESS);
    }
  }

  if (!input_path || !output_path) {
    fprintf(stderr, "Missing input, output or both.\n");
    exit(EXIT_FAILURE);
  }
  return 0;
}

int prepend(const char *filepath, const char *prepend_text) {
  FILE *original = fopen(filepath, "rb"); /* "rb" makes it readonly, it opens the file which is meant be prepended. */
  if (!original) return 1;                /* returns with an error if the file does not exist */

  /* Getting the size of the file */
  fseek(original, 0, SEEK_END);
  long file_size = ftell(original);
  rewind(original);

  /* Getting the length of the text that is to be prepended */
  size_t prepend_len = strlen(prepend_text);
  char *buffer = malloc(prepend_len + file_size + 1);
  if (!buffer) return 2;

  memcpy(buffer, prepend_text,  prepend_len);
  fread(buffer + prepend_len, 1, file_size, original);
  buffer[prepend_len + file_size] = '\0';
  char *boxed_text_buffer = boxed_text(HORIZONTAL_WALLS, buffer, &prepend_len);

  fclose(original);

  FILE *out = fopen(filepath, "wb");
  if (!out) return 3;
  fwrite(boxed_text_buffer, 1, prepend_len + file_size, out);
  fclose(out);
  free(buffer);

  return 0;
}

int box_from_file(char *input_path, char *output_path) {
  FILE *input_stream = fopen(input_path, "rb");
  fseek(input_stream, 0, SEEK_END);
  size_t fsize = ftell(input_stream);
  rewind(input_stream);

  char *input_buffer = malloc(fsize + 1);
  fread(input_buffer, 1, fsize, input_stream);
  fclose(input_stream);

  input_buffer[fsize] = '\0';
  char *boxed_text_buffer = boxed_text(HORIZONTAL_WALLS, input_buffer, &fsize);
  free(input_buffer);
  FILE *output_stream = fopen(output_path, "wb");
  fwrite(boxed_text_buffer, 1, fsize, output_stream);
  fclose(output_stream);
  free(boxed_text_buffer);

  return SUCCESS;
}

int box_from_string_to_stdo(char string[]) {
  FILE *input_stream = fopen("stringlib.c", "rb");
  fseek(input_stream, 0, SEEK_END);
  size_t fsize = ftell(input_stream);
  rewind(input_stream);

  char *input_buffer = malloc(fsize + 1);
  fread(input_buffer, 1, fsize, input_stream);
  fclose(input_stream);

  input_buffer[fsize] = '\0';
  char *boxed_text_buffer = boxed_text(HORIZONTAL_WALLS, input_buffer, &fsize);
  free(input_buffer);
  FILE *output_stream = fopen("output.txt", "wb");
  fwrite(boxed_text_buffer, 1, fsize, output_stream);
  fclose(output_stream);
  free(boxed_text_buffer);
}

char *boxed_text(char symb, const char *input_buffer, size_t *fsize) {
  size_t width, rows;

  get_dims(input_buffer, &width, &rows);
  *fsize = (width + 3) * (rows + 2) + 1;

  char *buffer = malloc(*fsize);
  char *write_head = buffer;

  horizontal_border(width, symb, &write_head);
  body(width, &input_buffer, &write_head);
  horizontal_border(width, symb, &write_head);

  *write_head = NULL_TERM;
  *fsize = write_head - buffer;
  return buffer;
}

void get_dims(const char *str, size_t *width, size_t *rows) {
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
}

void horizontal_border(size_t width, char symb, char **write_head) {
  size_t i;

  *(*write_head)++ = CORNERS;
  for (i = 0; i < width; i++) *(*write_head)++ = symb;
  *(*write_head)++ = CORNERS;
  *(*write_head)++ = NEW_LINE;
}

void body(size_t width, const char **input_buffer, char **write_head) {
  const char *reader = *input_buffer;
  size_t col_count = 0;

  while (*reader) {
    if (col_count == 0) {
      *(*write_head)++ = VERTICAL_WALLS;
      *(*write_head)++ = ' ';
    }

    if (*reader == NEW_LINE) {
      // Fill remaining line
      for (; col_count < width; col_count++)
        *(*write_head)++ = ' ';

      *(*write_head)++ = VERTICAL_WALLS;
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
    *(*write_head)++ = VERTICAL_WALLS;
    *(*write_head)++ = NEW_LINE;
  }
}
