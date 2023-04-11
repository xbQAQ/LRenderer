#include "Rasterization.h"

LRenderer::Rasterization::Rasterization(int w, int h) : width(w), height(h) {
  frame_buffer.resize(w * h);
  depth_buffer.resize(w * h);
  AA_frame_buffer.resize(w * freq * h * freq);
  AA_depth_buffer.resize(w * freq * h * freq);
}

void LRenderer::Rasterization::clear() {
  std::fill(frame_buffer.begin(), frame_buffer.end(), Eigen::Vector3f(0, 0, 0));
  std::fill(AA_frame_buffer.begin(), AA_frame_buffer.end(),
            Eigen::Vector3f(0, 0, 0));

  std::fill(depth_buffer.begin(), depth_buffer.end(),
            -std::numeric_limits<float>::infinity());
  std::fill(AA_depth_buffer.begin(), AA_depth_buffer.end(),
            -std::numeric_limits<float>::infinity());
}

inline int Cross2f(Eigen::Vector2f a, Eigen::Vector2f b) {
  return a.x() * b.y() - a.y() * b.x();
}

static bool insideTriangle(float x, float y, const LRenderer::Triangle& _t) {
  Eigen::Vector3f v[3];
  for (int i = 0; i < 3; i++)
    v[i] = {_t[i].windows_position.x(), _t[i].windows_position.y(), 1.0};
  Eigen::Vector3f f0, f1, f2;
  f0 = v[1].cross(v[0]);
  f1 = v[2].cross(v[1]);
  f2 = v[0].cross(v[2]);
  Eigen::Vector3f p(x, y, 1.);
  if ((p.dot(f0) * f0.dot(v[2]) > 0) && (p.dot(f1) * f1.dot(v[0]) > 0) &&
      (p.dot(f2) * f2.dot(v[1]) > 0))
    return true;
  return false;
}

template <class T>
T interpolate(float alpha, float beta, float gamma, const T& v1, const T& v2,
              const T& v3) {
  return v1 * alpha + beta * v2 + gamma * v3;
}

