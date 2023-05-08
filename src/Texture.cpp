#include "Texture.h"

LRenderer::Texture::Texture(const std::string& path) {
  image_data = cv::imread(path);
  if (image_data.empty()) {
    std::cerr << "read image error!\n";
  }
  cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
  width = image_data.cols;
  height = image_data.rows;
}

bool LRenderer::Texture::isEmpty() { return image_data.empty(); }

void LRenderer::Texture::loadTexture(const std::string& path) {
  image_data = cv::imread(path);
  if (image_data.empty()) {
    std::cerr << "read image error!\n";
    exit(-1);
  }
  cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
  width = image_data.cols;
  height = image_data.rows;
}

const Eigen::Vector3f& LRenderer::Texture::getColor(float u, float v) {
  // repeat
  if (u < 0) u -= floor(u);
  if (u > 1) u -= floor(u);
  if (v < 0) v -= floor(v);
  if (v > 1) v -= floor(v);
  auto u_img = u * width - 0.5;
  auto v_img = (1 - v) * height - 0.5;
  auto color = image_data.at<cv::Vec3b>(v_img, u_img);
  return Eigen::Vector3f(color[0], color[1], color[2]);
}
