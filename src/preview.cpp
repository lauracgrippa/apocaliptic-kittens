// Apocaliptic Kittens — visual prototype, no game engine.
// Based on the GLFW/GLAD/GLM/tinyobj stack supplied in the UFRGS course ZIP.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>
#include <stb_image.h>
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
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    stbi_image_free(pix);return t;
}

struct Render {
    GLuint program=0, groundTex=0, brickTex=0;
    Mesh cube, sphere;
    glm::vec3 camPos;
    GLint uModel=-1,uColor=-1,uTextured=-1,uTile=-1;
    bool init() {
        program=create_program(); if(!program) return false;
        if(!cube.load("../../data/cube.obj") || !sphere.load("../../data/sphere.obj"))return false;
        groundTex=load_texture("../../data/rocky_terrain_02_diff_1k.jpg");
        brickTex=load_texture("../../data/red_brick_diff_1k.jpg");
        glUseProgram(program);
        uModel=glGetUniformLocation(program,"uModel");
        uColor=glGetUniformLocation(program,"uColor");
        uTextured=glGetUniformLocation(program,"uTextured");
        uTile=glGetUniformLocation(program,"uTile");
        glUniform1i(glGetUniformLocation(program,"uTexture"),0);
        glUniform3f(glGetUniformLocation(program,"uSunDirection"),-.5f,1.0f,.35f);
        return true;
    }
    void begin(const glm::mat4& view,const glm::mat4& projection,const glm::vec3& camera) {
        camPos=camera;glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program,"uView"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program,"uProjection"),1,GL_FALSE,glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(program,"uCameraPosition"),1,glm::value_ptr(camPos));
    }
    void draw(const Mesh& mesh,const glm::mat4& transform,glm::vec3 color, GLuint texture=0,float tiles=1) const {
        glUniformMatrix4fv(uModel,1,GL_FALSE,glm::value_ptr(transform));
        glUniform3fv(uColor,1,glm::value_ptr(color));
        glUniform1i(uTextured,texture!=0);glUniform1f(uTile,tiles);
        glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture);
        mesh.draw();
    }
    void box(glm::mat4 parent,glm::vec3 local,glm::vec3 size,glm::vec3 color,float rx=0,float ry=0,float rz=0,
             GLuint texture=0,float tiles=1) const {
        // The supplied UFRGS cube OBJ has Y from -1 to 0: offset +0.5 to center it.
        glm::mat4 m=glm::translate(parent,local);
        m=glm::rotate(m,rx,glm::vec3(1,0,0));
        m=glm::rotate(m,ry,glm::vec3(0,1,0));
        m=glm::rotate(m,rz,glm::vec3(0,0,1));
        m=glm::scale(m,size);
        m=glm::translate(m,glm::vec3(0,.5f,0));
        draw(cube,m,color,texture,tiles);
    }
    void ball(glm::mat4 parent,glm::vec3 local,glm::vec3 size,glm::vec3 color) const {
        auto m=glm::scale(glm::translate(parent,local),size);
        draw(sphere,m,color);
    }
    void destroy() { cube.destroy();sphere.destroy();if(groundTex)glDeleteTextures(1,&groundTex);
        if(brickTex)glDeleteTextures(1,&brickTex);if(program)glDeleteProgram(program); }
};

// Diferença angular no intervalo [-pi, pi], para girar pelo menor caminho.
static float wrap_angle(float a) {
    const float PI=3.14159265f;
    while(a>PI)a-=2*PI;
    while(a<-PI)a+=2*PI;
    return a;
}

