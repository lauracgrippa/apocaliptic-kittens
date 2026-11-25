// Apocaliptic Kittens — visual prototype, no game engine.
// Based on the GLFW/GLAD/GLM/tinyobj stack supplied in the UFRGS course ZIP.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>
#include <stb_image.h>
#include "cg_math.h"
#include "collisions.h"
#include "overlay.h"
#include <iomanip>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr int WIDTH = 1280, HEIGHT = 720;
float clamp(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }

struct Vertex { glm::vec3 p, n; glm::vec2 uv; };
struct Mesh {
    GLuint vao=0, vbo=0; GLsizei count=0;
    bool load(const std::string& file) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warning, error;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file.c_str(), "../../data/", true)) {
            std::cerr << "OBJ nao encontrado: " << file << " " << error << '\n'; return false;
        }
        std::vector<Vertex> data;
        for (const auto& shape : shapes) {
            size_t off=0;
            for (int faceCount : shape.mesh.num_face_vertices) {
                if (faceCount != 3) { off += faceCount; continue; }
                glm::vec3 p[3];
                for (int k=0;k<3;k++) {
                    int id=shape.mesh.indices[off+k].vertex_index;
                    if (id<0 || 3*id+2 >= static_cast<int>(attrib.vertices.size())) return false;
                    p[k] = glm::vec3(attrib.vertices[3*id], attrib.vertices[3*id+1], attrib.vertices[3*id+2]);
                }
                glm::vec3 normal = glm::cross(p[1]-p[0],p[2]-p[0]);
                if (glm::length(normal)<0.00001f) { off+=3; continue; }
                normal=glm::normalize(normal);
                for (int k=0;k<3;k++) {
                    const auto& idx=shape.mesh.indices[off+k];
                    glm::vec2 uv;
                    if (idx.texcoord_index>=0 && static_cast<size_t>(2*idx.texcoord_index+1)<attrib.texcoords.size()) {
                        uv={attrib.texcoords[2*idx.texcoord_index],attrib.texcoords[2*idx.texcoord_index+1]};
                    } else if (std::abs(normal.y)>.6f) uv={p[k].x+.5f,p[k].z+.5f};
                    else if (std::abs(normal.z)>.6f) uv={p[k].x+.5f,p[k].y+.5f};
                    else uv={p[k].z+.5f,p[k].y+.5f};
                    data.push_back({p[k],normal,uv});
                }
                off+=3;
            }
        }
        if (data.empty()) return false;
        count=static_cast<GLsizei>(data.size());
        glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
        glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(Vertex),data.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,p));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,n));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
        glBindVertexArray(0);
        return true;
    }
    void draw() const { glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,count);glBindVertexArray(0); }
    void destroy() { if(vbo)glDeleteBuffers(1,&vbo);if(vao)glDeleteVertexArrays(1,&vao); }
};

std::string read_text(const std::string& path) {
    std::ifstream f(path); if(!f) return {};
    std::ostringstream s; s<<f.rdbuf(); return s.str();
}
GLuint compile(GLenum type, const std::string& source) {
    GLuint s=glCreateShader(type); const char* p=source.c_str();
    glShaderSource(s,1,&p,nullptr);glCompileShader(s);
    GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok) {char msg[2048];glGetShaderInfoLog(s,sizeof(msg),nullptr,msg);std::cerr<<"Shader: "<<msg<<'\n';glDeleteShader(s);return 0;}
    return s;
}
GLuint create_program() {
    auto vs=compile(GL_VERTEX_SHADER,read_text("../../src/preview_vertex.glsl"));
    auto fs=compile(GL_FRAGMENT_SHADER,read_text("../../src/preview_fragment.glsl"));
    if(!vs||!fs) return 0;
    GLuint id=glCreateProgram();glAttachShader(id,vs);glAttachShader(id,fs);glLinkProgram(id);
    GLint ok=0;glGetProgramiv(id,GL_LINK_STATUS,&ok);
    if(!ok) {char msg[2048];glGetProgramInfoLog(id,sizeof(msg),nullptr,msg);std::cerr<<"Link: "<<msg<<'\n';glDeleteProgram(id);id=0;}
    glDeleteShader(vs);glDeleteShader(fs);return id;
}
GLuint load_texture(const char* file) {
    int w=0,h=0,ch=0;stbi_set_flip_vertically_on_load(true);
    unsigned char* pix=stbi_load(file,&w,&h,&ch,0);
    if(!pix) {std::cerr<<"Textura indisponivel: "<<file<<'\n';return 0;}
    GLenum format=ch==4?GL_RGBA:ch==3?GL_RGB:GL_RED;
    GLuint t=0;glGenTextures(1,&t);glBindTexture(GL_TEXTURE_2D,t);
    glTexImage2D(GL_TEXTURE_2D,0,format,w,h,0,format,GL_UNSIGNED_BYTE,pix);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    stbi_image_free(pix);return t;
}

