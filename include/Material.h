#ifndef MATERIAL_H
#define MATERIAL_H

#include <Eigen/Eigen>
#include <string>

#include "Texture.h"
#include "Shader.h"

namespace LRenderer {
class Material {
 public:
  Eigen::Vector3f ka;  // ��������
  Eigen::Vector3f kd;  // ���������
  Eigen::Vector3f ks;  // �߹����

  Material() = default;
  ~Material() = default;
};
}  // namespace LRenderer

#endif