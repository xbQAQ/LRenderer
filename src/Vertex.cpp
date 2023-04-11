#include "Vertex.h"

void LRenderer::Vertex::setPosition(const Eigen::Vector4f& p) { position = p; }
void LRenderer::Vertex::setNormal(const Eigen::Vector3f& n) { normal = n; }
void LRenderer::Vertex::setColor(const Eigen::Vector3f& c) { color = c / 255; }
void LRenderer::Vertex::setTex(const Eigen::Vector2f& t) { tex = t; }