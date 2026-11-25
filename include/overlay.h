#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
struct Overlay {
 static constexpr int W=640, H=360;
 GLuint program=0,vao=0,vbo=0;
 struct V {float x,y,r,g,b,a;};
 std::vector<V> vertices;
 bool init();
 void destroy();
 void clear(){vertices.clear();}
 void rect(float x,float y,float w,float h,float r,float g,float b,float a=1.f);
 void tri(float x1,float y1,float x2,float y2,float x3,float y3,float r,float g,float b,float a=1.f);
 void text(int x,int y,std::string value,int pixels,float r,float g,float b);
 void flush();
};
