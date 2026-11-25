#include "collisions.h"
#include <algorithm>
#include <cmath>
bool circleIntersectsWall(glm::vec3 p,float radius,const WallAABB& w){
 float x=std::max(w.minX,std::min(w.maxX,p.x));
 float z=std::max(w.minZ,std::min(w.maxZ,p.z));
 float dx=p.x-x,dz=p.z-z;
 return dx*dx+dz*dz < radius*radius;
}
bool collidesWithMap(glm::vec3 p,float radius,const std::vector<WallAABB>& walls){
 for(const auto& w:walls) if(circleIntersectsWall(p,radius,w))return true;
 return false;
}
glm::vec3 slideAgainstWalls(glm::vec3 p,glm::vec3 d,float r,const std::vector<WallAABB>& walls){
 // Discrete swept subdivision avoids tunneling even under a long delta.
 const float length=std::sqrt(d.x*d.x+d.z*d.z);
 int steps=std::max(1,(int)std::ceil(length/.12f));
 const glm::vec3 step=d/(float)steps;
 for(int i=0;i<steps;i++){
   glm::vec3 x=p;x.x+=step.x;
   if(!collidesWithMap(x,r,walls))p=x;
   glm::vec3 z=p;z.z+=step.z;
   if(!collidesWithMap(z,r,walls))p=z;
 }
 return p;
}