void LRenderer::Rasterization::drawTriangle(const Triangle& t) {
  int minX = std::floor(
      std::min(std::min(t[0].windows_position.x(), t[1].windows_position.x()),
               t[2].windows_position.x()));  // 向下取整
  int maxX = std::ceil(
      std::max(std::max(t[0].windows_position.x(), t[1].windows_position.x()),
               t[2].windows_position.x()));  // 向上取整
  int minY = std::floor(
      std::min(std::min(t[0].windows_position.y(), t[1].windows_position.y()),
               t[2].windows_position.y()));
  int maxY = std::ceil(
      std::max(std::max(t[0].windows_position.y(), t[1].windows_position.y()),
               t[2].windows_position.y()));
  minX = std::clamp(minX, 0, width - 1), maxX = std::clamp(maxX, 0, width - 1),
  minY = std::clamp(minY, 0, height - 1),
  maxY = std::clamp(maxY, 0, height - 1);

  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      // 有抗锯齿
      if (SSAA) {
        int sampling_frequency = freq;
        int sampling_times = sampling_frequency * sampling_frequency;
        float sampling_period = 1.0f / sampling_frequency;
        bool judge = false;
        float z_interpolate;

        for (int m = 0; m < sampling_frequency; m++) {
          for (int n = 0; n < sampling_frequency; n++) {
            float center_x = x + (0.5 + n) * sampling_period;
            float center_y = y + (0.5 + m) * sampling_period;
            if (insideTriangle(center_x, center_y, t)) {
              // 根据中心点算出对应像素的重心坐标
              auto [alpha, beta, gamma] =
                  barycentricPerspectiveLerp(t, center_x, center_y);
              z_interpolate = interpolate(
                  alpha, beta, gamma, t[0].windows_position.z(),
                  t[1].windows_position.z(), t[2].windows_position.z());
              // 对每个小像素执行一次fragShader
              Frag frag =
                  constructFrag(x, y, z_interpolate, alpha, beta, gamma, t);
              // 输入Vertex输出插值后的Frag
              m_shader->fragShader(frag);
              // z为负数，越大离摄像头越近
              int depth_x = x * freq + n, depth_y = y * freq + m;
              if (z_interpolate >
                  AA_depth_buffer[getAAIndex(depth_x, depth_y)]) {
                Eigen::Vector3f color = frag.color;
                AA_depth_buffer[getAAIndex(depth_x, depth_y)] = z_interpolate;
                AA_frame_buffer[getAAIndex(depth_x, depth_y)] = color;
                judge = true;
              }
            }
          }
        }
        if (judge) {
          Eigen::Vector3f average_color(0, 0, 0);
          for (int m = 0; m < sampling_frequency; m++) {
            for (int n = 0; n < sampling_frequency; n++) {
              average_color +=
                  AA_frame_buffer[getAAIndex(freq * x + n, freq * y + m)];
            }
          }
          average_color /= sampling_times;
          Eigen::Vector2i point(x, y);
          setPixel(point, average_color);
        }
      } else if (MSAA) {
        int sampling_frequency = freq;
        int sampling_times = sampling_frequency * sampling_frequency;
        float sampling_period = 1.0f / sampling_frequency;
        auto [alphaPixel, betaPixel, gammaPixel] =
            barycentricPerspectiveLerp(t, x + 0.5, y + 0.5);
        float z_interpolate = interpolate(
            alphaPixel, betaPixel, gammaPixel, t[0].windows_position.z(),
            t[1].windows_position.z(), t[2].windows_position.z());
        Frag frag = constructFrag(x, y, z_interpolate, alphaPixel, betaPixel,
                                  gammaPixel, t);
        // 计算每个像素的颜色
        m_shader->fragShader(frag);

        int count = 0;

        // 对于每一个子像素, 计算AA_frame_buffer的颜色
        for (int m = 0; m < sampling_frequency; m++) {
          for (int n = 0; n < sampling_frequency; n++) {
            float center_x = x + (0.5 + n) * sampling_period;
            float center_y = y + (0.5 + m) * sampling_period;
            if (insideTriangle(center_x, center_y, t)) {
              // 根据中心点算出对应像素的重心坐标
              auto [alpha, beta, gamma] =
                  barycentricPerspectiveLerp(t, center_x, center_y);
              z_interpolate = interpolate(
                  alpha, beta, gamma, t[0].windows_position.z(),
                  t[1].windows_position.z(), t[2].windows_position.z());

              // z为负数，越大离摄像头越近
              int depth_x = x * freq + n, depth_y = y * freq + m;
              if (z_interpolate >
                  AA_depth_buffer[getAAIndex(depth_x, depth_y)]) {
                AA_depth_buffer[getAAIndex(depth_x, depth_y)] = z_interpolate;
                AA_frame_buffer[getAAIndex(depth_x, depth_y)] = frag.color;
                count++;
              }
            }
          }
        }
        if (count > 0) {
          Eigen::Vector3f average_color(0, 0, 0);
          // 对于每个像素的颜色等于子像素的平均
          for (int m = 0; m < sampling_frequency; m++) {
            for (int n = 0; n < sampling_frequency; n++) {
              average_color +=
                  AA_frame_buffer[getAAIndex(freq * x + n, freq * y + m)];
            }
          }
          average_color /= sampling_times;
          Eigen::Vector2i point(x, y);
          setPixel(point, average_color);
        }
      } else {
        if (insideTriangle(x + 0.5, y + 0.5, t)) {
          // 根据中心点算出对应像素的重心坐标
          auto [alpha, beta, gamma] =
              barycentricPerspectiveLerp(t, x + 0.5, y + 0.5);
          float z_interpolate =
              interpolate(alpha, beta, gamma, t[0].windows_position.z(),
                          t[1].windows_position.z(), t[2].windows_position.z());
          Frag frag = constructFrag(x, y, z_interpolate, alpha, beta, gamma, t);
          // 输入Vertex输出插值后的Frag
          m_shader->fragShader(frag);
          //  z为负数，越大离摄像头越近
          if (z_interpolate > depth_buffer[getIndex(x, y)]) {
            Eigen::Vector3f color = frag.color;
            depth_buffer[getIndex(x, y)] = z_interpolate;
            Eigen::Vector2i point(x, y);
            setPixel(point, color);
          }
        }
      }
    }
  }
}