// Render objects reuse the mesh data while each instance has an independent Model matrix.
struct Render {
    GLuint program=0, groundTex=0, brickTex=0, metalTex=0;
    Mesh cube,sphere;
    GLint uModel=-1,uColor=-1,uTextured=-1,uTile=-1,uBoxUV=-1,uObjectScale=-1;
    bool init(){
        program=create_program();if(!program)return false;
        if(!cube.load("../../data/cube.obj")||!sphere.load("../../data/sphere.obj"))return false;
        groundTex=load_texture("../../data/rocky_terrain_02_diff_1k.jpg");
        brickTex=load_texture("../../data/red_brick_diff_1k.jpg");
        metalTex=load_texture("../../data/metal.png");
        glUseProgram(program);
        uModel=glGetUniformLocation(program,"uModel");uColor=glGetUniformLocation(program,"uColor");
        uTextured=glGetUniformLocation(program,"uTextured");uTile=glGetUniformLocation(program,"uTile");
        uBoxUV=glGetUniformLocation(program,"uBoxUV");uObjectScale=glGetUniformLocation(program,"uObjectScale");
        glUniform1i(glGetUniformLocation(program,"uTexture"),0);
        glUniform3f(glGetUniformLocation(program,"uSunDirection"),-.45f,.82f,.18f);
        return groundTex&&brickTex&&metalTex;
    }
    void begin(const glm::mat4& view,const glm::mat4& projection,const glm::vec3& cam){
        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program,"uView"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program,"uProjection"),1,GL_FALSE,glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(program,"uCameraPosition"),1,glm::value_ptr(cam));
    }
    // tiles: repeticoes da textura. Com boxSize != 0 (caixas texturizadas), passa a ser
    // repeticoes por metro, e o UV vem do tamanho real de cada face: a imagem nao estica.
    void draw(const Mesh& mesh,const glm::mat4& transform,glm::vec3 color,GLuint texture=0,float tiles=1,
              glm::vec3 boxSize=glm::vec3(0))const{
        glUniformMatrix4fv(uModel,1,GL_FALSE,glm::value_ptr(transform));
        glUniform3fv(uColor,1,glm::value_ptr(color));
        glUniform1i(uTextured,texture!=0);glUniform1f(uTile,tiles);
        glUniform1i(uBoxUV,boxSize!=glm::vec3(0));glUniform3fv(uObjectScale,1,glm::value_ptr(boxSize));
        glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture);mesh.draw();
    }
    void box(glm::mat4 parent,glm::vec3 local,glm::vec3 size,glm::vec3 color,
        float rx=0,float ry=0,float rz=0,GLuint texture=0,float perMeter=1)const{
        glm::mat4 m=cg::model(parent,local,size,rx,ry,rz)*cg::translate({0,.5f,0});
        draw(cube,m,color,texture,perMeter,texture?size:glm::vec3(0));
    }
    void ball(glm::mat4 parent,glm::vec3 local,glm::vec3 size,glm::vec3 color)const{
        draw(sphere,cg::model(parent,local,size),color);
    }
    void destroy(){
      cube.destroy();sphere.destroy();
      if(groundTex)glDeleteTextures(1,&groundTex);
      if(brickTex)glDeleteTextures(1,&brickTex);
      if(metalTex)glDeleteTextures(1,&metalTex);
      if(program)glDeleteProgram(program);
    }
};

struct Cat {
    glm::vec3 pos={0,0,8};float yaw=0,walkPhase=0,motion=0;
    void updateAnimation(float speed,float dt){
        if(speed>.01f){walkPhase+=(speed>4.5f?13.f:8.f)*dt;motion=std::min(1.f,motion+dt*7);}
        else motion=std::max(0.f,motion-dt*9);
    }
    void draw(const Render& r,float t)const{
        const glm::vec3 orange{.99f,.39f,.08f},dark{.30f,.16f,.13f},cream{1.0f,.81f,.52f};
        float bob=.035f*std::sin(walkPhase*2)*motion;
        glm::mat4 root=cg::translate(pos+glm::vec3(0,bob,0))*cg::rotate_y(yaw);
        r.ball(root,{0,.56f,.08f},{.42f,.34f,.66f},orange);
        r.ball(root,{0,.65f,-.55f},{.35f,.34f,.33f},orange);
        r.ball(root,{0,.41f,-.76f},{.19f,.13f,.13f},cream);
        r.ball(root,{-.13f,.70f,-.84f},{.06f,.08f,.044f},dark);
        r.ball(root,{ .13f,.70f,-.84f},{.06f,.08f,.044f},dark);
        r.ball(root,{0,.47f,-.90f},{.05f,.05f,.05f},{.93f,.44f,.45f});
        for(float side:{-1.f,1.f}){
            r.box(root,{side*.23f,1.f,-.61f},{.18f,.28f,.14f},orange,0,0,side*.3f);
            r.ball(root,{side*.23f,.96f,-.69f},{.07f,.12f,.04f},{.97f,.64f,.52f});
        }
        for(int i=0;i<4;i++){
            float sx=i%2==0?-1.f:1.f,sz=i<2?-.32f:.47f;
            float ph=walkPhase+(i==0||i==3?0.f:PI);
            glm::mat4 hip=root*cg::translate({sx*.27f,.43f,sz})*cg::rotate_x(motion*.39f*std::sin(ph));
            r.ball(hip,{0,-.25f,0},{.15f,.26f,.16f},orange);
            r.ball(hip,{0,-.42f,-.05f},{.17f,.1f,.18f},cream);
        }
        for(int i=0;i<5;i++){
            float u=i/4.f,sway=.16f*std::sin(t*2.9f+u*1.7f);
            r.ball(root,{sway*u,.71f+u*.38f,.71f+u*.49f},
                {.14f-u*.035f,.14f-u*.035f,.23f},i%2?dark:orange);
        }
        r.box(root,{0,.86f,.02f},{.51f,.075f,.31f},dark);
    }
};

