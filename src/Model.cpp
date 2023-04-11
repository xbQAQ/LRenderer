#include "Model.h"
#include "API/OBJ_Loader.h"

void LRenderer::Model::loadModel(std::string path) {
  objects.clear();
  objl::Loader Loader;
  bool loadout = Loader.LoadFile(path);
  if (loadout) {
    for (auto& mesh : Loader.LoadedMeshes) {
      Object object;
      int vnum = mesh.Vertices.size(), inum = mesh.Indices.size();
      Mesh m(vnum, inum);
      object.mesh = m;
      for (int i = 0; i != mesh.Vertices.size(); i += 3) {
        for (int j = 0; j != 3; j++) {
          Vertex v;
          v.setPosition(Eigen::Vector4f(mesh.Vertices[i + j].Position.X,
                                        mesh.Vertices[i + j].Position.Y,
                                        mesh.Vertices[i + j].Position.Z, 1));
          v.setNormal(Eigen::Vector3f(mesh.Vertices[i + j].Normal.X,
                                      mesh.Vertices[i + j].Normal.Y,
                                      mesh.Vertices[i + j].Normal.Z));
          v.setTex(Eigen::Vector2f(mesh.Vertices[i + j].TextureCoordinate.X,
                                   mesh.Vertices[i + j].TextureCoordinate.Y));
          
          object.mesh.vertexBuffer[i + j] = v;
        }
      }
      for (int i = 0; i != mesh.Indices.size(); i++) {
        object.mesh.indexBuffer[i] = mesh.Indices[i];
      }

      object.material.ka =
          Eigen::Vector3f(mesh.MeshMaterial.Ka.X, mesh.MeshMaterial.Ka.Y,
                          mesh.MeshMaterial.Ka.Z);
      object.material.kd =
          Eigen::Vector3f(mesh.MeshMaterial.Kd.X, mesh.MeshMaterial.Kd.Y,
                          mesh.MeshMaterial.Kd.Z);
      object.material.ks =
          Eigen::Vector3f(mesh.MeshMaterial.Ks.X, mesh.MeshMaterial.Ks.Y,
                          mesh.MeshMaterial.Ks.Z);
      objects.push_back(object);
    }
    for (int i = 0; i != Loader.LoadedMaterials.size(); i++) {
      if (objects[i].material.ka == Eigen::Vector3f(0, 0, 0)) {
        objects[i].material.ka = Eigen::Vector3f(
            Loader.LoadedMaterials[i].Ka.X, Loader.LoadedMaterials[i].Ka.Y,
            Loader.LoadedMaterials[i].Ka.Z);
      }
      if (objects[i].material.kd == Eigen::Vector3f(0, 0, 0)) {
        objects[i].material.kd = Eigen::Vector3f(
            Loader.LoadedMaterials[i].Kd.X, Loader.LoadedMaterials[i].Kd.Y,
            Loader.LoadedMaterials[i].Kd.Z);
      }
      if (objects[i].material.ks == Eigen::Vector3f(0, 0, 0)) {
        objects[i].material.ks = Eigen::Vector3f(
            Loader.LoadedMaterials[i].Ks.X, Loader.LoadedMaterials[i].Ks.Y,
            Loader.LoadedMaterials[i].Ks.Z);
      }
    } 
  } else {
    std::cout << "Load " + path + " error!" << std::endl;
    exit(-1);
  }
}