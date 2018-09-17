#ifndef DENOISE_H
#define DENOISE_H

#include "pngreq.h"
#include <Eigen/Core>

Eigen::MatrixXd Compute_Channel_Denoise(Eigen::MatrixXd a, int perc);
int Denoise(pngreq& img, int perc);

#endif
