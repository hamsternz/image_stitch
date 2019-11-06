#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <memory.h>
#include <jpeglib.h>

#define PANEL_50  (5906)
#define PANEL_75  (8859)
#define PANEL_100 (PANEL_50*2)
#define PANEL_150 (PANEL_50*3)

#define IMAGE_WIDTH    (PANEL_150)
#define IMAGE_HEIGHT   (PANEL_100)

#define PREVIEW_SCALE  (16)
#define PREVIEW_WIDTH  (IMAGE_WIDTH/PREVIEW_SCALE)
#define PREVIEW_HEIGHT (IMAGE_HEIGHT/PREVIEW_SCALE)

static unsigned char panel_set_buffer[IMAGE_HEIGHT][IMAGE_WIDTH][3];
static unsigned int  preview_work[PREVIEW_HEIGHT][PREVIEW_WIDTH][3];
static unsigned char preview_buffer[PREVIEW_HEIGHT][PREVIEW_WIDTH][3];
/**************************************************************************/
struct layout_entry {
   int image;
   int origin_x, origin_y, limit_x,limit_y;
};

static struct layout_entry rapport_two[3] = {
   { 1, PANEL_75*0, 0, PANEL_75,  PANEL_100},
   { 2, PANEL_75*1, 1, PANEL_75,  PANEL_100},
   { 0, 0,0,0,0}
};

static struct layout_entry rapport_three[4] = {
   { 1, PANEL_50*0, PANEL_50*0, PANEL_50,  PANEL_50},
   { 2, PANEL_50*0, PANEL_50*1, PANEL_50,  PANEL_50},
   { 3, PANEL_50*1, PANEL_50*0, PANEL_100, PANEL_100},
   { 0,0,0,0}
};

static struct layout_entry rapport_six[7] = {
   { 1, PANEL_50*0, PANEL_50*0, PANEL_50, PANEL_50},
   { 2, PANEL_50*1, PANEL_50*0, PANEL_50, PANEL_50},
   { 3, PANEL_50*2, PANEL_50*0, PANEL_50, PANEL_50},
   { 4, PANEL_50*1, PANEL_50*1, PANEL_50, PANEL_50},
   { 5, PANEL_50*2, PANEL_50*1, PANEL_50, PANEL_50},
   { 6, PANEL_50*3, PANEL_50*1, PANEL_50, PANEL_50},
   { 0,0,0,0}
};


/**************************************************************************/
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

static int line = 0;
static int origin_x;
static int origin_y;
static int limit_x;
static int limit_y;


/**************************************************************************/
static void put_scanline_someplace(unsigned char *buffer, int stride) {
  if(line %20 == 0) {
     fprintf(stderr,"\rLine %i", line);
  }
  if(line < limit_y) {
     int copy_size = stride < limit_x*3 ? stride : limit_x*3;
     memcpy(panel_set_buffer[origin_y+line][origin_x],buffer, copy_size);
  }
  line++;
}

static void my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/*****************************************************************/
static void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
     panel_set_buffer[y][x][0] = r;
     panel_set_buffer[y][x][1] = g;
     panel_set_buffer[y][x][2] = b;
}
/*****************************************************************/
static void borders(unsigned char r, unsigned char g, unsigned char b) {
  int x,y;

  for(x = 0; x < limit_x; x++) {
     set_pixel(origin_x+x,         origin_y,           r,g,b);
     set_pixel(origin_x+x,         origin_y+limit_y-1, r,g,b);
  }

  for(y =0 ; y < limit_y; y++) {
     set_pixel(origin_x,           origin_y+y, r,g,b);
     set_pixel(origin_x+limit_x-1, origin_y+y, r,g,b);
  }
}
/*****************************************************************/
static int readfile(char *filename) {
  FILE * infile;		/* source file */
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }
  line = 0;
  fprintf(stderr, "Processing file %s\n", filename);

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);

  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);

  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)
  ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    put_scanline_someplace(buffer[0], row_stride);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
  printf("\n");
  return 1;
}

/********************************************************************************************/
static void generate_preview(void) {
   int x,y;
   for(y = 0; y < IMAGE_HEIGHT; y++) {
     for(x = 0; x < IMAGE_WIDTH; x++) {
       preview_work[y/PREVIEW_SCALE][x/PREVIEW_SCALE][0] += panel_set_buffer[y][x][0];
     }
   }

   for(y = 0; y < PREVIEW_HEIGHT; y++) {
     for(x = 0; x < PREVIEW_WIDTH; x++) {
	preview_buffer[y][x][0] = preview_work[y][x][0] / 256;
	preview_buffer[y][x][0] = preview_work[y][x][0] / 256;
	preview_buffer[y][x][0] = preview_work[y][x][0] / 256;
	preview_buffer[y][x][0] = preview_work[y][x][0] / 256;
     }
   }
}

/********************************************************************************************/
static int  write_preview_JPEG_file (char * filename, int quality)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width      = PREVIEW_WIDTH;
  cinfo.image_height     = PREVIEW_HEIGHT;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  cinfo.density_unit = 1;   /* Dots per inch */
  cinfo.X_density = 300;             /* Horizontal pixel density */
  cinfo.Y_density = 300;             /* Vertical pixel density */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&cinfo, TRUE);

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = preview_buffer[cinfo.next_scanline][0];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
  return 1;
}
/********************************************************************************************/
static int  write_JPEG_file (char * filename, int quality)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width  = IMAGE_WIDTH; 	/* image width and height, in pixels */
  cinfo.image_height = IMAGE_HEIGHT;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  cinfo.density_unit = 1;   /* Dots per inch */
  cinfo.X_density = 300;             /* Horizontal pixel density */
  cinfo.Y_density = 300;             /* Vertical pixel density */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&cinfo, TRUE);

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = panel_set_buffer[cinfo.next_scanline][0];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
  return 1;
}

/*****************************************************************************/
int main(int argc, char *argv[]) {
   int i;
   struct layout_entry *r = NULL;

   fprintf(stderr, "Emptying target image\n");
   memset(panel_set_buffer,255,sizeof(panel_set_buffer));

   switch(argc) {
      case 7: r = rapport_six;     break;
      case 3: r = rapport_two;     break;
      case 4: r = rapport_three;   break;
      default:
         fprintf(stderr, "Unknown set of file names\n");
         exit(3);
   }

   /* Process each element */
   for(i = 0; r[i].image < 1; i++) {
      limit_x  = r[i].limit_x;
      limit_y  = r[i].limit_y;
      origin_x = r[i].origin_x;
      origin_y = r[i].origin_y;
      if(r[i].image >= argc) {
         fprintf(stderr, "Not enough command line arguments\n");
         exit(3);
      }
      if(!readfile(argv[r[i].image])) {
         fprintf(stderr, "Exiting with error\n");
         exit(3);
      }
      borders(0,0,0); 
   }

   generate_preview();
   write_preview_JPEG_file("preview.jpg",200);
   write_JPEG_file("out.jpg",200);
   return 0;
}
