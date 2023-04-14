#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen/Eigen>
#include <iostream>

#include "global.h"
namespace LRenderer {
class Camera {
 public:
  Eigen::Vector3f eye_pos;
  Eigen::Vector3f up;
  Eigen::Vector3f right;
  Eigen::Vector3f front;
  Eigen::Vector3f world_up;
  float fov;
  float aspect_radio;
  float z_near, z_far;
  Eigen::Vector3f axis;       // ��ת��
  float rotate_angle;         // ��ת�Ƕ�
  Eigen::Vector3f scale;      // ����
  Eigen::Vector3f translate;  // ƽ��

  Camera(const Eigen::Vector3f& _eye_pos = {0, 0, 10},
         const Eigen::Vector3f& _up = {0, 1, 0},
         const Eigen::Vector3f& _look_at = {0, 0, 0}, float _fov = 45.0f,
         float w = 700, float h = 700, float _z_near = 0.1, float _zfar = 50,
         const Eigen::Vector3f& _axis = {0, 1, 0}, float _rotate_angle = 0,
         const Eigen::Vector3f& _scale = {2.5, 2.5, 2.5},
         const Eigen::Vector3f& _translate = {0, 0, 0})
      : eye_pos(_eye_pos),
        world_up(_up),
        fov(_fov),
        aspect_radio(w / h),
        z_near(_z_near),
        z_far(_zfar),
        axis(_axis),
        rotate_angle(_rotate_angle),
        scale(_scale),
        translate(_translate),
        x_angle(0),
        y_angle(180),
        z_angle(0) {
    front = (eye_pos - _look_at) / (eye_pos - _look_at).norm();
    right = world_up.cross(front) / world_up.cross(front).norm();
    up = front.cross(right) / front.cross(right).norm();
  }

  void reset();
  void changeNear(float _near);
  void changeFar(float _far);
  void updateEyePos(const Eigen::Vector3f& _eye_pos);
  void updateFov(float _fov);
  void updateAspect(int w, int h);
  void moveForward();
  void moveBack();
  void moveRight();
  void moveLeft();
  void moveUp();
  void moveDown();
  void magnification();                   //�Ŵ�
  void shrinks();                         //��С
  void rotatePositiveDirectionAroundX();  //��X����������ת
  void rotateNegitiveDirectionAroundX();  //��X����������ת
  void rotatePositiveDirectionAroundY();  //��X����������ת
  void rotateNegitiveDirectionAroundY();  //��X����������ת
  void rotatePositiveDirectionAroundZ();  //��X����������ת
  void rotateNegitiveDirectionAroundZ();  //��X����������ת

 private:
  float x_angle, y_angle, z_angle;
};
}  // namespace LRenderer

#endif
