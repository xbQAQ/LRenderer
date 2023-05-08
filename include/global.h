#ifndef GLOBAL_H
#define GLOBAL_H

#include "Vertex.h"

namespace LRenderer {
#define MY_PI 3.1415926
#define TWO_PI (2.0 * MY_PI)

class Vertex;
class Model;
class PipeLine;
class Texture;
class Camera;
class Material;
class Shader;
class Rasterization;
class UI;

static std::tuple<float, float, float> barycentric(Eigen::Vector4f a,
                                                   Eigen::Vector4f b,
                                                   Eigen::Vector4f c, float x,
                                                   float y) {
  float betaH = (x * (a.y() - c.y()) + y * (c.x() - a.x()) + a.x() * c.x() -
                 c.x() * a.y()) /
                (b.x() * (a.y() - c.y()) + b.y() * (c.x() - a.x()) +
                 a.x() * c.x() - c.x() * a.y());
  float gammaH = (x * (a.y() - b.y()) + y * (b.x() - a.x()) + a.x() * b.x() -
                  b.x() * a.y()) /
                 (c.x() * (a.y() - b.y()) + c.y() * (b.x() - a.x()) +
                  a.x() * b.x() - b.x() * a.y());
  float alphaH = 1.0 - betaH - gammaH;
  return {alphaH, betaH, gammaH};
}

static std::tuple<float, float, float> barycentricPerspectiveLerp(
    const Triangle& t, float x, float y) {
  // alpha' beta' gamma' 为透视后的
  double Xa = t[0].windows_position.x(), Xb = t[1].windows_position.x(),
        Xc = t[2].windows_position.x();
  double Ya = t[0].windows_position.y(), Yb = t[1].windows_position.y(),
        Yc = t[2].windows_position.y();

  float gammaH = (x * (Ya - Yb) + y * (Xb - Xa) + Xa * Yb - Xb * Ya) /
                (Xc * (Ya - Yb) + Yc * (Xb - Xa) + Xa * Yb - Xb * Ya);
  float betaH = (x * (Ya - Yc) + y * (Xc - Xa) + Xa * Yc - Xc * Ya) /
                (Xb * (Ya - Yc) + Yb * (Xc - Xa) + Xa * Yc - Xc * Ya);
  float alphaH = 1.0 - betaH - gammaH;

  float Wa = t[0].windows_position.w(), Wb = t[1].windows_position.w(),
        Wc = t[2].windows_position.w();

  float W = 1.0 / (alphaH / Wa + betaH / Wb + gammaH / Wc);
  float alpha = (alphaH / Wa) * W;
  float beta = (betaH / Wb) * W;
  float gamma = 1.0 - alpha - beta;

  return {alpha, beta, gamma};
}
}  // namespace LRenderer

#endif