struct Cat {
    static constexpr float TURN_SPEED=9.0f; // rad/s ao virar para a direção do movimento
    glm::vec3 pos={0,0,7};
    float yaw=0, walkPhase=0, motion=0;
    // faceMovement: se true, gira suavemente para a direção de 'input';
    // se false, mantém o yaw atual (ex.: andar de ré na câmera fixa).
    void update(glm::vec3 input,float dt,bool run,bool faceMovement=true,float speedScale=1.0f) {
        float length=glm::length(input);
        if(length>.0001f) {
            glm::vec3 d=input/length;
            if(faceMovement) {
                float step=TURN_SPEED*dt;
                yaw=wrap_angle(yaw+clamp(wrap_angle(std::atan2(-d.x,-d.z)-yaw),-step,step));
            }
            pos+=d*(run?6.0f:3.3f)*speedScale*dt;
            pos.x=clamp(pos.x,-10.5f,10.5f);
            pos.z=clamp(pos.z,-26.f,12.f);
            walkPhase+=(run?13.0f:8.0f)*dt;
            motion=std::min(1.0f,motion+dt*8.0f);
        } else motion=std::max(0.0f,motion-dt*10.0f);
    }
    void draw(const Render& r,float t) const {
        const glm::vec3 orange{.99f,.39f,.08f}, dark{.30f,.16f,.13f},cream{1.0f,.81f,.52f};
        float bob=.035f*std::sin(walkPhase*2)*motion;
        glm::mat4 root=glm::translate(glm::mat4(1),pos+glm::vec3(0,bob,0));
        root=glm::rotate(root,yaw,glm::vec3(0,1,0));
        r.ball(root,{0,.56f,.08f},{.42f,.34f,.66f},orange); // body
        r.ball(root,{0,.65f,-.55f},{.35f,.34f,.33f},orange); // face
        r.ball(root,{0,.41f,-.76f},{.19f,.13f,.13f},cream); // muzzle
        r.ball(root,{-.13f,.70f,-.84f},{.06f,.08f,.044f},dark);
        r.ball(root,{ .13f,.70f,-.84f},{.06f,.08f,.044f},dark);
        r.ball(root,{0,.47f,-.90f},{.05f,.05f,.05f},{.93f,.44f,.45f});
        for(float side: {-1.f,1.f}) {
            r.box(root,{side*.23f,1.00f,-.61f},{.18f,.28f,.14f},orange,0,0,side*.3f);
            r.ball(root,{side*.23f,.96f,-.69f},{.07f,.12f,.04f},{.97f,.64f,.52f});
        }
        for(int i=0;i<4;i++) {
            float sx=(i%2==0?-1.f:1.f),sz=(i<2?-.32f:.47f);
            float ph=walkPhase+(i==0||i==3?0.f:PI);
            glm::mat4 hip=glm::translate(root,{sx*.27f,.43f,sz});
            hip=glm::rotate(hip,motion*.39f*std::sin(ph),glm::vec3(1,0,0));
            r.ball(hip,{0,-.25f,0},{.15f,.26f,.16f},orange);
            r.ball(hip,{0,-.42f,-.05f},{.17f,.10f,.18f},cream);
        }
        for(int i=0;i<5;i++) {
            float u=i/4.f,sway=.16f*std::sin(t*2.9f+u*1.7f);
            r.ball(root,{sway*u,.71f+u*.38f,.71f+u*.49f},{.14f-u*.035f,.14f-u*.035f,.23f},i%2?dark:orange);
        }
        // A small dark harness, deliberately simple procedural geometry.
        r.box(root,{0,.86f,.02f},{.51f,.075f,.31f},dark);
    }
};

struct Chad {
    glm::vec3 pos;
    glm::vec3 a,b;
    float speed=1.0f, phase=0, heading=0;
    bool forward=true;
    void update(float dt) {
        glm::vec3 target=forward?b:a;
        glm::vec3 d=target-pos;d.y=0;
        if(glm::length(d)<.15f) {forward=!forward;target=forward?b:a;d=target-pos;}
        if(glm::length(d)>.001f) {
            auto v=glm::normalize(d);
            pos+=v*speed*dt;
            heading=std::atan2(-v.x,-v.z);
            phase+=dt*5.f;
        }
    }
    void draw(const Render& r,float t) const {
        const glm::vec3 shirt{.25f,.29f,.34f},skin{.88f,.59f,.40f},jeans{.18f,.22f,.33f};
        auto root=glm::rotate(glm::translate(glm::mat4(1),pos),heading,glm::vec3(0,1,0));
        float b=.035f*std::sin(2*phase);
        r.box(root,{0,1.43f+b,0},{.73f,.78f,.38f},shirt);
        r.ball(root,{0,2.10f+b,-.04f},{.27f,.29f,.26f},skin);
        r.box(root,{0,2.32f+b,-.01f},{.56f,.12f,.54f},{.13f,.10f,.09f}); // hair
        r.box(root,{0,2.12f+b,-.287f},{.43f,.085f,.09f},{.12f,.12f,.14f}); // sunglasses
        for(float side:{-1.f,1.f}) {
            float kick=std::sin(phase+(side>0?PI:0));
            auto leg=glm::rotate(glm::translate(root,{side*.21f,.99f,0}),kick*.25f,glm::vec3(1,0,0));
            r.box(leg,{0,-.38f,0},{.28f,.72f,.31f},jeans);
            r.box(leg,{0,-.79f,-.08f},{.33f,.17f,.46f},{.08f,.09f,.12f});
            auto arm=glm::rotate(glm::translate(root,{side*.46f,1.78f,0}),-kick*.27f,glm::vec3(1,0,0));
            r.box(arm,{0,-.35f,0},{.21f,.68f,.20f},skin);
        }
        // Red warning marker above each enemy (no combat implemented).
        r.ball(glm::mat4(1),pos+glm::vec3(0,2.85f+.08f*std::sin(t*3),0),{.14f,.14f,.14f},{1.f,.14f,.15f});
    }
};

