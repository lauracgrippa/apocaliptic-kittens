#pragma once
#include <glm/glm.hpp>
#include <vector>
struct WallAABB { float minX,maxX,minZ,maxZ; };
bool circleIntersectsWall(glm::vec3 center,float radius,const WallAABB& wall);
bool collidesWithMap(glm::vec3 center,float radius,const std::vector<WallAABB>& walls);
glm::vec3 slideAgainstWalls(glm::vec3 start,glm::vec3 delta,float radius,const std::vector<WallAABB>& walls);
