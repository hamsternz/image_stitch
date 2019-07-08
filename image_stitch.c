#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <memory.h>
#include <jpeglib.h>

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

int origin_x;
int origin_y;
static int line = 0;


#define PANEL_X (5906)
#define PANEL_Y (5906)

unsigned char panel_set_buffer[PANEL_Y*2][PANEL_X*3][3];

void put_scanline_someplace(unsigned char *buffer, int stride) {
  fprintf(stderr,"\rLine %i", line);
  if(line < PANEL_Y) {
     memcpy(panel_set_buffer[origin_y+line][origin_x],buffer, (stride < PANEL_X*3 ? stride : PANEL_X*3));
  }
  line++;
}

void my_error_exit (j_common_ptr cinfo)
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
void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
     panel_set_buffer[y][x][0] = r;
     panel_set_buffer[y][x][1] = g;
     panel_set_buffer[y][x][2] = b;
}
/*****************************************************************/
void borders(void) {
  int x,y;

  for(x = 0; x < PANEL_X*3; x++) {
     set_pixel(x,PANEL_Y*0+0,  0,0,0);
     set_pixel(x,PANEL_Y*1-1,  0,0,0);

     set_pixel(x,PANEL_Y*1+0,  0,0,0);
     set_pixel(x,PANEL_Y*2-1,  0,0,0);
  }

  for(y = 0; y < PANEL_X*2; y++) {
     set_pixel(PANEL_X*0+0, y, 0,0,0);
     set_pixel(PANEL_X*1-1, y, 0,0,0);

     set_pixel(PANEL_X*1+0, y, 0,0,0);
     set_pixel(PANEL_X*2-1, y, 0,0,0);

     set_pixel(PANEL_X*2+0, y, 0,0,0);
     set_pixel(PANEL_X*3-1, y, 0,0,0);
  }

}
/*****************************************************************/
int readfile(char *filename) {
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
int  write_JPEG_file (char * filename, int quality)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = PANEL_X*3; 	/* image width and height, in pixels */
  cinfo.image_height = PANEL_Y*2;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  cinfo.density_unit = 1;   /* Dots per inch */
  cinfo.X_density = 300;             /* Horizontal pixel density */
  cinfo.Y_density = 300;             /* Vertical pixel density */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&cinfo, TRUE);
  row_stride = PANEL_X * 3;	/* JSAMPLEs per row in image_buffer */

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
   if(argc != 7) {
      fprintf(stderr, "Please supply all six file names\n");
      exit(0);
   } 
   fprintf(stderr, "Emptying target image\n");
   memset(panel_set_buffer,255,sizeof(panel_set_buffer));
   origin_x = PANEL_X*0; origin_y = PANEL_Y*0; if(!readfile(argv[1])) { fprintf(stderr, "Exiting with error\n"); }
   origin_x = PANEL_X*1; origin_y = PANEL_Y*0; if(!readfile(argv[2])) { fprintf(stderr, "Exiting with error\n"); }
   origin_x = PANEL_X*2; origin_y = PANEL_Y*0; if(!readfile(argv[3])) { fprintf(stderr, "Exiting with error\n"); }
   origin_x = PANEL_X*0; origin_y = PANEL_Y*1; if(!readfile(argv[4])) { fprintf(stderr, "Exiting with error\n"); }
   origin_x = PANEL_X*1; origin_y = PANEL_Y*1; if(!readfile(argv[5])) { fprintf(stderr, "Exiting with error\n"); }
   origin_x = PANEL_X*2; origin_y = PANEL_Y*1; if(!readfile(argv[6])) { fprintf(stderr, "Exiting with error\n"); }
   borders(); 
   write_JPEG_file("out.jpg",200);
   return 0;
}

