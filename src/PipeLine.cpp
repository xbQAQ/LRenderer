#include "PipeLine.h"

tbb::concurrent_vector<LRenderer::Triangle>
LRenderer::PipeLine::constructTriangle(const Object& object) {
  tbb::concurrent_vector<LRenderer::Triangle> triangleList;
  for (int i = 0; i != object.mesh.indexBuffer.size(); i += 3) {
    LRenderer::Triangle tri;
    for (int j = 0; j < 3; j++) {
      tri[j] = object.mesh.vertexBuffer[object.mesh.indexBuffer[i + j]];
      // TODO: 设置颜色
      tri[j].setColor(triangle_color);
    }
    triangleList.push_back(tri);
  }
  return triangleList;
}

// 罗德里格斯旋转公式

Eigen::Matrix4f get_model_matrix(const Eigen::Vector3f& axis, float angle,
                                 const Eigen::Vector3f& scale,
                                 const Eigen::Vector3f& translate) {
  Eigen::Matrix4f rotationMatrix = Eigen::Matrix4f::Identity();
  angle = angle * MY_PI / 180;
  float nx = axis.x(), ny = axis.y(), nz = axis.z();
  Eigen::Matrix4f N;
  Eigen::Vector4f Axis;
  // 转换为齐次坐标
  Axis << nx, ny, nz, 0;
  N << 0, -nz, ny, 0, nz, 0, -nx, 0, -ny, nx, 0, 0, 0, 0, 0, 1;

  rotationMatrix << std::cos(angle) * Eigen::Matrix4f::Identity() +
                        (1 - std::cos(angle)) * Axis * Axis.transpose() +
                        std::sin(angle) * N;
  rotationMatrix(3, 3) = 1;
  Eigen::Matrix4f scaleMatrix = Eigen::Matrix4f::Identity();
  scaleMatrix << scale.x(), 0, 0, 0, 0, scale.y(), 0, 0, 0, 0, scale.z(), 0, 0,
      0, 0, 1;

  Eigen::Matrix4f transMatrix = Eigen::Matrix4f::Identity();
  transMatrix << 1, 0, 0, translate.x(), 0, 1, 0, translate.y(), 0, 0, 1,
      translate.z(), 0, 0, 0, 1;
  /*std::cout << transMatrix * rotationMatrix * scaleMatrix << std::endl;*/

  return transMatrix * rotationMatrix * scaleMatrix;
}

// A坐标轴在B坐标轴下的表示，从A到B的变换为
// [Xa Ya Za 0]
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos, Eigen::Vector3f right,
                                Eigen::Vector3f up, Eigen::Vector3f front) {
  Eigen::Matrix4f view;
  view << right.x(), up.x(), front.x(), 0, right.y(), up.y(), front.y(), 0,
      right.z(), up.z(), front.z(), 0, 0, 0, 0, 1;

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1, -eye_pos[2],
      0, 0, 0, 1;

  view = translate * view;

  return view;
}

// zNear > 0, zFar > 0
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar) {
  Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

   float n = -zNear, f = -zFar;
   Eigen::Matrix4f Mperspect = Eigen::Matrix4f::Identity();
   float A = n + f, B = -n * f;
   Mperspect << n, 0, 0, 0, 0, n, 0, 0, 0, 0, A, B, 0, 0, 1, 0;

   float tanAngle = tan(eye_fov / 2 * MY_PI / 180);
  // 正交投影矩阵，因为在观测投影时x0y平面视角默认是中心，所以上下左右相等
   float t = std::abs(n) * tanAngle;
   float b = -t;
   float r = t * aspect_ratio;
   float l = -r;
   Eigen::Matrix4f MorthoTran = Eigen::Matrix4f::Identity();
   Eigen::Matrix4f MorthoScale = Eigen::Matrix4f::Identity();

  // 压缩成[-1, 1]
   MorthoScale << 2 / (r - l), 0, 0, 0, 0, 2 / (t - b), 0, 0, 0, 0, 2 / (n - f),
       0, 0, 0, 0, 1;
  // 移动到原点
   MorthoTran << 1, 0, 0, -(r + l) / 2, 0, 1, 0, -(t + b) / 2, 0, 0, 1,
      -(n + f) / 2, 0, 0, 0, 1;

   Eigen::Matrix4f Morth = MorthoScale * MorthoTran;
   //Eigen::Matrix4f Morth;
   //Morth << 2 / abs(r - l), 0, 0, -(r + l) / abs(r - l),
   //         0, 2 / abs(t - b), 0, -(t + b) / abs(t - b),
   //         0, 0, 2 / abs(n - f), -(n + f) / abs(n - f), 
   //         0, 0, 0, 1;

   //Eigen::Matrix4f Mreverse;
   //Mreverse << -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;

   projection =  Morth * Mperspect;

  return projection;
}

