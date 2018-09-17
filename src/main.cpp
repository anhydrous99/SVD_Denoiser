#include "pngreq.h"
#include "denoise.h"
#include <iostream>

int main(int argc, char* argv[])
{
  pngreq img;
  // Read PNG Image
  img.read_png_file(argv[1]);
  // DeNoise Image
  Denoise(img, std::stoi(argv[3]));
  // Write Image
  img.write_png_file(argv[2]);
  return 0;
}
