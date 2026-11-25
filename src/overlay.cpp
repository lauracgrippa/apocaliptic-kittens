#include "overlay.h"
#include <unordered_map>
#include <array>
#include <cctype>
#include <cstddef>
#include <iostream>
namespace {
using Glyph=std::array<const char*,7>;
const std::unordered_map<char,Glyph> glyphs = {
 {'A',{".###.","#...#","#...#","#####","#...#","#...#","#...#"}},
 {'B',{"####.","#...#","#...#","####.","#...#","#...#","####."}},
 {'C',{".####","#....","#....","#....","#....","#....",".####"}},
 {'D',{"####.","#...#","#...#","#...#","#...#","#...#","####."}},
 {'E',{"#####","#....","#....","####.","#....","#....","#####"}},
 {'F',{"#####","#....","#....","####.","#....","#....","#...."}},
 {'G',{".####","#....","#....","#.###","#...#","#...#",".###."}},
 {'H',{"#...#","#...#","#...#","#####","#...#","#...#","#...#"}},
 {'I',{"#####","..#..","..#..","..#..","..#..","..#..","#####"}},
 {'J',{"..###","...#.","...#.","...#.","#..#.","#..#.",".##.."}},
 {'K',{"#...#","#..#.","#.#..","##...","#.#..","#..#.","#...#"}},
 {'L',{"#....","#....","#....","#....","#....","#....","#####"}},
 {'M',{"#...#","##.##","#.#.#","#.#.#","#...#","#...#","#...#"}},
 {'N',{"#...#","##..#","#.#.#","#..##","#...#","#...#","#...#"}},
 {'O',{".###.","#...#","#...#","#...#","#...#","#...#",".###."}},
 {'P',{"####.","#...#","#...#","####.","#....","#....","#...."}},
 {'Q',{".###.","#...#","#...#","#...#","#.#.#","#..#.",".##.#"}},
 {'R',{"####.","#...#","#...#","####.","#.#..","#..#.","#...#"}},
 {'S',{".####","#....","#....",".###.","....#","....#","####."}},
 {'T',{"#####","..#..","..#..","..#..","..#..","..#..","..#.."}},
 {'U',{"#...#","#...#","#...#","#...#","#...#","#...#",".###."}},
 {'V',{"#...#","#...#","#...#","#...#","#...#",".#.#.","..#.."}},
 {'W',{"#...#","#...#","#...#","#.#.#","#.#.#","##.##","#...#"}},
 {'X',{"#...#","#...#",".#.#.","..#..",".#.#.","#...#","#...#"}},
 {'Y',{"#...#","#...#",".#.#.","..#..","..#..","..#..","..#.."}},
 {'Z',{"#####","....#","...#.","..#..",".#...","#....","#####"}},
 {'0',{".###.","#...#","#..##","#.#.#","##..#","#...#",".###."}},
 {'1',{"..#..",".##..","..#..","..#..","..#..","..#..",".###."}},
 {'2',{".###.","#...#","....#","...#.","..#..",".#...","#####"}},
 {'3',{"####.","....#","....#",".###.","....#","....#","####."}},
 {'4',{"...#.","..##.",".#.#.","#..#.","#####","...#.","...#."}},
 {'5',{"#####","#....","#....","####.","....#","....#","####."}},
 {'6',{".###.","#....","#....","####.","#...#","#...#",".###."}},
 {'7',{"#####","....#","...#.","..#..",".#...",".#...",".#..."}},
 {'8',{".###.","#...#","#...#",".###.","#...#","#...#",".###."}},
 {'9',{".###.","#...#","#...#",".####","....#","....#",".###."}},
 {'/',{"....#","....#","...#.","..#..",".#...","#....","#...."}},
 {'!',{"..#..","..#..","..#..","..#..","..#..",".....","..#.."}},
 {':',{".....","..#..","..#..",".....","..#..","..#..","....."}},
 {'-',{".....",".....",".....","#####",".....",".....","....."}},
 {'.',{".....",".....",".....",".....",".....",".##..",".##.."}},
 {'?',{".###.","#...#","....#","...#.","..#..",".....","..#.."}},
 {'+',{".....","..#..","..#..","#####","..#..","..#..","....."}},
 {'%',{"##..#","##.#.","...#.","..#..",".#...",".#.##","#..##"}},
 {' ',{".....",".....",".....",".....",".....",".....","....."}}
};
GLuint shader(GLenum type,const char* src){GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char b[1024];glGetShaderInfoLog(s,1024,nullptr,b);std::cerr<<"HUD shader: "<<b<<'\n';}return s;}
}
bool Overlay::init(){
 const char* vs="#version 330 core\nlayout(location=0) in vec2 aPos;layout(location=1) in vec4 aColor;out vec4 vColor;void main(){gl_Position=vec4(aPos.x/320.0-1.0,1.0-aPos.y/180.0,0,1);vColor=aColor;}";
 const char* fs="#version 330 core\nin vec4 vColor;out vec4 FragColor;void main(){FragColor=vColor;}";
 GLuint a=shader(GL_VERTEX_SHADER,vs),b=shader(GL_FRAGMENT_SHADER,fs);
 program=glCreateProgram();glAttachShader(program,a);glAttachShader(program,b);glLinkProgram(program);glDeleteShader(a);glDeleteShader(b);
 GLint ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok)return false;
 glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
 glBufferData(GL_ARRAY_BUFFER,200000*sizeof(V),nullptr,GL_STREAM_DRAW);
 glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,x));glEnableVertexAttribArray(0);
 glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(V),(void*)offsetof(V,r));glEnableVertexAttribArray(1);
 glBindVertexArray(0);return true;
}
void Overlay::tri(float x1,float y1,float x2,float y2,float x3,float y3,float r,float g,float b,float a){vertices.push_back({x1,y1,r,g,b,a});vertices.push_back({x2,y2,r,g,b,a});vertices.push_back({x3,y3,r,g,b,a});}
void Overlay::rect(float x,float y,float w,float h,float r,float g,float b,float a){tri(x,y,x+w,y,x+w,y+h,r,g,b,a);tri(x,y,x+w,y+h,x,y+h,r,g,b,a);}
void Overlay::text(int x,int y,std::string s,int px,float r,float g,float b){
 for(char ch:s){ch=static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  const auto it=glyphs.find(ch);if(it!=glyphs.end())for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(it->second[yy][xx]=='#')rect(x+xx*px,y+yy*px,px,px,r,g,b);
  x+=6*px;
 }
}
void Overlay::flush(){if(vertices.empty())return;glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glUseProgram(program);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(V),vertices.data(),GL_STREAM_DRAW);glDrawArrays(GL_TRIANGLES,0,(GLsizei)vertices.size());glBindVertexArray(0);glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);}
void Overlay::destroy(){if(vbo)glDeleteBuffers(1,&vbo);if(vao)glDeleteVertexArrays(1,&vao);if(program)glDeleteProgram(program);}
