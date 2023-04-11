#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include "Vertex.h"

namespace LRenderer {

class Mesh {
 public:
  Mesh() = default;
  Mesh(const int& vnum, const int inum) {
    vertexBuffer.resize(vnum);
    indexBuffer.resize(inum);
  }
  ~Mesh() = default;
  
  std::vector<Vertex> vertexBuffer;
  std::vector<unsigned int> indexBuffer;
  
};
}  // namespace LRenderer

#endif