void LRenderer::Rasterization::drawLine(Eigen::Vector3f begin,
                                        Eigen::Vector3f end) {
  auto x1 = begin.x();
  auto y1 = begin.y();
  auto x2 = end.x();
  auto y2 = end.y();

  Eigen::Vector3f line_color = {255, 255, 255};

  int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;

  dx = x2 - x1;
  dy = y2 - y1;
  dx1 = fabs(dx);
  dy1 = fabs(dy);
  px = 2 * dy1 - dx1;
  py = 2 * dx1 - dy1;

  if (dy1 <= dx1) {
    if (dx >= 0) {
      x = x1;
      y = y1;
      xe = x2;
    } else {
      x = x2;
      y = y2;
      xe = x1;
    }
    Eigen::Vector2i point(x, y);
    setPixel(point, line_color);
    for (i = 0; x < xe; i++) {
      x = x + 1;
      if (px < 0) {
        px = px + 2 * dy1;
      } else {
        if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
          y = y + 1;
        } else {
          y = y - 1;
        }
        px = px + 2 * (dy1 - dx1);
      }
      //            delay(0);
      Eigen::Vector2i point(x, y);
      setPixel(point, line_color);
    }
  } else {
    if (dy >= 0) {
      x = x1;
      y = y1;
      ye = y2;
    } else {
      x = x2;
      y = y2;
      ye = y1;
    }
    Eigen::Vector2i point(x, y);
    setPixel(point, line_color);
    for (i = 0; y < ye; i++) {
      y = y + 1;
      if (py <= 0) {
        py = py + 2 * dx1;
      } else {
        if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
          x = x + 1;
        } else {
          x = x - 1;
        }
        py = py + 2 * (dx1 - dy1);
      }
      Eigen::Vector2i point(x, y);
      setPixel(point, line_color);
    }
  }
}

void LRenderer::Rasterization::drawLineTriangle(const Triangle& t) {
  drawLine(t[0].windows_position.head<3>(), t[1].windows_position.head<3>());
  drawLine(t[1].windows_position.head<3>(), t[2].windows_position.head<3>());
  drawLine(t[2].windows_position.head<3>(), t[0].windows_position.head<3>());
}

void LRenderer::Rasterization::setPixel(const Eigen::Vector2i& point,
                                        const Eigen::Vector3f& color) {
  int index = (height - point.y() - 1) * width + point.x();
  frame_buffer[index] = color * 255;
}

void LRenderer::Rasterization::setShader(Shader& shader) {
  this->m_shader = std::make_unique<Shader>(shader);
}

std::unique_ptr<LRenderer::Shader>& LRenderer::Rasterization::shader() {
  return this->m_shader;
}

int LRenderer::Rasterization::getIndex(int x, int y) {
  if (x < 0) x = 0;
  if (x >= width) x = width - 1;
  if (y < 0) y = 0;
  if (y >= height) y = height - 1;
  return (height - 1 - y) * width + x;
}

int LRenderer::Rasterization::getAAIndex(int x, int y) {
  /*if (x < 0 || x >= freq * width || y < 0 || y >= freq * height) return 0;*/
  return (freq * height - 1 - y) * width * freq + x;
}

void LRenderer::Rasterization::openMSAA() {
  if (SSAA) {
    SSAA = false;
  }
  MSAA = true;
}

void LRenderer::Rasterization::openSSAA() {
  if (MSAA) {
    MSAA = false;
  }
  SSAA = true;
}

void LRenderer::Rasterization::openNormal() {
  MSAA = false;
  SSAA = false;
}

void LRenderer::Rasterization::reSize() {
  AA_frame_buffer.resize(width * freq * height * freq);
  AA_depth_buffer.resize(width * freq * height * freq);
}

void LRenderer::Rasterization::set4xFreq() {
  freq = 2;  // 2*2
  reSize();
  clear();
}

void LRenderer::Rasterization::set9xFreq() {
  freq = 3;  // 3*3
  reSize();
  clear();
}

void LRenderer::Rasterization::set16xFreq() {
  freq = 4;  // 4*4
  reSize();
  clear();
}

LRenderer::Frag LRenderer::Rasterization::constructFrag(int x, int y, int z,
                                                        float alpha, float beta,
                                                        float gamma,
                                                        const Triangle& t) {
  Frag frag;
  frag.view_position = interpolate(alpha, beta, gamma, t[0].world_position,
                                   t[1].world_position, t[2].world_position);
  frag.normal =
      interpolate(alpha, beta, gamma, t[0].normal, t[1].normal, t[2].normal)
          .normalized();
  frag.tex_position =
      interpolate(alpha, beta, gamma, t[0].tex, t[1].tex, t[2].tex);
  frag.color =
      interpolate(alpha, beta, gamma, t[0].color, t[1].color, t[2].color);
  frag.windows_position.x() = x;
  frag.windows_position.y() = y;
  return frag;
}
