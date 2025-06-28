#pragma once
#include "glm/glm.hpp"
#include "vkRender.h" 

namespace vkRender
{
class Camera
{
public:
    static Camera* Instance();

    void setPos(glm::vec3 pos);
    glm::vec3 getPos() const {return pos_;}
    void setFovy(float fovy);
    float getFovy() const {return fovy_;}
    glm::mat4 getView() const {return view_;}
    glm::mat4 getProj() const {return proj_;}
    
    Camera();
private:
    void update(glm::vec3 pos, float fovy);
    
private:
    glm::vec3 pos_;
    float fovy_;

    glm::mat4 view_;
    glm::mat4 proj_;

    static Camera* instance;
};
}
