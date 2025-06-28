#include "Camera.h"

#include "Context.h"

namespace vkRender
{
Camera* Camera::instance = nullptr;

Camera* Camera::Instance()
{
    if (instance == nullptr)
    {
        instance = new (std::nothrow) Camera;
    }
    return instance;
}

void Camera::setPos(glm::vec3 pos)
{
    this->pos_ = pos;
    update(pos, fovy_);
}

void Camera::setFovy(float fovy)
{
    this->fovy_ = fovy;
    update(pos_, fovy);
}

Camera::Camera():
pos_(0),
fovy_(45.f)
{
}

void Camera::update(glm::vec3 pos, float fovy)
{
    auto frameSize = Context::getInstance()->getFrameSize();
    view_ = glm::lookAt(pos, glm::vec3(0), glm::vec3(0.f, 0.f, 1.f));
    proj_ = glm::perspective(glm::radians(fovy), static_cast<float>(frameSize.width) / frameSize.height, 0.1f, 10.f);
    proj_[1][1] *= -1;
}
}
