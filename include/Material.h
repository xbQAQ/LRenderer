#ifndef MATERIAL_H
#define MATERIAL_H

#include <Eigen/Eigen>
#include <string>

#include "Texture.h"
#include "Shader.h"

namespace LRenderer {
class Material {
 public:
  Eigen::Vector3f ka;  // 环境光照
  Eigen::Vector3f kd;  // 漫反射光照
  Eigen::Vector3f ks;  // 高光光照

  Material() = default;
  ~Material() = default;
};
}  // namespace LRenderer

#endif