struct Beta {
    glm::vec3 pos;int hp=3;bool saved=false;float alert=0;
    bool alive()const{return !saved&&hp>0;}
    void draw(const Render& r,float t)const{
        if(!alive())return;
        glm::mat4 root=cg::translate(pos)*cg::rotate_y(.18f*std::sin(t));
        const glm::vec3 jacket{.26f,.53f,.71f},skin{.89f,.69f,.53f};
        r.box(root,{0,.95f,0},{.58f,.78f,.34f},jacket);
        r.ball(root,{0,1.58f,0},{.25f,.27f,.24f},skin);
        r.ball(root,{0,1.77f,.02f},{.27f,.09f,.25f},{.22f,.19f,.16f});
        for(float s:{-1.f,1.f}){
          r.box(root,{s*.17f,.30f,0},{.17f,.60f,.22f},{.19f,.23f,.30f});
          r.box(root,{s*.39f,1.f,0},{.16f,.59f,.18f},skin);
        }
        glm::vec3 beacon=alert>0?glm::vec3(1,.14f,.13f):glm::vec3(.2f,1.f,.57f);
        r.ball(glm::mat4(1),pos+glm::vec3(0,2.25f+.1f*std::sin(t*3),0),{.14f,.14f,.14f},beacon);
    }
};
struct Chad {
    glm::vec3 pos,origin,patrolA,patrolB;float speed=1.8f,phase=0,heading=0,cooldown=0;
    bool towardB=true;int hp=2;bool alive()const{return hp>0;}
    void draw(const Render& r,float t)const{
        if(!alive())return;
        glm::mat4 root=cg::translate(pos)*cg::rotate_y(heading);
        glm::vec3 shirt{.30f,.26f,.37f},skin{.87f,.56f,.35f},jeans{.18f,.20f,.29f};
        float bob=.04f*std::sin(phase*2.f);
        r.box(root,{0,1.43f+bob,0},{.77f,.82f,.40f},shirt);
        r.ball(root,{0,2.14f+bob,-.05f},{.29f,.30f,.28f},skin);
        r.box(root,{0,2.37f+bob,-.02f},{.60f,.13f,.56f},{.14f,.10f,.11f});
        r.box(root,{0,2.14f+bob,-.30f},{.47f,.09f,.10f},{.06f,.08f,.12f});
        for(float side:{-1.f,1.f}){
            float kick=std::sin(phase+(side>0?PI:0));
            auto leg=root*cg::translate({side*.21f,.99f,0})*cg::rotate_x(kick*.35f);
            r.box(leg,{0,-.38f,0},{.29f,.73f,.34f},jeans);
            r.box(leg,{0,-.80f,-.08f},{.35f,.17f,.46f},{.08f,.09f,.12f});
            auto arm=root*cg::translate({side*.49f,1.82f,0})*cg::rotate_x(-kick*.32f);
            r.box(arm,{0,-.35f,0},{.24f,.73f,.24f},skin);
        }
        r.ball(glm::mat4(1),pos+glm::vec3(0,2.91f+.09f*std::sin(t*3),0),
          {.13f,.13f,.13f},{1.f,.12f,.12f});
    }
};
struct Drone {
    float u=0.15f,speed=.13f,clock=.2f;
    glm::vec3 pos={-4,4.1f,-12};
    void update(float dt){
        clock+=dt*speed;
        // Cosine easing gives zero velocity at each endpoint, including the turnaround.
        u=.5f-.5f*std::cos(2.f*PI*clock);
        const glm::vec3 p0{-5,4.2f,-12},p1{-1,6.f,-7},p2{5,5.1f,-23},p3{-3,4.7f,-25};
        float v=1-u;pos=v*v*v*p0+3*v*v*u*p1+3*v*u*u*p2+u*u*u*p3;
    }
    void draw(const Render& r,float t)const{
        glm::mat4 I(1);
        r.box(I,pos,{.65f,.24f,.52f},{.35f,.43f,.48f});
        for(float s:{-1.f,1.f}){
          r.box(I,pos+glm::vec3(s*.54f,.03f,0),{.60f,.07f,.08f},{.57f,.64f,.65f});
          r.ball(I,pos+glm::vec3(s*.78f,.08f,0),{.28f,.025f,.28f},{.12f,.14f,.17f});
        }
        r.ball(I,pos+glm::vec3(0,-.17f,-.26f),{.18f,.14f,.15f},
          {.94f,.13f+.12f*std::sin(t*7),.12f});
    }
};
struct Projectile {glm::vec3 pos,velocity;float life=2.f;};
struct Particle {glm::vec3 pos,vel,color;float life=.4f;};

