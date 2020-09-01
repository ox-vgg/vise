#include "detect_points.h"

#include <iostream>

float scale_mult = 3.0;

int main(int argc, char **argv) {
  std::cout << "running detect_points_test" << std::endl;
  return KM_detect_points::lib_main(argc, argv);
}