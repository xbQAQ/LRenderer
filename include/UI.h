#ifndef UI_H
#define UI_H

#define NOMINMAX
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#include <io.h>
#include <direct.h>

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

#include "PipeLine.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imstb_truetype.h"

namespace LRenderer {
class UI {
 public:
  UI(int w, int h);

  void Init();
  void show();
  ~UI();

 private:
  void Mat2Texture(const cv::Mat& image, GLuint& imageTexture);
  void inputProcess();
  void showMenuBar();
  void showAbout();

 private:
  int width, height;
  int picture_width, picture_height;
  bool showWindow;

  enum ShaderMode {
    BLINPHONGSHADER, TEXTURESHADER
  };
  enum AAMode {
      SSAA, MSAA, NORMAL
  };
  enum FreqMode { _4X, _9X, _16X, FREQSIZE };

  GLFWwindow* windows;
  bool multiThread = true;
  bool back_clip = false;
  bool save = false;
    
  bool use_texture_shader;
  bool use_blin_phong_shader;
  std::unique_ptr<PipeLine> pipeline;
  cv::Mat image;
  GLuint image_texture;
  ImVec4 model_color;
  bool about_open;
};
}  // namespace LRenderer

#endif