std::vector<WallAABB> makeWalls(){
    std::vector<WallAABB> w={
     {-10,-9.55f,-31,13},{9.55f,10,-31,13},
     {-10,10,-31,-30.5f},{-10,10,12.5f,13},
     {-8.9f,-2.8f,-8.3f,-7.55f},{2.6f,8.95f,-19.5f,-18.85f},
     {-1.6f,1.6f,-27.2f,-26.35f},
    };
    for(float z:{6.f,-.5f,-13.f,-22.f})for(float x:{-6.5f,6.5f})
       w.push_back({x-.4f,x+.4f,z-.4f,z+.4f});
    for(float x:{-4.5f,4.5f})w.push_back({x-.85f,x+.85f,-2.3f,-1.9f});
    return w;
}
void drawEnvironment(const Render& r){
    glm::mat4 I(1);
    r.box(I,{0,-.34f,-9},{23,.65f,73},{.47f,.49f,.51f},0,0,0,r.groundTex,.45f);
    r.box(I,{0,-.012f,-9},{17,.035f,72},{.46f,.49f,.50f},0,0,0,r.metalTex,.25f);
    // Three-dimensional brick campus facades, with side passages and skylights.
    for(float side:{-1.f,1.f}){
        r.box(I,{side*10.5f,3.0f,-9},{1.75f,6.f,72},{.63f,.54f,.53f},0,0,0,r.brickTex,.5f);
        r.box(I,{side*9.65f,3.f,-9},{.19f,5.8f,71},{.30f,.30f,.36f},0,0,0,r.metalTex,.25f);
        for(int k=0;k<11;k++){
            float z=9.5f-k*3.6f;
            for(int level=0;level<2;level++){
               float y=1.65f+level*2.5f;
               r.box(I,{side*9.53f,y,z},{.045f,1.1f,1.8f},{.095f,.17f,.20f});
               r.box(I,{side*9.51f,y,z},{.055f,.045f,1.85f},{.39f,.32f,.29f});
            }
        }
        for(float z:{6.f,-.5f,-13.f,-22.f}){
           r.box(I,{side*6.5f,2.25f,z},{.8f,4.5f,.8f},{.40f,.43f,.48f},0,0,0,r.metalTex,.25f);
           r.box(I,{side*6.5f,4.5f,z},{1.2f,.35f,1.1f},{.22f,.26f,.30f});
           r.box(I,{side*6.5f,2.9f,z+.47f},{.42f,.48f,.11f},{.68f,.14f,.13f});
        }
    }
    for(float z:{6.f,-.5f,-13.f,-22.f}){
        r.box(I,{0,4.5f,z},{14.f,.35f,.95f},{.25f,.29f,.33f},0,0,0,r.metalTex,.25f);
        r.box(I,{0,4.22f,z+.37f},{2.5f,.08f,.15f},{.84f,.27f,.12f});
    }
    // Zig-zag barricades, forcing a real FPS navigation path.
    r.box(I,{-5.85f,1.3f,-7.93f},{6.1f,2.6f,.75f},{.52f,.46f,.46f},0,0,0,r.brickTex,.5f);
    r.box(I,{5.78f,1.3f,-19.18f},{6.3f,2.6f,.65f},{.44f,.43f,.44f},0,0,0,r.brickTex,.5f);
    r.box(I,{0,.85f,-26.75f},{3.2f,1.7f,.80f},{.45f,.42f,.39f},0,0,0,r.metalTex,.25f);
    r.box(I,{0,4.0f,-30.5f},{20.f,1.f,.9f},{.20f,.25f,.28f});
    // Ambient safety lights, hazard markings and ground props.
    for(float z:{3.f,-6.f,-15.f,-24.f}){
       r.box(I,{-.1f,.045f,z},{2.6f,.025f,.24f},{.78f,.54f,.12f});
       for(float side:{-1.f,1.f}){
          r.box(I,{side*8.7f,2.6f,z},{.17f,.45f,.35f},{.85f,.10f,.08f});
       }
    }
    for(float s:{-1.f,1.f}){
       r.box(I,{s*4.5f,.4f,-2.1f},{1.7f,.18f,.55f},{.32f,.23f,.18f});
       r.box(I,{s*4.5f,.20f,-2.25f},{1.5f,.40f,.15f},{.28f,.23f,.20f});
       for(float z:{-12.f,-23.5f}){
           r.box(I,{s*8.0f,.55f,z},{1.1f,1.1f,1.0f},{.34f,.40f,.44f},0,.19f,0,r.metalTex,.25f);
       }
    }
    // Surviving holographic beacon over the main objective.
    r.box(I,{0,3.35f,-28.8f},{5.1f,.60f,.30f},{.24f,.58f,.43f});
}

