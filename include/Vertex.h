#ifndef VERTEX_H
#define VERTEX_H

#include <Eigen/Eigen>

namespace LRenderer {

class Vertex {
 public:
  Vertex() = default;
  Vertex(const Eigen::Vector4f& p,
         const Eigen::Vector3f& n = Eigen::Vector3f(0, 0, 1, 0),
         const Eigen::Vector3f& c = Eigen::Vector3f(255, 255, 255),
         const Eigen::Vector2f& t = Eigen::Vector2f(0, 0))
      : windows_position(p),
        world_position(p.head<3>()),
        position(p),
        normal(n),
        color(c),
        tex(t) {}
  Vertex(const Vertex& V)
      : windows_position(V.windows_position),
        world_position(V.world_position),
        position(V.position),
        normal(V.normal),
        color(V.color),
        tex(V.tex) {}
  ~Vertex() {}

  void setPosition(const Eigen::Vector4f& p);
  void setNormal(const Eigen::Vector3f& n);
  void setColor(const Eigen::Vector3f& c);
  void setTex(const Eigen::Vector2f& t);

  Eigen::Vector4f windows_position;
  Eigen::Vector3f world_position;
  Eigen::Vector4f position;
  Eigen::Vector3f normal;
  Eigen::Vector3f color;
  Eigen::Vector2f tex;
};

struct Frag {
  Eigen::Vector3f view_position;
  Eigen::Vector3f color;
  Eigen::Vector3f normal;
  Eigen::Vector2f tex_position;
  Eigen::Vector2f windows_position;
};

using Triangle = std::array<Vertex, 3>;

class Line {
 public:
  Vertex v1, v2;
  bool isNull = false;
  Line() : isNull(true) {}
  Line(const Vertex& _v1, const Vertex& _v2) : v1(_v1), v2(_v2) {}
};

}  // namespace LRenderer

#endif