Eigen::Vector4f LRenderer::PipeLine::viewport(const Eigen::Vector4f& point) {
  Eigen::Matrix4f vp;
  int w = raster->width, h = raster->height;
  vp << w / 2.0, 0, 0, w / 2.0, 0, h / 2.0, 0, h / 2.0, 0, 0, 1, 0, 0, 0, 0, 1;
  return vp * point;
}

Eigen::Matrix4f get_normal_matrix(Eigen::Matrix4f project) {
  return project.reverse().transpose();
}

Eigen::Vector4f LRenderer::PipeLine::homogeneous_division(
    const Eigen::Vector4f& point) {
  Eigen::Vector4f result;
  result = point / point.w();
  return result;
}

bool LRenderer::PipeLine::viewCull(const Eigen::Vector4f& v1,
                                   const Eigen::Vector4f& v2,
                                   const Eigen::Vector4f& v3) {
  if (isViewCull(v1 / v1.w()) && isViewCull(v2 / v2.w()) && isViewCull(v3 / v3.w())) {
    return true;
  }
  return false;
}

bool LRenderer::PipeLine::isViewCull(const Eigen::Vector4f& windows_position) {
  if ((windows_position.x() < -1 || windows_position.x() > 1) ||
      (windows_position.y() < -1 || windows_position.y() > 1) ||
      (windows_position.z() < -1 || windows_position.z() > 1)) {
    return true;
  }
  return false;
}

void LRenderer::PipeLine::Init() {
  triangle_color = Eigen::Vector3f(9, 216, 229);
  model.loadModel(obj_path);
  raster->openNormal();
  raster->clear();

  raster->shader() = std::make_unique<Blin_Phong_Shader>();
  InitShader();

  raster->openSSAA();
  raster->set4xFreq();
  raster->reSize();

  for (auto& object : model.objects) {
    triangleList = constructTriangle(object);
    if (object.material.ka == Eigen::Vector3f(0, 0, 0) &&
        object.material.kd == Eigen::Vector3f(0, 0, 0) &&
        object.material.ks == Eigen::Vector3f(0, 0, 0)) {
      raster->shader()->setMaterialParm();
    } else {
      raster->shader()->setMaterialParm(object.material.ka, object.material.kd,
                                        object.material.ks);
    }
  }
}

void LRenderer::PipeLine::InitShader() {
  raster->shader()->setModelMatrix(get_model_matrix(
      camera.axis, camera.rotate_angle, camera.scale, camera.translate));
  raster->shader()->setViewMatrix(
      get_view_matrix(camera.eye_pos, camera.right, camera.up, camera.front));
  raster->shader()->setProjectMatrix(get_projection_matrix(
      camera.fov, camera.aspect_radio, camera.z_near, camera.z_far));
  raster->shader()->loadTexture(texture_path);
  raster->shader()->addLight(Eigen::Vector3f(20, 20, 20),
                             Eigen::Vector3f(500, 500, 500));
  raster->shader()->addLight(Eigen::Vector3f(-20, 20, 0),
                             Eigen::Vector3f(500, 500, 500));
  raster->shader()->setEyePosition(camera.eye_pos);
}

void LRenderer::PipeLine::reloadObj() {
  model.loadModel(obj_path);
  raster->openSSAA();
  raster->clear();

  for (auto& object : model.objects) {
    triangleList = constructTriangle(object);
    if (object.material.ka == Eigen::Vector3f(0, 0, 0) &&
        object.material.kd == Eigen::Vector3f(0, 0, 0) &&
        object.material.ks == Eigen::Vector3f(0, 0, 0)) {
      raster->shader()->setMaterialParm();
    } else {
      raster->shader()->setMaterialParm(object.material.ka, object.material.kd,
                                        object.material.ks);
    }
  }
}

void LRenderer::PipeLine::draw() {
  if (this->multi_thread) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, triangleList.size(), 2),
                      [&](tbb::blocked_range<size_t> r) {
                        for (size_t i = r.begin(); i < r.end(); i++) {
                          drawTriangle(triangleList[i]);
                        }
                      });
  } else {
    for (int i = 0; i != triangleList.size(); i++) {
      drawTriangle(triangleList[i]);
    }
  }
}

void LRenderer::PipeLine::drawTriangle(const Triangle& tri) {
  Triangle newTriangle;
  for (int j = 0; j != 3; j++) {
    // 计算mvp之后的位置
    newTriangle[j] = raster->shader()->vertexShader(tri[j]);
  }

    //视锥剔除
  if (viewCull(newTriangle[0].windows_position, newTriangle[1].windows_position,
               newTriangle[2].windows_position)) {
    return;
  }

  for (int j = 0; j != 3; j++) {
    newTriangle[j].windows_position =
        homogeneous_division(newTriangle[j].windows_position);
  }



  if (back_clip) {
    if (!backCull(newTriangle[0].windows_position,
                  newTriangle[1].windows_position,
                  newTriangle[2].windows_position)) {
      return;
    }
  }

  for (int j = 0; j != 3; j++) {
    newTriangle[j].windows_position = viewport(newTriangle[j].windows_position);
  }
  if (line_mode) {
    raster->drawLineTriangle(newTriangle);
  } else {
    raster->drawTriangle(newTriangle);
  }
}