struct Game {
  Cat cat;std::vector<Chad> chads;std::vector<Beta> betas;
  Drone drone;std::vector<Projectile> shots;std::vector<Particle> particles;
  std::vector<WallAABB> walls=makeWalls();
  int life=9,saved=0;float time=0,attackCooldown=0,sonicCooldown=0,attackVisual=0,damageVisual=0;
  bool won=false,lost=false;unsigned int particleSeed=713;
  Game(){reset();}
  void reset(){
    cat=Cat{};life=9;saved=0;time=0;attackCooldown=0;sonicCooldown=0;
    attackVisual=0;damageVisual=0;won=lost=false;drone=Drone{};shots.clear();particles.clear();
    betas={{{3.8f,0,-6.5f},3,false,0},{{-3.4f,0,-16.f},3,false,0},{{3.5f,0,-24.f},3,false,0}};
    chads={
      {{.2f,0,-5.1f},{.2f,0,-5.1f},{-2.f,0,-5.1f},{2.1f,0,-10.f},1.65f},
      {{-2.4f,0,-13.4f},{-2.4f,0,-13.4f},{-2.4f,0,-13.4f},{1.f,0,-16.f},1.7f},
      {{1.3f,0,-22.f},{1.3f,0,-22.f},{1.3f,0,-22.f},{-2.f,0,-25.f},1.8f}
    };
    // Grace period so every Betinha can be reached before the first attack.
    for(auto& ch:chads)ch.cooldown=4.0f;
  }
  void burst(glm::vec3 p,glm::vec3 color){
    for(int i=0;i<15;i++){
      particleSeed=particleSeed*1664525u+1013904223u;
      float a=(particleSeed%6283)*.001f;
      particleSeed=particleSeed*1664525u+1013904223u;
      float s=.8f+(particleSeed%2000)*.001f;
      particles.push_back({p,{std::sin(a)*s,1.0f+(.23f*i),std::cos(a)*s},color,.46f});
    }
  }
  bool ended()const{return won||lost;}
  int deadBetas()const{int n=0;for(auto& b:betas)if(b.hp<=0)n++;return n;}
  int aliveChads()const{int n=0;for(auto& c:chads)if(c.alive())n++;return n;}
  bool nearRescue()const{for(auto& b:betas)if(b.alive()&&glm::distance(cat.pos,b.pos)<1.9f)return true;return false;}
  void rescue(){
    if(ended())return;
    for(auto& b:betas)if(b.alive()&&glm::distance(cat.pos,b.pos)<1.9f){
      b.saved=true;saved++;burst(b.pos+glm::vec3(0,1.5f,0),{.2f,1.f,.56f});break;
    }
    if(saved>=2)won=true;
  }
  void pawAttack(float yaw){
    if(ended()||attackCooldown>0)return;
    attackCooldown=.43f;attackVisual=.24f;
    glm::vec3 front=cg::forward(yaw);
    for(auto& ch:chads){
      if(!ch.alive())continue;
      glm::vec3 dif=ch.pos-cat.pos;dif.y=0;float d=glm::length(dif);
      if(d<2.3f && d>.01f && glm::dot(front,dif/d)>.40f){
        ch.hp--;burst(ch.pos+glm::vec3(0,1.5f,0),{1.f,.35f,.11f});break;
      }
    }
  }
  void miau(float yaw,float pitch){
    if(ended()||sonicCooldown>0)return;
    sonicCooldown=.62f;attackVisual=.22f;
    glm::vec3 d=cg::forward(yaw,pitch);
    shots.push_back({cat.pos+glm::vec3(0,.78f,0)+d*.52f,d*17.f,1.8f});
  }
  void update(float dt,glm::vec3 move,bool sprint,float yaw){
    time+=dt;attackCooldown=std::max(0.f,attackCooldown-dt);
    sonicCooldown=std::max(0.f,sonicCooldown-dt);
    attackVisual=std::max(0.f,attackVisual-dt);
    damageVisual=std::max(0.f,damageVisual-dt);
    if(ended())return;
    if(glm::length(move)>.01f){
       move=glm::normalize(move);float speed=sprint?6.2f:3.75f;
       cat.pos=slideAgainstWalls(cat.pos,move*speed*dt,.35f,walls);
       cat.updateAnimation(speed,dt);
    }else cat.updateAnimation(0,dt);
    cat.yaw=yaw;
    drone.update(dt);
    for(auto& b:betas)b.alert=std::max(0.f,b.alert-dt);
    for(auto& ch:chads){
      if(!ch.alive())continue;
      ch.cooldown=std::max(0.f,ch.cooldown-dt);
      glm::vec3 target=ch.towardB?ch.patrolB:ch.patrolA;
      float distanceToTarget=1000.f;
      bool targetingBeta=false;int targetBeta=-1;
      for(size_t k=0;k<betas.size();k++){
        if(!betas[k].alive())continue;
        float dist=glm::distance(ch.pos,betas[k].pos);
        if(dist<9.2f && dist<distanceToTarget){distanceToTarget=dist;target=betas[k].pos;targetBeta=(int)k;targetingBeta=true;}
      }
      float distPlayer=glm::distance(ch.pos,cat.pos);
      if(distPlayer<6.f && (!targetingBeta || distPlayer<distanceToTarget*.8f)){
        target=cat.pos;targetingBeta=false;targetBeta=-1;distanceToTarget=distPlayer;
      }
      glm::vec3 delta=target-ch.pos;delta.y=0;
      float dst=glm::length(delta);
      if(!targetingBeta && targetBeta<0 && dst<.23f && distPlayer>6.f){
        ch.towardB=!ch.towardB;target=ch.towardB?ch.patrolB:ch.patrolA;
        delta=target-ch.pos;delta.y=0;dst=glm::length(delta);
      }
      if(dst>1.20f){
        glm::vec3 direction=delta/dst;
        glm::vec3 old=ch.pos;
        ch.pos=slideAgainstWalls(ch.pos,direction*ch.speed*dt,.40f,walls);
        // If trapped at a barricade, move laterally to avoid permanent blocking.
        if(glm::distance(ch.pos,old)<.0002f){
           glm::vec3 aside{-direction.z,0,direction.x};
           ch.pos=slideAgainstWalls(ch.pos,aside*ch.speed*dt,.40f,walls);
        }
        ch.heading=std::atan2(-direction.x,-direction.z);ch.phase+=dt*5.3f;
      }else if(ch.cooldown<=0){
        ch.cooldown=2.0f;
        if(targetingBeta && targetBeta>=0 && betas[targetBeta].alive()){
          betas[targetBeta].hp--;betas[targetBeta].alert=1.f;
          burst(betas[targetBeta].pos+glm::vec3(0,1.f,0),{1.f,.16f,.12f});
        } else if(distPlayer<1.5f){life--;damageVisual=.55f;}
      }
    }
    for(auto& shot:shots){
      if(shot.life<=0)continue;
      shot.pos+=shot.velocity*dt;shot.life-=dt;
      if(collidesWithMap(shot.pos,.15f,walls)){burst(shot.pos,{.45f,.72f,1.f});shot.life=0;continue;}
      for(auto& ch:chads)if(ch.alive()){
        glm::vec3 target=ch.pos+glm::vec3(0,1.35f,0);
        if(glm::distance(shot.pos,target)<.95f){
          ch.hp--;burst(shot.pos,{.85f,.54f,1.f});shot.life=0;break;
        }
      }
    }
    shots.erase(std::remove_if(shots.begin(),shots.end(),[](const Projectile& p){return p.life<=0;}),shots.end());
    for(auto& p:particles){p.pos+=p.vel*dt;p.vel.y-=7.f*dt;p.life-=dt;}
    particles.erase(std::remove_if(particles.begin(),particles.end(),[](const Particle& p){return p.life<=0;}),particles.end());
    if(deadBetas()>1||life<=0)lost=true;
    if(saved>=2)won=true;
  }
  void draw(const Render& r)const{
    drawEnvironment(r);
    drone.draw(r,time);
    for(auto& b:betas)b.draw(r,time);
    for(auto& ch:chads)ch.draw(r,time);
    for(auto& s:shots)r.ball(glm::mat4(1),s.pos,{.20f,.20f,.20f},{.47f,.76f,1.f});
    for(auto& p:particles)r.ball(glm::mat4(1),p.pos,{.07f,.07f,.07f},p.color);
  }
};

