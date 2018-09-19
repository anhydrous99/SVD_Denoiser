#include "pngreq.h"

#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void abort_(const char * s, ...)
{
  va_list args;
  va_start(args, s);
  vfprintf(stderr, s, args);
  fprintf(stderr, "\n");
  va_end(args);
  abort();
}

png_byte to_png_byte(double input)
{
	png_byte mm = std::numeric_limits<png_byte>::max();
	png_byte ll = std::numeric_limits<png_byte>::lowest();
	if (input > mm)
		return mm;
	else if (input < ll)
		return ll;
	else
		return static_cast<png_byte>(input);
}

void pngreq::read_png_file(char* file_name)
{
  char header[8]; // 8 is the maximum size that can be checked

  /* open file and test for it being a png */
  FILE *fp = fopen(file_name, "rb");
  if (!fp)
    abort_("[read_png_file] File %s could not be opened for reading\n", file_name);

  size_t result = fread(header, 1, 8, fp);
  if (result != 8) 
    abort_("[read_png_file] Reading error!");

  if (png_sig_cmp((png_const_bytep)header, 0, 8))
    abort_("[read_png_file] File %s is not recognized as a PNG file\n", file_name);

  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    abort_("[read_png_file] png_create_read_struct failed\n");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    abort_("[read_png_file] png_create_info_struct failed\n");

  if(setjmp(png_jmpbuf(png_ptr)))
    abort_("[read_png_file] Error during init_io\n");

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);

  /* read file */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[read_png_file] Error during read_image\n");

  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (y = 0; y < height; y++)
    row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));

  png_read_image(png_ptr, row_pointers);

  fclose(fp);
}
void pngreq::write_png_file(char* file_name)
{
  /* create file */
  FILE *fp = fopen(file_name, "wb");
  if (!fp)
    abort_("[write_png_file] File %s could not be opened for writing\n", file_name);

  /* initialize stuff */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    abort_("[write_png_file] png_create_write_struct failed\n");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    abort_("[write_png_file] png_create_info_struct failed\n");

  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during init_io\n");

  png_init_io(png_ptr, fp);

  /* write deader */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during writing header\n");

  png_set_IHDR(png_ptr, info_ptr, width, height,
	       bit_depth, color_type, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  /* write bytes */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during writing bytes\n");

  png_write_image(png_ptr, row_pointers);

  /* end write */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during end of write\n");

  png_write_end(png_ptr, NULL);

  /* clean heap allocation */
  for (y = 0; y < height; y++)
    free(row_pointers[y]);
  free(row_pointers);
  fclose(fp);
}

void pngreq::get_rgb_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G, Eigen::MatrixXd& B)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA)
    abort_("[get_rgb_channels] input file is PNG_COLOR_TYPE_RGBA but must be PNG_COLOR_TYPE_RGB"
		    "(has the alpha channel)");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGB)
    abort_("[get_rgb_channels] color_type of input file must be PNG_COLOR_TYPE_RGB (%d) (is %d)",
		    PNG_COLOR_TYPE_RGB, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x=0; x<width; x++)
    {
      png_byte* ptr = &(row[x*3]);
      R(x,y) = static_cast<double>(ptr[0]);
      G(x,y) = static_cast<double>(ptr[1]);
      B(x,y) = static_cast<double>(ptr[2]);
    }
  }
}
void pngreq::get_rgba_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G, 
                               Eigen::MatrixXd& B, Eigen::MatrixXd& A)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
    abort_("[get_rgba_channels] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA"
		    "(lacks the alpha channel)");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
    abort_("[get_rgba_channels] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
		    PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x=0; x<width; x++)
    {
      png_byte* ptr = &(row[x*4]);
      R(x,y) = static_cast<double>(ptr[0]);
      G(x,y) = static_cast<double>(ptr[1]);
      B(x,y) = static_cast<double>(ptr[2]);
      A(x,y) = static_cast<double>(ptr[3]);
    }
  }
}
void pngreq::get_gray_channel(Eigen::MatrixXd& G)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
    abort_("[get_gray_channel] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_GRAY");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_GRAY)
    abort_("[get_gray_channel] color_type of input file must be PNG_COLOR_TYPE_GRAY (%d) (is %d)",
		    PNG_COLOR_TYPE_GRAY, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x=0; x<width; x++)
    {
      png_byte* ptr = &(row[x]);
      G(x,y) = static_cast<double>(ptr[0]);
    }
  }
}
void pngreq::get_graya_channels(Eigen::MatrixXd& G, Eigen::MatrixXd& A)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY)
    abort_("[get_graya_channel] input file is PNG_COLOR_TYPE_GRAY but must be "
		    "PNG_COLOR_TYPE_GRAY_ALPHA");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_GRAY_ALPHA)
    abort_("[get_graya_channel] color_type of input file must be PNG_COLOR_TYPE_GRAY_ALPHA (%d) (is %d)",
		    PNG_COLOR_TYPE_GRAY_ALPHA, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x=0; x < width; x++)
    {
      png_byte* ptr = &(row[x*2]);
      G(x,y) = static_cast<double>(ptr[0]);
      A(x,y) = static_cast<double>(ptr[1]);
    }
  }
}
// SET STUFF -------------------------------------------------------
void pngreq::set_rgb_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G, Eigen::MatrixXd& B)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA)
    abort_("[set_rgb_channels] input file is PNG_COLOR_TYPE_RGBA but must be "
		    "PNG_COLOR_TYPE_RGB");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGB)
    abort_("[set_rgb_channels] color_type of input file must be PNG_COLOR_TYPE_RGB (%d) (is %d)",
		    PNG_COLOR_TYPE_RGB, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x = 0; x < width; x++)
    {
      png_byte* ptr = &(row[x*3]);
      ptr[0] = to_png_byte(R(x,y));
      ptr[1] = to_png_byte(G(x,y));
      ptr[2] = to_png_byte(B(x,y));
    }
  }
}
void pngreq::set_rgba_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G,
                               Eigen::MatrixXd& B, Eigen::MatrixXd& A)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
    abort_("[set_rgba_channels] input file is PNG_COLOR_TYPE_RGB but must be "
		    "PNG_COLOR_TYPE_RGBA");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
    abort_("[set_rgba_channels] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
		    PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x = 0; x < width; x++)
    {
      png_byte* ptr = &(row[x*4]);
      ptr[0] = to_png_byte(R(x,y));
      ptr[1] = to_png_byte(G(x,y));
      ptr[2] = to_png_byte(B(x,y));
      ptr[3] = to_png_byte(A(x,y));
    }
  }
}
void pngreq::set_gray_channel(Eigen::MatrixXd& G)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
    abort_("[set_gray_channel] input file is PNG_COLOR_TYPE_RGB but must be "
		    "PNG_COLOR_TYPE_GRAY");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_GRAY)
    abort_("[set_gray_channel] color_type of input file must be PNG_COLOR_TYPE_GRAY (%d) (is %d)",
		    PNG_COLOR_TYPE_GRAY, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x = 0; x < width; x++)
    {
      png_byte* ptr = &(row[x]);
      ptr[0] = to_png_byte(G(x,y));
    }
  }
}
void pngreq::set_graya_channels(Eigen::MatrixXd& G, Eigen::MatrixXd& A)
{
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY)
    abort_("[set_graya_channel] input file is PNG_COLOR_TYPE_GRAY but must be " 
		    "PNG_COLOR_TYPE_GRAY_ALPHA");

  if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_GRAY_ALPHA)
    abort_("[set_graya_channel] color_type of input file must be PNG_COLOR_TYPE_GRAY_ALPHA (%d) (is %d)",
		    PNG_COLOR_TYPE_GRAY_ALPHA, png_get_color_type(png_ptr, info_ptr));

  for (y = 0; y < height; y++)
  {
    png_byte* row = row_pointers[y];
    for (x = 0; x < width; y++)
    {
      png_byte* ptr = &(row[x*2]);
      ptr[0] = to_png_byte(G(x,y));
      ptr[1] = to_png_byte(A(x,y));
    }
  }
}
