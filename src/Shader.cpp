#include "Shader.h"

void LRenderer::Shader::setModelMatrix(const Eigen::Matrix4f& m) {
  this->model = m;
}
void LRenderer::Shader::setViewMatrix(const Eigen::Matrix4f& v) {
  this->view = v;
}
void LRenderer::Shader::setProjectMatrix(const Eigen::Matrix4f& p) {
  this->project = p;
}

void LRenderer::Shader::setMaterialParm(const Eigen::Vector3f& _ka,
                                        const Eigen::Vector3f& _kd,
                                        const Eigen::Vector3f& _ks) {
  ka = _ka;
  kd = _kd;
  ks = _ks;
}

void LRenderer::Shader::setEyePosition(const Eigen::Vector3f& _eye_pos) {
  eye_pos = _eye_pos;
}

void LRenderer::Shader::addLight(Eigen::Vector3f position,
                                 Eigen::Vector3f intensity) {
  lights.push_back({position, intensity});
}

void LRenderer::Shader::addLight(const Light& light) {
  lights.push_back(light);
}

void LRenderer::Shader::loadTexture(std::string path) {
  texture.loadTexture(path);
}

const Eigen::Matrix4f& LRenderer::Shader::getModelMatrix() { return model; }

const Eigen::Matrix4f& LRenderer::Shader::getViewMatrix() { return view; }

const Eigen::Matrix4f& LRenderer::Shader::getProjectMatrix() { return project; }

LRenderer::Vertex LRenderer::Shader::vertexShader(const Vertex& v) {
  Vertex result;
  const Eigen::Matrix3f mvInverseTranspose =
      ((view * model).transpose().inverse()).block<3, 3>(0, 0);
  result.world_position = (view * model * v.position).head<3>();
  result.windows_position = project * view * model * v.position;
  result.normal = mvInverseTranspose * v.normal;
  result.color = v.color;
  result.tex = v.tex;
  return result;
}

void LRenderer::Shader::fragShader(Frag& f) { return; }

void LRenderer::Blin_Phong_Shader::fragShader(Frag& f) {
  kd = f.color;  // 漫反射系数

  const Eigen::Vector3f amb_light_intensity{10, 10, 10};  // 环境光强度,同时也是rgb值

  float p = 150;

  const Eigen::Vector3f point = f.view_position;
  const Eigen::Vector3f normal = f.normal;

  Eigen::Vector3f result_color = {0, 0, 0};
  auto computeLight = [=](const Light& light) -> Eigen::Vector3f {
    const float r = (light.position - point).norm();
    const Eigen::Vector3f l = (light.position - point).normalized();
    const Eigen::Vector3f v = (eye_pos - point).normalized();
    const Eigen::Vector3f h = (l + v).normalized();

    // cwiseProduct 矩阵/向量对应位置相乘
    //  环境光照
    Eigen::Vector3f La = ka.cwiseProduct(amb_light_intensity);

    auto cosThetaDiffuse = l.dot(normal);
    // 漫反射光照
    Eigen::Vector3f Ld = kd.cwiseProduct(
        light.intensity / std::pow(r, 2) * std::max((float)0, cosThetaDiffuse));

    auto cosThetaSpecular = h.dot(normal);
    // 镜面光照
    Eigen::Vector3f Ls =
        ks.cwiseProduct(light.intensity / std::pow(r, 2) *
                        std::pow(std::max((float)0, cosThetaSpecular), p));
    return La + Ld + Ls;
  };

  for (auto& light : lights) {
    result_color += computeLight(light);
  }
  f.color = result_color;
  return;
}

void LRenderer::Texture_Shader::fragShader(Frag& f) {
  Eigen::Vector3f texture_color = {0, 0, 0};
  if (!texture.isEmpty()) {
    auto u = f.tex_position.x(), v = f.tex_position.y();
    texture_color = texture.getColor(u, v);
  }

  Eigen::Vector3f kd = texture_color / 255.f;  //将纹理颜色作为漫反射系数

  const Eigen::Vector3f amb_light_intensity{10, 10, 10};  //环境光强度

  constexpr float p = 150;

  const Eigen::Vector3f point = f.view_position;
  const Eigen::Vector3f normal = f.normal;

  Eigen::Vector3f result_color = {0, 0, 0};

  auto computeLight = [=](const Light& light) -> Eigen::Vector3f {
    const float r = (light.position - point).norm();
    const Eigen::Vector3f l = (light.position - point).normalized();
    const Eigen::Vector3f v = (eye_pos - point).normalized();
    const Eigen::Vector3f h = (l + v).normalized();

    // cwiseProduct 矩阵/向量对应位置相乘
    //  环境光照
    const Eigen::Vector3f La = ka.cwiseProduct(amb_light_intensity);

    auto cosThetaDiffuse = l.dot(normal);
    // 漫反射光照
    const Eigen::Vector3f Ld = kd.cwiseProduct(
        light.intensity / std::pow(r, 2) * std::max((float)0, cosThetaDiffuse));

    auto cosThetaSpecular = h.dot(normal);
    // 镜面光照
    const Eigen::Vector3f Ls =
        ks.cwiseProduct(light.intensity / std::pow(r, 2) *
                        std::pow(std::max((float)0, cosThetaSpecular), p));
    
    return La + Ld + Ls;
  };

  for (auto& light : lights) {
    result_color += computeLight(light);
  }
  f.color = result_color;
  return;
}
