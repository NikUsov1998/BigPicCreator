#define GNU_SOURCE
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <png.h>
#include <time.h>

void write_jpeg_file()
{
  int width = 640;
  int height = 480;
  unsigned char* buffer = malloc(width * height * 3);
  char* filename = "test.jpg";


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
    exit(1);
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
        8,                                 // bit depth (8 bits per channel)
        PNG_COLOR_TYPE_RGBA,               // color type (Red, Green, Blue, Alpha)
        PNG_INTERLACE_ADAM7,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    png_bytep *row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png, info));
        
        // Fill row pixels (Example: Solid Red with full opacity)
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
    { "filename", required_argument, NULL, 'f' },
    { "type", required_argument, NULL, 't'},
    { NULL, 0, NULL, 0 }
	};

	int rez;
	int option_index = -1;
	while ((rez=getopt_long(argc,argv,short_options,
		long_options,&option_index))!=-1){

		switch(rez){
			case 'h': {
        print_help();
				break;
			};

			case 's': {
				if (optarg!=NULL)
					printf("found size with value %s\n",optarg);
				else
					printf("found size without value\n");
				break;
			};

      case 'v': {
        break;
      };
	
			case 'f': {
				break;
			};

      case 't': {
        break;
      };

			case '?': default: {
				printf("found unknown option\n");
        print_help();
				break;
			};
		};
    option_index = -1;
	};
  
  write_png_file("output.png", 300, 200);
  write_jpeg_file();
  int height = 0;
  int width = 0;
  int size = 0;
  return 0;
}
