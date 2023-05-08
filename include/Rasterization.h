#ifndef RASTERIZATION_H
#define RASTERIZATION_H

#include <Eigen/Eigen>
#include <vector>
#include <tuple>

#include "Model.h"
#include "global.h"
#include "Vertex.h"
#include "Shader.h"


namespace LRenderer {
extern class PipeLine;

class Rasterization {
 public:
  Rasterization(int w, int h);
  void clear();
  void drawTriangle(const Triangle& triangle);
  void drawLine(Eigen::Vector3f begin, Eigen::Vector3f end);
  void drawLineTriangle(const Triangle& triangle);
  void setPixel(const Eigen::Vector2i& point, const Eigen::Vector3f& color);
  std::vector<Eigen::Vector3f>& frameBuffer() { return frame_buffer; }
  void setShader(Shader& shader);
  std::unique_ptr<Shader>& shader();
  int getIndex(int x, int y);
  int getAAIndex(int x, int y);
  void openMSAA();
  void openSSAA();
  void openNormal();
   
  void reSize();
  Model model;
  int width, height;
  void set4xFreq();
  void set9xFreq();
  void set16xFreq();

  void changeBackgroudColor(const Eigen::Vector3f& bcolor);

  private:
  Frag constructFrag(int x, int y, int z, float alpha, float beta, float gamma,
                     const Triangle& t);

 private:
  Eigen::Vector3f backgroud_color;
  std::vector<Eigen::Vector3f> frame_buffer;  // rgb
  std::vector<Eigen::Vector3f> AA_frame_buffer;
  std::vector<float> depth_buffer;            // z-buffer
  std::vector<float> AA_depth_buffer;
  bool MSAA = false;
  bool SSAA = false;
  std::unique_ptr<Shader> m_shader;
  int freq;
  
};
}  // namespace LRenderer

#endif
