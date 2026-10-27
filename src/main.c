#define GNU_SOURCE
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>
#include <time.h>
#include <limits.h>
#include <setjmp.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "include/xstrtol.h"

#define MAX_QUALITY 100
#define SIDE_MIN 1u
#define SIDE_MAX 65535u
#define JPEG_BYTES_PER_PIXEL 4.1
#define PNG_BYTES_PER_PIXEL 3.4

static uint32_t jpeg_side_estimate(uintmax_t target_bytes)
{
  double base_count = sqrt((double)target_bytes / JPEG_BYTES_PER_PIXEL);
  uint32_t side = (uint32_t)(base_count + 0.5);
  if (side < SIDE_MIN) side = SIDE_MIN;
  if (side > SIDE_MAX) side = SIDE_MAX;
  printf("%i\n", side);
  return side;
}

struct jpeg_error_context {
  struct jpeg_error_mgr pub;
  jmp_buf jump_buffer;
};

static void jpeg_error_exit(j_common_ptr cinfo)
{
  struct jpeg_error_context *err = (struct jpeg_error_context*)cinfo->err;
  longjmp(err->jump_buffer, 1);
}

int write_jpeg_file_by_line(const char *filename, int width, int height)
{
  if (filename == NULL || 
      filename[0] == '\0' ||
      width <= 0 ||
      height <= 0) {
    fprintf(stderr, "[ERROR]:\t Invalid filename or image dimension\n");
    return -1;
  }

  if ((size_t)width > SIZE_MAX / 3) {
    fprintf(stderr, "[ERROR]:\t Image row is too big\n");
    return -1;
  }


  size_t row_stride = (size_t)width * 3;

  if ((uintmax_t)width > UINT_MAX ||
      (uintmax_t)height > UINT_MAX) {
    
    fprintf(stderr, "[ERROR]:\t Image dimension exceed jpeg limits\n");
    return -1;
  }

  size_t name_len = strlen(filename);
  static const char suffix[] = ".tmp.XXXXXX";

  if (name_len > SIZE_MAX - sizeof(suffix)) {
    fprintf(stderr, "[ERROR]:\t Filename is too long\n");
    return -1;
  }

  char* temp_name = malloc(name_len + sizeof(suffix));

  if (temp_name == NULL) {
    perror("malloc");
    return -1;
  }
  
  memcpy(temp_name, filename, name_len);
  memcpy(temp_name + name_len, suffix, sizeof(suffix));

  int fd = mkstemp(temp_name);
  if (fd == -1) {
    perror("mkstemp");
    free(temp_name);
    return -1;
  }

  FILE* outfile = fdopen(fd, "wb");

  if (outfile == NULL) {
    perror("fdopen");
    close(fd);
    unlink(temp_name);
    free(temp_name);
    return -1;
  }

  struct jpeg_compress_struct cinfo = {0};
  struct jpeg_error_context jerr;

  unsigned char* row = NULL;
  int compressor_created = 0;
  int success = 0;
  

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = jpeg_error_exit;

  if (setjmp(jerr.jump_buffer)) {
    fprintf(stderr, "[ERROR]:\t JPEG compression failed\n");
    goto cleanup;
  }

  jpeg_create_compress(&cinfo);
  compressor_created = 1;

  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = (JDIMENSION)width;
  cinfo.image_height = (JDIMENSION)height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, MAX_QUALITY, TRUE);

  cinfo.comp_info[0].h_samp_factor = 1;
  cinfo.comp_info[0].v_samp_factor = 1;

  row = malloc(row_stride);

  if (row == NULL) {
    perror("malloc row");
    goto cleanup;
  }

  jpeg_start_compress(&cinfo, TRUE);

  while (cinfo.next_scanline < cinfo.image_height) {
    for (size_t i = 0; i < row_stride; ++i) {
      row[i] = (unsigned char)(rand() % 256);
    }
    JSAMPROW row_pointer = row;
    if (jpeg_write_scanlines(&cinfo, &row_pointer, 1) != 1) {
      fprintf(stderr, "[ERROR]:\t Could not write jpeg scanline\n");
      goto cleanup;
    }

  }
  jpeg_finish_compress(&cinfo);

  if (fclose(outfile) != 0) {
    outfile = NULL;
    perror("fclose");
    goto cleanup;
  }
  outfile = NULL;
  if (rename(temp_name, filename) !=0) {
    perror("rename");
    goto cleanup;
  }
  success = 1;

cleanup:
  free(row);
  if (compressor_created) {
    jpeg_destroy_compress(&cinfo);
  }
  
  if (outfile != NULL) {
    if (fclose(outfile) != 0) {
      perror("fclose");
      success = 0;
    }
  }

  if (success != 0) {
    unlink(temp_name);
  }
  free(temp_name);
  return success ? 0 : -1;
}

void write_jpeg_file(const char *filename, int width, int height)
{
  unsigned char* buffer = malloc(width * height * 3);

  for (int i = 0; i < width * height * 3; ++i)
  {
    buffer[i] = rand() % 256;
  }

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  FILE* outfile = fopen(filename, "wb");

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "Can't open %s\n", filename);
    exit(EXIT_FAILURE);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, MAX_QUALITY, TRUE);

  cinfo.comp_info[0].h_samp_factor = 1;
  cinfo.comp_info[0].v_samp_factor = 1;

  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW row_pointer;
  int row_stride = width * 3;
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer = (JSAMPROW)&buffer[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
  free(buffer);
}

