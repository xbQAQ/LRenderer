#ifndef OBJECT_H
#define OBJECT_H

#include "Material.h"
#include "Mesh.h"

namespace LRenderer {
class Object {
 public:
  Object() = default;
  Object(const Mesh& me, const Material& ma) {
    mesh = me;
    material = ma;
  }
  Object(const Object& obj) {
    if (&obj != this) {
      mesh = obj.mesh;
      material = obj.material;
    }
  }
  Object& operator = (const Object& obj) {
    if (&obj != this) {
      mesh = obj.mesh;
      material = obj.material;
    }
  }
  Mesh mesh;
  Material material;
};
}  // namespace LRenderer

#endif
