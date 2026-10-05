#define GNU_SOURCE
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>
#include <time.h>

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
  printf("  -s | --size\t\tSet size for output file; Format - 1Kb 1Gb 1Tb\n");
  printf("  -v | --visual\t\tPrint info to terminal\n");
  printf("  -f | --filename\tSet name for output file\n");
  printf("  -t | --type\t\tSet file extention; jpeg or png \n");
  
}

int main(int argc, char* argv[])
{
  srand(time(NULL));
	const char* short_options = "hsv::ft:";

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
  	char* filename;
  	char* filetype;
  	char* filesize;

	while ((rez=getopt_long(argc,argv,short_options,
		long_options,&option_index))!=-1){

		switch(rez){
			case 'h': {
        			print_help();
				break;
			};
			case 's': {
				if (optarg!=NULL) { 
					printf("Found size with value %s\n",optarg);
					filesize = optarg;
					printf("%s\n", filesize);
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
					  //strcpy(optarg, filename);
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

  int required_size = strlen(filename) + strlen(filetype) + 2; // +2 for . and \0
  if (required_size == NULL) {
    exit(EXIT_FAILURE);
  }
  char* full_filename = malloc(required_size);
  snprintf(full_filename, required_size, "%s.%s", filename, filetype);

  if (!strcmp(filetype, "jpg"))
  {
    printf("Make jpeg\n");
    write_jpeg_file(full_filename, 16384, 16384);
  }
  else if (!strcmp(filetype, "png")) {
    printf("Make png\n");
    write_png_file(full_filename, 4096, 4096);
  }
  else {
    printf("Unknown format!\n");
    print_help();
  }

  return 0;
}
