#ifndef SHADER_H
#define SHADER_H

#include <tbb/spin_mutex.h>

#include <Eigen/Eigen>

#include "Fragment.h"
#include "Texture.h"
#include "Vertex.h"
#include "global.h"

namespace LRenderer {
struct Light {
  Eigen::Vector3f position;
  // Eigen::Vector3f Ambient;  // 环境光
  // Eigen::Vector3f Diffuse;  // 漫反射
  // Eigen::Vector3f Specular; // 镜面光
  Eigen::Vector3f intensity;  // 强度
};

class Shader {
 public:
  Shader(const Eigen::Vector3f& _ka = Eigen::Vector3f(0.005, 0.005, 0.005),
         const Eigen::Vector3f& _kd = {0, 0, 0},
         const Eigen::Vector3f& _ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937))
      : pre_model(Eigen::Matrix4f::Identity()),
        model(Eigen::Matrix4f::Identity()),
        view(Eigen::Matrix4f::Identity()),
        project(Eigen::Matrix4f::Identity()),
        viewport(Eigen::Matrix4f::Identity()),
        texture(),
        eye_pos(Eigen::Vector3f(0, 0, 10)),
        ka(_ka),
        kd(_kd),
        ks(_ks) {}
  Shader(const Eigen::Matrix4f& _model, const Eigen::Matrix4f& _view,
         const Eigen::Matrix4f& _project, const Eigen::Matrix4f& _viewport,
         const Eigen::Vector3f& _eye_pos,
         const Eigen::Vector3f& _ka = Eigen::Vector3f(0.005, 0.005, 0.005),
         const Eigen::Vector3f& _kd = {0, 0, 0},
         const Eigen::Vector3f& _ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937))
      : pre_model(Eigen::Matrix4f::Identity()),
        model(_model),
        view(_view),
        project(_project),
        viewport(_viewport),
        texture(),
        eye_pos(_eye_pos),
        ka(_ka),
        kd(_kd),
        ks(_ks) {}
  virtual ~Shader() = default;

  void setModelMatrix(const Eigen::Matrix4f& m);
  void setViewMatrix(const Eigen::Matrix4f& v);
  void setProjectMatrix(const Eigen::Matrix4f& p);

  void setMaterialParm(
      const Eigen::Vector3f& _ka = Eigen::Vector3f(0.005, 0.005, 0.005),
      const Eigen::Vector3f& _kd = {0, 0, 0},
      const Eigen::Vector3f& _ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937));
  void setEyePosition(const Eigen::Vector3f& _eye_pos = Eigen::Vector3f(0, 0,
                                                                        10));

  void addLight(Eigen::Vector3f position, Eigen::Vector3f intensity);
  void addLight(const Light& light);
  void loadTexture(std::string path);

  const Eigen::Matrix4f& getModelMatrix();
  const Eigen::Matrix4f& getViewMatrix();
  const Eigen::Matrix4f& getProjectMatrix();

  virtual Vertex vertexShader(const Vertex& v);
  virtual void fragShader(Frag& f);

 protected:
  Eigen::Matrix4f pre_model;
  Eigen::Matrix4f model;
  Eigen::Matrix4f view;
  Eigen::Matrix4f project;
  Eigen::Matrix4f viewport;

  std::vector<Light> lights;
  Texture texture;
  Eigen::Vector3f eye_pos;

  Eigen::Vector3f ka, kd, ks;
};

class Texture_Shader : public Shader {
 public:
  Texture_Shader() = default;
  ~Texture_Shader() = default;
  void fragShader(Frag& f) override;
};

class Blin_Phong_Shader : public Shader {
 public:
  Blin_Phong_Shader() = default;
  ~Blin_Phong_Shader() = default;
  void fragShader(Frag& f) override;
};

}  // namespace LRenderer

#endif