struct Input {
  std::array<bool,GLFW_KEY_LAST+1> keys{};
  bool fps=true,mouseCaptured=true,firstMouse=true,attack=false,sonic=false,rescue=false;
  float yaw=0,pitch=0,orbitYaw=0,orbitPitch=.17f,orbitDist=3.7f;
  double lastX=0,lastY=0;
  Game game;
};
void key_cb(GLFWwindow* w,int k,int,int action,int){
  auto* s=static_cast<Input*>(glfwGetWindowUserPointer(w));
  if(k>=0&&k<=GLFW_KEY_LAST)s->keys[k]=action!=GLFW_RELEASE;
  if(action!=GLFW_PRESS)return;
  if(k==GLFW_KEY_TAB){
    s->fps=!s->fps;s->firstMouse=true;
    if(s->fps){s->yaw=s->game.cat.yaw;s->pitch=0;}
    else s->orbitYaw=s->yaw;
  }
  if(k==GLFW_KEY_R){s->game.reset();s->yaw=s->orbitYaw=0;s->pitch=0;}
  if(k==GLFW_KEY_E)s->rescue=true;
  if(k==GLFW_KEY_ESCAPE){
    s->mouseCaptured=!s->mouseCaptured;s->firstMouse=true;
    glfwSetInputMode(w,GLFW_CURSOR,s->mouseCaptured?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);
  }
  if(k==GLFW_KEY_Q)glfwSetWindowShouldClose(w,GL_TRUE);
}
void mouse_cb(GLFWwindow* w,double x,double y){
 auto* s=static_cast<Input*>(glfwGetWindowUserPointer(w));
 if(!s->mouseCaptured){s->lastX=x;s->lastY=y;return;}
 if(s->firstMouse){s->lastX=x;s->lastY=y;s->firstMouse=false;return;}
 float dx=static_cast<float>(x-s->lastX),dy=static_cast<float>(y-s->lastY);
 s->lastX=x;s->lastY=y;
 if(s->fps){s->yaw-=dx*.0028f;s->pitch=clamp(s->pitch-dy*.0024f,-.85f,.85f);}
 else {s->orbitYaw-=dx*.0035f;s->orbitPitch=clamp(s->orbitPitch-dy*.003f,-.50f,.95f);}
}
void button_cb(GLFWwindow* w,int button,int action,int){
 auto* s=static_cast<Input*>(glfwGetWindowUserPointer(w));
 if(action!=GLFW_PRESS)return;
 if(!s->mouseCaptured){s->mouseCaptured=true;s->firstMouse=true;glfwSetInputMode(w,GLFW_CURSOR,GLFW_CURSOR_DISABLED);return;}
 if(button==GLFW_MOUSE_BUTTON_LEFT)s->attack=true;
 if(button==GLFW_MOUSE_BUTTON_RIGHT)s->sonic=true;
}
void scroll_cb(GLFWwindow* w,double,double dy){
 auto* s=static_cast<Input*>(glfwGetWindowUserPointer(w));
 if(!s->fps)s->orbitDist=clamp(s->orbitDist-static_cast<float>(dy)*.35f,2.4f,6.5f);
}

struct PixelFramebuffer {
 GLuint framebuffer=0,color=0,depth=0;
 bool init(){
   glGenFramebuffers(1,&framebuffer);glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
   glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,Overlay::W,Overlay::H,0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
   glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);
   glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);
   glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,Overlay::W,Overlay::H);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
   bool ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
   glBindFramebuffer(GL_FRAMEBUFFER,0);return ok;
 }
 void destroy(){if(depth)glDeleteRenderbuffers(1,&depth);if(color)glDeleteTextures(1,&color);if(framebuffer)glDeleteFramebuffers(1,&framebuffer);}
};

// Viewmodel 2D para o modo FPS: APENAS o topo da cabeca e as orelhas.
// O corpo 3D completo nunca e renderizado nesta camera (veja o game loop).
// Todo o desenho fica diretamente acima do retrato TEO no HUD [273..365],
// sem ocupar a area central de combate ou bloquear a mira.
void drawFirstPersonCat(Overlay& o,const Cat& cat,float attackVisual){
    constexpr float hudTop=293.f;
    float bob=1.1f*std::sin(cat.walkPhase*2.f)*cat.motion;

    // As orelhas sao pequenas (aprox. 24 px) e ficam coladas ao HUD.
    o.tri(288,hudTop,296,267+bob,311,hudTop,.69f,.27f,.07f);
    o.tri(292,hudTop-4,297,275+bob,304,hudTop-4,.95f,.55f,.37f);
    o.tri(329,hudTop,344,267+bob,352,hudTop,.69f,.27f,.07f);
    o.tri(336,hudTop-4,343,275+bob,348,hudTop-4,.95f,.55f,.37f);

    // Pequena faixa curva do couro cabeludo. O restante fica escondido pelo HUD.
    o.tri(299,285+bob,320,277+bob,340,285+bob,.88f,.35f,.085f);
    o.rect(299,285+bob,42,hudTop-(285+bob),.82f,.30f,.065f);
    o.rect(309,286+bob,22,2,.98f,.45f,.11f,.65f);

    // As patas so entram no quadro durante a animacao de ataque.
    if(attackVisual>0){
        float f=clamp(attackVisual/.24f,0.f,1.f);
        o.rect(288-f*37,208+f*16,68,49,.91f,.38f,.09f);
        o.rect(289-f*37,202+f*16,18,16,1.f,.66f,.32f);
        o.rect(312-f*37,197+f*16,18,20,1.f,.66f,.32f);
        o.rect(337-f*37,204+f*16,16,16,1.f,.66f,.32f);
    }
}

