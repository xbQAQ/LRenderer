#include "Camera.h"
void LRenderer::Camera::reset() {
  eye_pos = {0, 0, 10};
  up = {0, 1, 0};
  fov = 45.0f;
  z_near = 0.1;
  z_far = 50;
  axis = {0, 0, 0};
  rotate_angle = 0;
  scale = {2.5, 2.5, 2.5};
  translate = {0, 0, 0};
}

void LRenderer::Camera::updateEyePos(const Eigen::Vector3f& _eye_pos) {
  eye_pos = _eye_pos;
}

void LRenderer::Camera::updateFov(float _fov) {
  fov = _fov;
}

void LRenderer::Camera::updateAspect(int w, int h) { aspect_radio = w / h; }

void LRenderer::Camera::moveForward() { eye_pos -= front * 0.2; }

void LRenderer::Camera::moveBack() { eye_pos += front * 0.2; }

void LRenderer::Camera::moveRight() {
  translate += Eigen::Vector3f(1, 0, 0) * 0.2;
}

void LRenderer::Camera::moveLeft() {
  translate -= Eigen::Vector3f(1, 0, 0) * 0.2;
}

void LRenderer::Camera::moveUp() {
  translate += Eigen::Vector3f(0, 1, 0) * 0.2;
}

void LRenderer::Camera::moveDown() {
  translate -= Eigen::Vector3f(0, 1, 0) * 0.2;
}

void LRenderer::Camera::magnification() {
  scale += Eigen::Vector3f().Ones() * 0.5;
}

void LRenderer::Camera::shrinks() {
  scale -= Eigen::Vector3f().Ones() * 0.5;
}

//逆时针为正方向
void LRenderer::Camera::rotatePositiveDirectionAroundX() {
  axis = Eigen::Vector3f(1, 0, 0);
  rotate_angle += 10;
}

void LRenderer::Camera::rotateNegitiveDirectionAroundX() {
  axis = Eigen::Vector3f(1, 0, 0);
  rotate_angle -= 10;
}

void LRenderer::Camera::rotatePositiveDirectionAroundY() {
  axis = Eigen::Vector3f(0, 1, 0);
  rotate_angle += 10;
}

void LRenderer::Camera::rotateNegitiveDirectionAroundY() {
  axis = Eigen::Vector3f(0, 1, 0);
  rotate_angle -= 10;
}

void LRenderer::Camera::rotatePositiveDirectionAroundZ() {
  axis = Eigen::Vector3f(0, 0, 1);
  rotate_angle += 10;
}

void LRenderer::Camera::rotateNegitiveDirectionAroundZ() {
  axis = Eigen::Vector3f(0, 0, 1);
  rotate_angle -= 10;
}
