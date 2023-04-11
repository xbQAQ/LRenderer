#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include "Object.h"

namespace LRenderer {
class Model {
 public:
  Model() = default;
  Model(const Model& m) = delete;
  Model& operator=(const Model&) = delete;
  ~Model() = default;

  std::vector<Object> objects;
  void loadModel(std::string path);
};
}  // namespace LRenderer

#endif