void environment(const Render& r,float t) {
    glm::mat4 I(1);
    r.box(I,{0,-.31f,-7},{27,.6f,61},{.72f,.69f,.67f},0,0,0,r.groundTex,26);
    // Walking route and curbs.
    r.box(I,{0,.025f,-7},{12,.055f,60},{.75f,.73f,.70f},0,0,0,r.groundTex,26);
    for(float side:{-1.f,1.f}) {
        r.box(I,{side*6.25f,.12f,-7},{.35f,.25f,58},{.39f,.41f,.42f});
        r.box(I,{side*11.5f,2.25f,-7},{2.0f,4.5f,51},{.62f,.57f,.55f},0,0,0,r.brickTex,7);
        // Windows along side walls, repeated geometry from the same box OBJ.
        for(int k=0;k<12;k++) {
            float z=10.f-k*3.2f;
            for(int floor=0;floor<2;floor++)
                r.box(I,{side*10.44f,1.5f+floor*1.55f,z},{.05f,.72f,1.1f},{.17f,.25f,.34f});
        }
        for(int k=0;k<8;k++) {
            float z=10.5f-k*5.0f;
            r.box(I,{side*8.1f,.90f,z},{.23f,1.8f,.24f},{.37f,.28f,.23f});
            r.ball(I,{side*8.1f,2.55f,z},{1.35f,1.25f,1.25f},{.26f,.43f,.26f});
            r.ball(I,{side*7.7f,2.8f,z+.35f},{.95f,.85f,.9f},{.33f,.48f,.29f});
        }
    }
    // Small campus gate and benches as props.
    r.box(I,{0,3.55f,-27.8f},{13.f,.55f,.9f},{.55f,.47f,.48f});
    for(float side:{-1.f,1.f}) {
        r.box(I,{side*6.1f,1.75f,-27.8f},{.7f,3.5f,.8f},{.55f,.47f,.48f});
        for(int k=0;k<3;k++) {
            float z=5.5f-k*7;
            r.box(I,{side*5.2f,.55f,z},{1.4f,.16f,.5f},{.39f,.29f,.22f});
            r.box(I,{side*5.2f,.38f,z-.2f},{1.25f,.5f,.14f},{.25f,.24f,.23f});
        }
    }
    // Churu-like pink collectible marker; decorative only in this preview.
    r.box(I,{3.7f,.45f,-4.5f},{.22f,.9f,.20f},{.95f,.45f,.67f},0,t*.85f,.08f);
    r.ball(I,{3.7f,.95f,-4.5f},{.24f,.10f,.24f},{1.f,.76f,.77f});
}

