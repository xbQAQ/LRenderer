#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>

namespace LRenderer {
class Texture {
 private:
  cv::Mat image_data;

 public:
  Texture() = default;
  Texture(const std::string& path);

  bool isEmpty();

  int width, height;

  void loadTexture(const std::string& path);

  const Eigen::Vector3f& getColor(float u, float v);

  // Ë«ÏßÐÔ²åÖµ
  Eigen::Vector3f getColorBilinear(float u, float v);
};
}  // namespace LRenderer

#endif  // !TEXTURE_H
