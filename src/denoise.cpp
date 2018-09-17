#include "denoise.h"
#include <future>
#include <algorithm>
#include <iostream>

#include <Eigen/SVD>

Eigen::MatrixXd Compute_Channel_Denoise(Eigen::MatrixXd a, int perc)
{
  // Compute SVD and store in svd
  Eigen::BDCSVD<Eigen::MatrixXd> svd(a, Eigen::ComputeThinU | Eigen::ComputeThinV);

  // Grab singular values
  auto singular = svd.singularValues();

  // Zero out perc% of the smallest singualar values
  int size = singular.size();
  for (int i = size * (1 - 1/perc); i < size; i++)
    singular[i] = 0.0;


  // recombine the decomposition
  return svd.matrixU() * singular.asDiagonal() * svd.matrixV().transpose();
}

int Denoise(pngreq& img, int perc)
{
  // Grab some info on images
  png_byte color_type = img.color_type;
  int w = img.width, h = img.height;
  // Check color_type
  if (color_type == PNG_COLOR_TYPE_RGB)
  {
    // Allocate Arrays that will store RGB data
    Eigen::MatrixXd R(w,h);
    Eigen::MatrixXd G(w,h);
    Eigen::MatrixXd B(w,h);

    // Get Image Values
    img.get_rgb_channels(R, G, B);

    // Pass each channel through Compute_Channel_Denoise
    std::future<Eigen::MatrixXd> newR_fut = std::async(Compute_Channel_Denoise, R, perc);
    std::future<Eigen::MatrixXd> newG_fut = std::async(Compute_Channel_Denoise, G, perc);
    Eigen::MatrixXd newB = Compute_Channel_Denoise(B, perc);

    Eigen::MatrixXd newR = newR_fut.get();
    Eigen::MatrixXd newG = newG_fut.get();

    // Set new Image Values
    img.set_rgb_channels(newR, newG, newB);
  }
  else if (color_type == PNG_COLOR_TYPE_RGBA)
  {
    // Allocate Arrays that will store RGBA data
    Eigen::MatrixXd R(w,h);
    Eigen::MatrixXd G(w,h);
    Eigen::MatrixXd B(w,h);
    Eigen::MatrixXd A(w,h);

    // Get Image Values
    img.get_rgba_channels(R, G, B, A);

    // Pass each channel through Compute_Channel_Denoise
    std::future<Eigen::MatrixXd> newR_fut = std::async(Compute_Channel_Denoise, R, perc);
    std::future<Eigen::MatrixXd> newG_fut = std::async(Compute_Channel_Denoise, G, perc);
    std::future<Eigen::MatrixXd> newB_fut = std::async(Compute_Channel_Denoise, B, perc);
    Eigen::MatrixXd newA = Compute_Channel_Denoise(A, perc);

    Eigen::MatrixXd newR = newR_fut.get();
    Eigen::MatrixXd newG = newG_fut.get();
    Eigen::MatrixXd newB = newB_fut.get();

    // Set new Image Values
    img.set_rgba_channels(newR, newG, newB, newA);
  }
  else if (color_type == PNG_COLOR_TYPE_GRAY)
  {
    // Allocate Arrays that will store Gray Scale Data
    Eigen::MatrixXd G(w,h);

    // Get Image Values
    img.get_gray_channel(G);

    // Pass the channel through Compute_Channel_Denoise
    Eigen::MatrixXd newG = Compute_Channel_Denoise(G, perc);

    // Set new Image Values
    img.set_gray_channel(G);
  }
  else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    // Allocate Arrays that will store Gray and Alpha Data
    Eigen::MatrixXd G(w,h);
    Eigen::MatrixXd A(w,h);

    // Get Image Values
    img.get_graya_channels(G, A);

    // Pass the channels through Compute_Channel_Denoise
    std::future<Eigen::MatrixXd> newG_fut = std::async(Compute_Channel_Denoise, G, perc);
    Eigen::MatrixXd newA = Compute_Channel_Denoise(A, perc);

    Eigen::MatrixXd newG = newG_fut.get();

    // Set new Image Values
    img.set_graya_channels(newG, newA);
  }
  else
  {
    std::cout << "[Denoise] image color_type not supported" << std::endl;
    return -1;
  }
  return 0;
}