void drawHUD(Overlay& o,const Input& state){
 const Game& g=state.game;
 o.clear();
 // Main viewport letterbox and compact feline silhouette above the HUD portrait.
 o.rect(0,0,640,4,.10f,.09f,.09f);
 o.rect(0,284,640,9,.08f,.09f,.10f);
 o.rect(0,3,640,2,.65f,.29f,.12f);
 o.text(12,13,"UFRGS // INF 01",2,.95f,.62f,.32f);
 o.text(508,13,state.fps?"CAT EYE":"CAT CHASE",2,.82f,.90f,.79f);
 if(state.fps)drawFirstPersonCat(o,g.cat,g.attackVisual);
 // Crosshair.
 o.rect(319,140,2,12,.97f,.87f,.65f);
 o.rect(314,145,12,2,.97f,.87f,.65f);
 // Chunky Doom-like HUD, divided into bolted metal panels.
 o.rect(0,293,640,67,.12f,.12f,.13f);
 o.rect(0,294,640,5,.48f,.45f,.37f);
 o.rect(4,301,632,55,.24f,.23f,.22f);
 for(int x:{126,253,384,513})o.rect(x,300,3,56,.08f,.09f,.10f);
 for(int x:{8,124,250,380,510,627}){
   o.rect(x,302,4,4,.58f,.57f,.51f);o.rect(x+1,303,2,2,.14f,.14f,.15f);
 }
 o.text(22,306,"VIDAS",2,.73f,.71f,.64f);
 o.text(31,324,std::to_string(std::max(0,g.life)),4,1.f,.39f,.28f);
 o.text(143,306,"BETINHAS",2,.80f,.77f,.70f);
 o.text(160,325,std::to_string(g.saved)+"/2",3,.98f,.83f,.31f);
 // Pixel-art cat portrait in the center, changing color on damage.
 o.rect(273,301,92,55,.12f,.11f,.12f);
 float rr=g.damageVisual>0?.94f:.96f,gg=g.damageVisual>0?.14f:.47f;
 o.rect(297,315,44,31,rr,gg,.12f);
 o.tri(296,317,295,306,311,315,rr,gg,.12f);
 o.tri(326,315,342,306,342,317,rr,gg,.12f);
 o.rect(305,328,6,5,.09f,.12f,.13f);o.rect(327,328,6,5,.09f,.12f,.13f);
 o.rect(319,335,5,4,.95f,.66f,.72f);
 o.text(277,348,"TEO",1,.93f,.80f,.65f);
 o.text(395,306,"CHADS",2,.74f,.71f,.65f);
 o.text(417,325,std::to_string(g.aliveChads()),3,1.f,.35f,.28f);
 o.text(533,306,"MIAU",2,.75f,.72f,.63f);
 o.text(542,325,g.sonicCooldown>0?"...":"OK",3,.39f,.83f,.95f);
 if(g.nearRescue()&&!g.ended()){
   o.rect(218,240,208,25,.12f,.15f,.13f,.88f);
   o.text(231,246,"E  RESGATAR",2,.64f,1.f,.63f);
 }
 for(auto& b:g.betas)if(b.alive()&&b.alert>0){
   o.rect(194,42,252,24,.39f,.06f,.06f,.88f);
   o.text(203,48,"BETINHA SOB ATAQUE!",2,1.f,.74f,.62f);break;
 }
 if(g.damageVisual>0){
   float a=clamp(g.damageVisual*.50f,0,.23f);
   o.rect(0,0,640,284,.72f,.07f,.05f,a);
 }
 if(g.ended()){
   o.rect(72,87,496,103,.05f,.06f,.08f,.93f);
   o.rect(72,87,496,5,g.won?.18f:.75f,g.won?.65f:.12f,.14f);
   o.text(g.won?202:212,107,g.won?"VITORIA!":"DERROTA!",4,g.won?.40f:1.f,g.won?1.f:.32f,.20f);
   o.text(168,159,"R  REINICIAR  /  Q  SAIR",2,.88f,.85f,.71f);
 }
 if(g.time<7.f&&!g.ended()){
   o.rect(146,64,348,27,.10f,.12f,.15f,.78f);
   o.text(157,72,"SALVE 2 BETINHAS DOS CHADS!",2,1.f,.79f,.32f);
 }
 o.flush();
}

void capturePPM(const std::string& filename){
 std::vector<unsigned char> img(Overlay::W*Overlay::H*3);
 glReadPixels(0,0,Overlay::W,Overlay::H,GL_RGB,GL_UNSIGNED_BYTE,img.data());
 std::ofstream f(filename,std::ios::binary);
 if(!f)return;
 f<<"P6\n"<<Overlay::W<<" "<<Overlay::H<<"\n255\n";
 for(int y=Overlay::H-1;y>=0;y--)f.write(reinterpret_cast<char*>(img.data()+y*Overlay::W*3),Overlay::W*3);
}
} // namespace

