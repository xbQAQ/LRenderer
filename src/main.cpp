#include <chrono>
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "UI.h"

int main() {
  LRenderer::UI ui(1280, 720);
  ui.Init();
  ui.show();

  return 0;
}