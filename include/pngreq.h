#ifndef PNGREQ_H
#define PNGREQ_H

#include <Eigen/Core>

#include <png.h>

class pngreq
{
public:
  int x, y;
  int width, height;
  png_byte color_type;
  png_byte bit_depth;

  png_structp png_ptr;
  png_infop info_ptr;
  int number_of_passes;
  png_bytep * row_pointers;

  void read_png_file(char* file_name);
  void write_png_file(char* file_name);

  void get_rgb_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G, Eigen::MatrixXd& B);
  void get_rgba_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G, 
                         Eigen::MatrixXd& B, Eigen::MatrixXd& A);
  void get_gray_channel(Eigen::MatrixXd& G);
  void get_graya_channels(Eigen::MatrixXd& G, Eigen::MatrixXd& A);

  void set_rgb_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G, Eigen::MatrixXd& B);
  void set_rgba_channels(Eigen::MatrixXd& R, Eigen::MatrixXd& G, 
                         Eigen::MatrixXd& B, Eigen::MatrixXd& A);
  void set_gray_channel(Eigen::MatrixXd& G);
  void set_graya_channels(Eigen::MatrixXd& G, Eigen::MatrixXd& A);
};


#endif // PNGREQ_H