bool LRenderer::PipeLine::backCull(const Eigen::Vector4f& v1,
                                   const Eigen::Vector4f& v2,
                                   const Eigen::Vector4f& v3) {
  auto vector12 = v2 - v1;
  auto vector3f12 = Eigen::Vector3f(vector12.x(), vector12.y(), vector12.z());

  auto vector23 = v3 - v2;
  auto vector3f23 = Eigen::Vector3f(vector23.x(), vector23.y(), vector23.z());

  auto v = vector3f12.cross(vector3f23);

  auto view = Eigen::Vector3f(0, 0, -1);
  // 视线方向和近裁面大于90度
  return v.dot(view) <= 0;
}

LRenderer::Vertex LRenderer::PipeLine::perspectiveLerp(
    const Vertex& v1, const Vertex& v2, float alpha, const Eigen::Vector4f& v1c,
    const Eigen::Vector4f& v2c) {
  float correctAlpha =
      (alpha * v1c.w()) / ((1 - alpha) * v2c.w() + alpha * v1c.w());
  return lerp(v1, v2, correctAlpha);
}

LRenderer::Vertex LRenderer::PipeLine::lerp(const Vertex& v1, const Vertex& v2,
                                            float alpha) {
  Vertex result;
  result.windows_position =
      (1 - alpha) * v1.windows_position + alpha * v2.windows_position;
  result.normal = (1 - alpha) * v1.normal + alpha * v2.normal;
  result.color = (1 - alpha) * v1.color + alpha * v2.color;
  result.tex = (1 - alpha) * v1.tex + alpha * v2.tex;
  return result;
}

void LRenderer::PipeLine::setObjPath(const std::string& op) { obj_path = op; }

void LRenderer::PipeLine::changeObjPath(const std::string& op) {
  obj_path = op;
  reloadObj();
}

void LRenderer::PipeLine::setTexturePath(const std::string& tp) {
  texture_path = tp;
}

void LRenderer::PipeLine::changeTexturePath(const std::string& tp) {
  texture_path = tp;
  raster->shader()->loadTexture(texture_path);
}

void LRenderer::PipeLine::setNormalShader() {
  raster->shader() = std::make_unique<Shader>();
}

void LRenderer::PipeLine::setBlinPhongShader() {
  if (typeid(*raster->shader()) != typeid(Blin_Phong_Shader)) {
    raster->shader() = std::make_unique<Blin_Phong_Shader>();
    InitShader();
  }
}

void LRenderer::PipeLine::setTextureShader() {
  if (typeid(*raster->shader()) != typeid(Texture_Shader)) {
    raster->shader() = std::make_unique<Texture_Shader>();
    InitShader();
  }
}

void LRenderer::PipeLine::setTriangleList(
    const tbb::concurrent_vector<Triangle>& tl) {
  triangleList = tl;
}

void LRenderer::PipeLine::openMutltiThread() { multi_thread = true; }

void LRenderer::PipeLine::closeMultiThread() { multi_thread = false; }

void LRenderer::PipeLine::openBackClip() { back_clip = true; }

void LRenderer::PipeLine::closeBackClip() { back_clip = false; }

void LRenderer::PipeLine::openSSAA() { raster->openSSAA(); }

void LRenderer::PipeLine::openMSAA() { raster->openSSAA(); }

void LRenderer::PipeLine::openNormal() { raster->openNormal(); }

void LRenderer::PipeLine::openLineMode() { line_mode = true; }

void LRenderer::PipeLine::closeLineMode() { line_mode = false; }

void LRenderer::PipeLine::changeColor(Eigen::Vector3f&& color) {
  triangle_color = color * 255;
  for (auto& object : model.objects) {
    triangleList = constructTriangle(object);
  }
}

void LRenderer::PipeLine::refresh() {
  raster->clear();
  raster->shader()->setModelMatrix(get_model_matrix(
      camera.axis, camera.rotate_angle, camera.scale, camera.translate));
  raster->shader()->setViewMatrix(
      get_view_matrix(camera.eye_pos, camera.right, camera.up, camera.front));
  raster->shader()->setProjectMatrix(get_projection_matrix(
      camera.fov, camera.aspect_radio, camera.z_near, camera.z_far));
  raster->shader()->setEyePosition(camera.eye_pos);
}