struct Input {
    std::array<bool,GLFW_KEY_LAST+1> keys{};
    bool dragging=false,firstMouse=true,orbit=false;
    float yaw=0,pitch=.28f,dist=6.9f;
    double lastX=0,lastY=0;
    Cat cat;
    Chad chads[3] = {
      {{-3,0,-8},{-3,0,-8},{-1.0f,0,-11},1.0f},
      {{ 3,0,-13},{ 3,0,-13},{ 1,0,-8},1.2f},
      {{-1,0,-21},{-1,0,-21},{ 3,0,-22},.65f}
    };
};
void key_cb(GLFWwindow* w,int k,int,int action,int) {
    auto* state=static_cast<Input*>(glfwGetWindowUserPointer(w));
    if(k==GLFW_KEY_ESCAPE && action==GLFW_PRESS)glfwSetWindowShouldClose(w,GL_TRUE);
    if(k>=0&&k<=GLFW_KEY_LAST)state->keys[k]=(action!=GLFW_RELEASE);
    if(k==GLFW_KEY_C&&action==GLFW_PRESS) {state->orbit=!state->orbit;state->yaw=state->cat.yaw;}
    if(k==GLFW_KEY_R&&action==GLFW_PRESS) {state->cat=Cat{};state->yaw=0;}
}
void mouse_cb(GLFWwindow* w,double x,double y) {
    auto* st=static_cast<Input*>(glfwGetWindowUserPointer(w));
    if(st->dragging && !st->firstMouse) {
        st->yaw-=static_cast<float>(x-st->lastX)*.007f;
        st->pitch=clamp(st->pitch+static_cast<float>(y-st->lastY)*.005f,.12f,1.1f);
    }
    st->lastX=x;st->lastY=y;st->firstMouse=false;
}
void button_cb(GLFWwindow* w,int b,int action,int) {
    if(b==GLFW_MOUSE_BUTTON_RIGHT) {
        auto* st=static_cast<Input*>(glfwGetWindowUserPointer(w));
        st->dragging=(action==GLFW_PRESS);st->firstMouse=true;
        if(st->dragging)st->orbit=true;
    }
}
void scroll_cb(GLFWwindow* w,double,double y) {
    auto* st=static_cast<Input*>(glfwGetWindowUserPointer(w));
    st->dist=clamp(st->dist-static_cast<float>(y)*.65f,3.4f,12.f);
}
// Manual look-at matrix, consistent with the course's CG math rules.
glm::mat4 view_from(const glm::vec3& camera,const glm::vec3& focus) {
    glm::vec3 z=glm::normalize(camera-focus);
    glm::vec3 x=glm::normalize(glm::cross(glm::vec3(0,1,0),z));
    glm::vec3 y=glm::cross(z,x);
    glm::mat4 m(1);
    m[0][0]=x.x;m[1][0]=x.y;m[2][0]=x.z;m[3][0]=-glm::dot(x,camera);
    m[0][1]=y.x;m[1][1]=y.y;m[2][1]=y.z;m[3][1]=-glm::dot(y,camera);
    m[0][2]=z.x;m[1][2]=z.y;m[2][2]=z.z;m[3][2]=-glm::dot(z,camera);
    return m;
}
// Manual OpenGL perspective matrix instead of glm::perspective.
glm::mat4 perspective(float aspect) {
    float fov=58*PI/180.f,n=.1f,f=160.f;
    float s=1.0f/std::tan(fov*.5f);
    glm::mat4 p(0);
    p[0][0]=s/aspect;p[1][1]=s;p[2][2]=-(f+n)/(f-n);
    p[2][3]=-1.f;p[3][2]=-2*f*n/(f-n);
    return p;
}
void capture_ppm(const std::string& filename,int width,int height) {
    std::vector<unsigned char> img(static_cast<size_t>(width)*height*3);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,img.data());
    std::ofstream f(filename,std::ios::binary);
    if(!f) return;
    f<<"P6\n"<<width<<" "<<height<<"\n255\n";
    for(int y=height-1;y>=0;y--)
        f.write(reinterpret_cast<const char*>(img.data()+static_cast<size_t>(y)*width*3),width*3);
}
} // namespace