int main(int argc,char**argv){
 int maxFrames=0;std::string capture;bool demo=false,thirdPerson=false,selfTest=false;
 for(int i=1;i<argc;i++){
    std::string a=argv[i];
    if(a=="--frames"&&i+1<argc)maxFrames=std::max(1,std::atoi(argv[++i]));
    if(a=="--capture"&&i+1<argc)capture=argv[++i];
    if(a=="--demo")demo=true;
    if(a=="--third-person")thirdPerson=true;
    if(a=="--self-test")selfTest=true;
 }
 if(selfTest){
   Game test;
   if(!circleIntersectsWall({9.7f,0,0},.4f,test.walls[1]))return 41;
   auto slide=slideAgainstWalls({9.2f,0,0},{4.f,0,0},.35f,test.walls);
   if(slide.x>9.30f)return 42;
   test.cat.pos=test.betas[0].pos;test.rescue();
   test.cat.pos=test.betas[1].pos;test.rescue();
   if(test.saved!=2||!test.won)return 43;
   Drone d;d.update(.01f);if(d.pos.y<3.f||d.pos.y>7.f)return 44;
   std::cout<<"OK: colisoes, resgates, condicao de vitoria e Bezier.\n";
   return 0;
 }
 if(!glfwInit()){std::cerr<<"GLFW sem suporte a video/driver.\n";return 1;}
 glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
 glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
 GLFWwindow* window=glfwCreateWindow(WIDTH,HEIGHT,"APOCALIPTIC KITTENS | INF DOOM PROTOTYPE",nullptr,nullptr);
 if(!window){std::cerr<<"Falha ao abrir janela OpenGL.\n";glfwTerminate();return 2;}
 glfwMakeContextCurrent(window);glfwSwapInterval(1);
 if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
   std::cerr<<"GLAD indisponivel\n";glfwDestroyWindow(window);glfwTerminate();return 3;
 }
 glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);
 Input st;st.fps=!thirdPerson;glfwSetWindowUserPointer(window,&st);
 glfwSetKeyCallback(window,key_cb);glfwSetCursorPosCallback(window,mouse_cb);
 glfwSetMouseButtonCallback(window,button_cb);glfwSetScrollCallback(window,scroll_cb);
 glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
 Render renderer;Overlay hud;PixelFramebuffer lowRes;
 int result=0;
 if(!renderer.init()||!hud.init()||!lowRes.init())result=4;
 else{
   double before=glfwGetTime();int frame=0;float accumulator=0;
   while(!glfwWindowShouldClose(window)){
     glfwPollEvents();
     double now=glfwGetTime();float elapsed=clamp(static_cast<float>(now-before),0,.10f);
     before=now;accumulator=std::min(accumulator+elapsed,.20f);
     if(st.attack){st.game.pawAttack(st.fps?st.yaw:st.game.cat.yaw);st.attack=false;}
     if(st.sonic){st.game.miau(st.fps?st.yaw:st.game.cat.yaw,st.fps?st.pitch:0);st.sonic=false;}
     if(st.rescue){st.game.rescue();st.rescue=false;}
     const auto key=[&](int k){return st.keys[k];};
     constexpr float fixedStep=1.f/60.f;
     for(int j=0;accumulator>=fixedStep&&j<6;j++,accumulator-=fixedStep){
       float movementYaw=st.fps?st.yaw:st.orbitYaw;
       glm::vec3 fw=cg::forward(movementYaw),right{std::cos(movementYaw),0,-std::sin(movementYaw)};
       glm::vec3 move(0);
       if(key(GLFW_KEY_W))move+=glm::vec3(fw.x,0,fw.z);
       if(key(GLFW_KEY_S))move-=glm::vec3(fw.x,0,fw.z);
       if(key(GLFW_KEY_D))move+=right;
       if(key(GLFW_KEY_A))move-=right;
       if(demo&&glm::length(move)<.01f)move=glm::vec3(0,0,-1);
       float facing=st.yaw;
       if(!st.fps&&glm::length(move)>.02f)facing=std::atan2(-move.x,-move.z);
       st.game.update(fixedStep,move,key(GLFW_KEY_LEFT_SHIFT)||key(GLFW_KEY_RIGHT_SHIFT),facing);
     }
     glm::vec3 camera,focus;
     if(st.fps){
       camera=st.game.cat.pos+glm::vec3(0,.73f,0)+cg::forward(st.yaw)*.56f;
       focus=camera+cg::forward(st.yaw,st.pitch)*6.f;
     }else{
       focus=st.game.cat.pos+glm::vec3(0,.67f,0);
       glm::vec3 direction=cg::forward(st.orbitYaw,st.orbitPitch);
       camera=focus-direction*st.orbitDist+glm::vec3(0,.18f,0);
     }
     glBindFramebuffer(GL_FRAMEBUFFER,lowRes.framebuffer);
     glViewport(0,0,Overlay::W,Overlay::H);
     glClearColor(.075f,.083f,.103f,1);
     glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
     renderer.begin(cg::view(camera,focus),cg::perspective(83.f*PI/180.f,(float)Overlay::W/Overlay::H),camera);
     st.game.draw(renderer);
     // FPS: a Teodora existe fisicamente no mundo, mas seu modelo completo
     // nao e desenhado. Somente as orelhas do overlay FPS aparecem no HUD.
     // Terceira pessoa: renderiza a gata integralmente.
     if(!st.fps)st.game.cat.draw(renderer,st.game.time);
     drawHUD(hud,st);
     if(!capture.empty()&&maxFrames>0&&frame+1==maxFrames)capturePPM(capture);
     int w=WIDTH,h=HEIGHT;glfwGetFramebufferSize(window,&w,&h);
     glBindFramebuffer(GL_READ_FRAMEBUFFER,lowRes.framebuffer);
     glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
     glBlitFramebuffer(0,0,Overlay::W,Overlay::H,0,0,w,h,GL_COLOR_BUFFER_BIT,GL_NEAREST);
     glBindFramebuffer(GL_FRAMEBUFFER,0);
     glfwSwapBuffers(window);
     if(maxFrames>0&&++frame>=maxFrames)break;
   }
 }
 lowRes.destroy();hud.destroy();renderer.destroy();glfwDestroyWindow(window);glfwTerminate();return result;
}
