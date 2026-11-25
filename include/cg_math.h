#pragma once
// Matrices authored for the INF01047 assignment. GLM is used only for vector/matrix
// storage and basic vector operators, NEVER for camera or transform construction.
#include <glm/glm.hpp>
#include <cmath>
namespace cg {
inline glm::mat4 translate(const glm::vec3& t){glm::mat4 m(1.0f);m[3][0]=t.x;m[3][1]=t.y;m[3][2]=t.z;return m;}
inline glm::mat4 scale(const glm::vec3& s){glm::mat4 m(1.0f);m[0][0]=s.x;m[1][1]=s.y;m[2][2]=s.z;return m;}
inline glm::mat4 rotate_x(float a){float c=std::cos(a),s=std::sin(a);glm::mat4 m(1.0f);m[1][1]=c;m[1][2]=s;m[2][1]=-s;m[2][2]=c;return m;}
inline glm::mat4 rotate_y(float a){float c=std::cos(a),s=std::sin(a);glm::mat4 m(1.0f);m[0][0]=c;m[0][2]=-s;m[2][0]=s;m[2][2]=c;return m;}
inline glm::mat4 rotate_z(float a){float c=std::cos(a),s=std::sin(a);glm::mat4 m(1.0f);m[0][0]=c;m[0][1]=s;m[1][0]=-s;m[1][1]=c;return m;}
inline glm::mat4 model(const glm::mat4& p,glm::vec3 pos,glm::vec3 size,float rx=0,float ry=0,float rz=0){
 return p * translate(pos) * rotate_x(rx) * rotate_y(ry) * rotate_z(rz) * scale(size);
}
inline glm::mat4 view(glm::vec3 eye,glm::vec3 focus){
 glm::vec3 z=glm::normalize(eye-focus);
 glm::vec3 x=glm::normalize(glm::cross(glm::vec3(0,1,0),z));
 glm::vec3 y=glm::cross(z,x);
 glm::mat4 m(1);
 m[0][0]=x.x;m[1][0]=x.y;m[2][0]=x.z;m[3][0]=-glm::dot(x,eye);
 m[0][1]=y.x;m[1][1]=y.y;m[2][1]=y.z;m[3][1]=-glm::dot(y,eye);
 m[0][2]=z.x;m[1][2]=z.y;m[2][2]=z.z;m[3][2]=-glm::dot(z,eye);
 return m;
}
inline glm::mat4 perspective(float fovRadians,float aspect,float n=0.065f,float f=120.f){
 const float q=1.f/std::tan(fovRadians*.5f);glm::mat4 m(0);
 m[0][0]=q/aspect;m[1][1]=q;m[2][2]=-(f+n)/(f-n);
 m[2][3]=-1.f;m[3][2]=-2*f*n/(f-n);return m;
}
inline glm::vec3 forward(float yaw,float pitch=0){return {-std::sin(yaw)*std::cos(pitch),std::sin(pitch),-std::cos(yaw)*std::cos(pitch)};}
}