static uint32_t png_side_estimate(uintmax_t target_bytes)
{
  double base_count = sqrt((double)target_bytes / PNG_BYTES_PER_PIXEL);
  uint32_t side = (uint32_t)(base_count + 0.5);
  if (side < SIDE_MIN) side = SIDE_MIN;
  if (side > SIDE_MAX) side = SIDE_MAX;
  printf("%i\n", side);
  return side;
}

void write_png_file(const char *filename, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("File opening failed");
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        width, height,
        8,                                 		// bit depth (8 bits per channel)
        PNG_COLOR_TYPE_RGBA,           // color type (Red, Green, Blue, Alpha)
        PNG_INTERLACE_ADAM7,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    png_bytep *row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png, info));
        for (int x = 0; x < width; ++x) {
            png_bytep px = &(row_pointers[y][x * 4]);
            px[0] = rand() % 256; // R
            px[1] = rand() % 256;   // G
            px[2] = rand() % 256;   // B
            px[3] = 255; // A
        }
    }
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);

    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

void print_help()
{
  printf("Usage:\n");
  printf("  -h | --help\t\tPrint help\n");
  printf("  -s | --size\t\tSet size for output file; Format - 1b 1k 1m or 1B 1K etc.\n");
  printf("  -v | --visual\t\tPrint info to terminal\n");
  printf("  -f | --filename\tSet name for output file\n");
  printf("  -t | --type\t\tSet file extention; jpeg or png \n");
  
}

int main(int argc, char* argv[])
{
  srand(time(NULL));
	const char* short_options = "hs:v::f:t:";

	const struct option long_options[] = {
	    { "help", no_argument, NULL, 'h' },
	    { "size", optional_argument, NULL, 's' },
	    { "visual", optional_argument, NULL, 'v'},
	    { "file", required_argument, NULL, 'f' },
	    { "type", required_argument, NULL, 't'},
	    { NULL, 0, NULL, 0 }
	};

	int rez;
	int option_index = -1;
  uintmax_t filesize = 0;
  char* filename = NULL;
  char* filetype = NULL;

  bool by_line = false;
  printf("By line: %i\n", by_line);

	while ((rez=getopt_long(argc,argv,short_options,
		long_options,&option_index))!=-1){

    char* arg = optarg;
    if (arg != NULL && arg[0] == '=') {
          arg++;
    }

		switch(rez){
			case 'h': {
        print_help();
				break;
			};

			case 's': {
				if (optarg!=NULL) { 
					printf("Found size with value %s\n", optarg);
          enum strtol_error err = xstrtoumax(optarg, NULL, 0, &filesize, "bkKmMgGtT");
          if (err != LONGINT_OK) {
            printf( "Wrong size suffix %s\n", optarg);
            exit(EXIT_FAILURE);
          } 
          if (filesize >= 314572800) {
            by_line = true; 
            printf("By line: %i\n", by_line);
          }
					printf("Size: %" PRIuMAX "\n", filesize);
				}
				else {
					printf("Found size without value\n");
          print_help();
          exit(EXIT_FAILURE);
				}
				break;
			};

      case 'v': {
				break;
      };
			
			case 'f': {
				if (optarg!=NULL){
          filename = optarg;
          printf("%s\n", filename);
				}
				else{
					printf("Found no filename!\n");
          print_help();
				 	exit(EXIT_FAILURE);
				}
				break;
			};

      case 't': {
				if (optarg!=NULL) {
          filetype = optarg;
          printf("%s\n", filetype);
				}
				else {
					printf("Found no filetype!\n");
          print_help();
          exit(EXIT_FAILURE);
				}
				break;
			};

			case '?': default: {
				printf("Found unknown option\n");
				print_help();
				break;
			};
	};
	option_index = -1;
};

  size_t required_size = strlen(filename) + strlen(filetype) + 2; // +2 for . and \0
  char* full_filename = malloc(required_size);

  if (full_filename == NULL) {
    perror("malloc");
    return EXIT_FAILURE;
  }

  snprintf(full_filename, required_size, "%s.%s", filename, filetype);

  if (!strcmp(filetype, "jpg"))
  {

    uint32_t jpeg_side = jpeg_side_estimate(filesize);
    printf("%u\n", jpeg_side);

    printf("Make jpeg\n");

    if (!by_line) {
      printf("By line: %i\n", by_line);
      write_jpeg_file(full_filename, jpeg_side, jpeg_side);
    }
    else {
      printf("By line: %i\n", by_line);
      write_jpeg_file_by_line(full_filename, jpeg_side, jpeg_side);
    }
  }

  else if (!strcmp(filetype, "png")) {
    uint32_t png_side = png_side_estimate(filesize);
    printf("%u\n", png_side);

    printf("Make png\n");
    write_png_file(full_filename, png_side, png_side);
  }

  else {
    printf("Unknown format!\n");
    print_help();
  }
  free(full_filename);
  return 0;
}