int main(int argc,char** argv) {
    int maxFrames=0; std::string capture; bool autoDemo=false;
    for(int i=1;i<argc;i++) {
        if(std::string(argv[i])=="--demo")autoDemo=true;
        else if(std::string(argv[i])=="--frames"&&i+1<argc)maxFrames=std::max(1,std::atoi(argv[++i]));
        else if(std::string(argv[i])=="--capture"&&i+1<argc)capture=argv[++i];
    }
    if(!glfwInit()){std::cerr<<"GLFW falhou; precisa de monitor/driver OpenGL.\n";return 1;}
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window=glfwCreateWindow(WIDTH,HEIGHT,"Apocaliptic Kittens | Preview OpenGL",nullptr,nullptr);
    if(!window){std::cerr<<"Falha ao criar janela\n";glfwTerminate();return 2;}
    glfwMakeContextCurrent(window);glfwSwapInterval(1);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cerr<<"GLAD falhou\n";glfwDestroyWindow(window);glfwTerminate();return 3;
    }
    glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);
    Input input;glfwSetWindowUserPointer(window,&input);
    glfwSetKeyCallback(window,key_cb);glfwSetCursorPosCallback(window,mouse_cb);
    glfwSetMouseButtonCallback(window,button_cb);glfwSetScrollCallback(window,scroll_cb);
    int result=0;
    {
        Render renderer;
        if(!renderer.init()){result=4;}
        else {
            double previous=glfwGetTime();int frame=0;
            while(!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                double now=glfwGetTime();float dt=clamp(static_cast<float>(now-previous),0,.05f);previous=now;
                auto active=[&](int k)->bool{return input.keys[k];};
                bool run=active(GLFW_KEY_LEFT_SHIFT)||active(GLFW_KEY_RIGHT_SHIFT);
                bool anyKey=active(GLFW_KEY_W)||active(GLFW_KEY_S)||active(GLFW_KEY_A)||active(GLFW_KEY_D);
                if(autoDemo && !anyKey) {
                    glm::vec3 dir=glm::normalize(glm::vec3(.16f*std::sin(static_cast<float>(now)*.55f),0,-1.f));
                    input.cat.update(dir,dt,run);
                } else if(!input.orbit) {
                    // Câmera fixa: A/D giram a gata (velocidade limitada), W anda
                    // para frente e S anda de ré sem virar. O movimento depende só
                    // do yaw da gata, nunca da câmera, então não há realimentação.
                    const float TURN_RATE=2.8f; // rad/s
                    float turn=(active(GLFW_KEY_A)?1.f:0.f)-(active(GLFW_KEY_D)?1.f:0.f);
                    input.cat.yaw=wrap_angle(input.cat.yaw+turn*TURN_RATE*dt);
                    glm::vec3 catForward(-std::sin(input.cat.yaw),0,-std::cos(input.cat.yaw));
                    float move=(active(GLFW_KEY_W)?1.f:0.f)-(active(GLFW_KEY_S)?1.f:0.f);
                    input.cat.update(catForward*move,dt,run,false,move<0?.6f:1.f);
                } else {
                    // Órbita: movimento relativo à câmera; a gata vira suavemente.
                    glm::vec3 forward(-std::sin(input.yaw),0,-std::cos(input.yaw));
                    glm::vec3 right(std::cos(input.yaw),0,-std::sin(input.yaw));
                    glm::vec3 dir(0);
                    if(active(GLFW_KEY_W))dir+=forward;
                    if(active(GLFW_KEY_S))dir-=forward;
                    if(active(GLFW_KEY_D))dir+=right;
                    if(active(GLFW_KEY_A))dir-=right;
                    input.cat.update(dir,dt,run);
                }
                for(auto& ch:input.chads)ch.update(dt);
                // Câmera fixa segue o yaw da gata com suavização exponencial.
                if(!input.orbit)
                    input.yaw=wrap_angle(input.yaw+wrap_angle(input.cat.yaw-input.yaw)*(1.f-std::exp(-6.f*dt)));
                glm::vec3 focus=input.cat.pos+glm::vec3(0,.68f,0);
                glm::vec3 camera=focus+glm::vec3(
                    input.dist*std::cos(input.pitch)*std::sin(input.yaw),
                    input.dist*std::sin(input.pitch)+1.1f,
                    input.dist*std::cos(input.pitch)*std::cos(input.yaw));
                glClearColor(.47f,.43f,.53f,1);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                int w=WIDTH,h=HEIGHT;glfwGetFramebufferSize(window,&w,&h);
                if(h>0&&w>0) {
                    glViewport(0,0,w,h);
                    renderer.begin(view_from(camera,focus),perspective(static_cast<float>(w)/h),camera);
                    environment(renderer,static_cast<float>(now));
                    for(const auto& ch:input.chads)ch.draw(renderer,static_cast<float>(now));
                    input.cat.draw(renderer,static_cast<float>(now));
                    if(!capture.empty() && maxFrames>0 && frame+1==maxFrames)capture_ppm(capture,w,h);
                }
                glfwSwapBuffers(window);
                if(maxFrames>0&&++frame>=maxFrames)break;
            }
        }
        renderer.destroy();
    }
    glfwDestroyWindow(window);glfwTerminate();return result;
}
