#ifndef PIPELINE_H
#define PIPELINE_H

#include <tbb/parallel_for.h>
#include <tbb/tbb.h>

#include <Eigen/Eigen>
#include <tuple>
#include <vector>

#include "Camera.h"
#include "Model.h"
#include "Rasterization.h"
#include "Texture.h"

namespace LRenderer {
class PipeLine {
 public:
  PipeLine() = default;
  PipeLine(int w, int h)
      : width(w),
        height(h),
        obj_path(".\\models\\spot\\spot_triangulated_good.obj"),
        texture_path(".\\models\\spot\\spot_texture.png"),
        line_mode(false),
        back_clip(false),
        multi_thread(false) {
    raster = std::make_unique<Rasterization>(width, height);
  }
  int width;
  int height;
  std::unique_ptr<Rasterization> raster;
  Model model;
  Camera camera;

  void Init();
  void InitShader();
  tbb::concurrent_vector<Triangle> constructTriangle(const Object& object);
  void draw();
  void drawTriangle(const Triangle& tri);
  void refresh();
  void reloadObj();

  void setObjPath(const std::string& op);
  void changeObjPath(const std::string& op);
  void setTexturePath(const std::string& tp);
  void changeTexturePath(const std::string& tp);
  void setNormalShader();
  void setBlinPhongShader();
  void setTextureShader();
  void setTriangleList(const tbb::concurrent_vector<Triangle>& tl);
  void openMutltiThread();
  void closeMultiThread();
  void openBackClip();
  void closeBackClip();
  void openSSAA();
  void openMSAA();
  void openNormal();
  void openLineMode();
  void closeLineMode();
  void changeColor(Eigen::Vector3f&& color);

 private:
  Eigen::Vector4f viewport(const Eigen::Vector4f& point);
  Eigen::Vector4f homogeneous_division(const Eigen::Vector4f& point);
  bool viewCull(const Eigen::Vector4f& v1, const Eigen::Vector4f& v2,
                const Eigen::Vector4f& v3);
  bool isViewCull(const Eigen::Vector4f& windows_position);
  bool backCull(const Eigen::Vector4f& v1, const Eigen::Vector4f& v2,
                const Eigen::Vector4f& v3);

  Vertex perspectiveLerp(const Vertex& v1, const Vertex& v2, float alpha,
                         const Eigen::Vector4f& v1c,
                         const Eigen::Vector4f& v2c);
  Vertex lerp(const Vertex& v1, const Vertex& v2, float alpha);

 private:
  tbb::concurrent_vector<Triangle> triangleList;
  Eigen::Vector3f triangle_color;
  std::string obj_path;
  std::string texture_path;
  bool back_clip;
  bool line_mode;
  bool multi_thread;
};
}  // namespace LRenderer

#endif