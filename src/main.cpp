#include <iostream>

#include <fstream>
#include <sstream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "config.h"

#include "engine.h"
#include "resources/shader.h"
#include "resources/trimesh.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); 
void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y);

Config config;

const GLuint WIDTH = 800, HEIGHT = 600;

// Shaders
//const GLchar* vertexShaderSource = "#version 330 core\n"
//"layout (location = 0) in vec3 position;\n"
//"void main()\n"
//"{\n"
//"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
//"}\0";
std::string vertexShaderSource;
std::string fragmentShaderSource;

#include "linalg.h"

using namespace linalg::aliases;


const float3 origin = { 0.0, 0.0, -10 };
const float3 poi = { 0.0, 0.0, 0.0 };
const float3 up = { 0.0, 1.0, 0.0 };


constexpr float PI = 3.14159265358979f;
constexpr float DegToRad = PI / 180.0f;
constexpr float RadToDeg = 180.0f / PI;

float4x4 perspectiveMatrix(float fovy, float aspect, float zNear, float zFar) {
    float4x4 m;
    const float f = 1.0f / (tan(fovy * DegToRad / 2.0f));
    m[0] = { f / aspect, 0.0f, 0.0f, 0.0f };
    m[1] = { 0.0f, f, 0.0f, 0.0f };
    m[2] = { 0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f };
    m[3] = { 0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f };

    return m;
}

float4x4 lookAtMatrix() {

    float3 forward = normalize(poi - origin);
    float3 right = normalize(cross(forward, up));
    float3 localUp = normalize(cross(right, forward));
    
    float4x4 out;

    out[0] = { right.x, right.y, right.z, -dot(origin, right) };
    out[1] = { localUp.x, localUp.y, localUp.z, -dot(origin, localUp) };
    out[2] = { -forward.x, -forward.y, -forward.z, dot(origin, forward)};
    out[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

    return transpose(out);
}

GLFWwindow *setup_window(){
    if (!glfwInit()) {
        std::cerr << "[Init Failed] Could not start glfw." << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(config.width, config.height, "Vertex Block Descent", nullptr, nullptr);
    if (!window) {
        std::cerr << "[Init Failed] Could not make window." << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);

    // set callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "[Init Failed] Could not start glew." << std::endl;
        glfwTerminate();
        exit(-1);
    }

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    return window;
}

struct Vertex {
    float3 position;
    float3 velocity;
};

class ParticleSystem {
    std::vector<Vertex> vertices;

    TriangleMesh obj;

    float particle_scale = 100;

    void draw_particle(Vertex &v){

    }

    public:
    void draw(){

    }

};


float3 solveHouseholderQR(float3x3 H, float3 f){
    auto dot_from_k = [](float3 a, float3 b, int k){
        float acc = 0;
        
        for(int i = k; i < 3; ++i){ // row
            acc += a[i] * b[i]; 
        }

        return acc;
    };
    
    for(int k = 0; k < 3; ++k){
        float3 x = H[k];

        float3 vk = x;
        vk[k] += (x[k] > 0 ? 1 : -1) * sqrt(dot_from_k(x, x, k));
        vk /= sqrt(dot_from_k(vk, vk, k));

        // update lower right block
        for(int j = k; j < 3; ++j){ // col
            float vkTAj = dot_from_k(vk, H[j], k);

            // update R
            for(int i = k; i < 3; ++i){ // row
                H[j][i] -= 2 * vk[i] * vkTAj;
            }   
        }

        // update QTb
        float vkTb = dot_from_k(vk, f, k);
        
        for(int i = k; i < 3; ++i){ // row
            f[i] -= 2 * vk[i] * vkTb;
        }
    }
    
    // backward solve
    for(int i = 3 - 1; i >= 0; --i){
        float tmp = 0;
        for(int j = i+1; j < 3; ++j){
            tmp += H[i][j] * f[j];
        }
        f[i] = (f[i] - tmp) / H[i][i];
    }

    return f;
}

int main(int argc, char *argv[])
{

    Engine engine;

    config = parse_args(argc, argv);

    // vertexShaderSource = read_file(config.resources / "shaders" / "default.vert");
    // fragmentShaderSource = read_file(config.resources / "shaders" / "default.frag");

    GLFWwindow *window = setup_window();


    Resource<Shader> defaultShader = engine.create<Shader>(
        (config.resources / "shaders" / "default.vert"), 
        (config.resources / "shaders" / "default.frag")
    );

    // triangle mesh resource
    TriangleMesh obj((config.resources / "models" / "smallsphere.obj"));
    obj.load();

    GLuint VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, obj.positions.size() * sizeof(GLfloat), obj.positions.data(), GL_STATIC_DRAW);
        
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        obj.faces.size() * sizeof(unsigned int),
        obj.faces.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0);

    // particle system resource
    std::vector<Vertex> vertices;

    for(int i = 0; i < 10; ++ i){
        vertices.emplace_back();
        vertices[i].position = float3(i, 1.0f, i);
        vertices[i].velocity = float3(0.0f, 5.0 * i + 2.0, 0.0f);
    }

    const float timestep = 1.0/60.0;

    const double dt = 1.0 / 60.0;
    double last = glfwGetTime();
    double accumulator = 0.0;

    float3 gravity(0.0, -9.8, 0.0);

    float stiffness = 1e+5;
    float planey = -1;
    float3 planen(0.0, 1.0, 0.0); 

    float mass = 1.0;

    while (!glfwWindowShouldClose(window))
    {

        double now = glfwGetTime();
        accumulator += now - last;
        last = now;
        
        glfwPollEvents();

        while (accumulator >= timestep)
        {
            // naive physics update
            for(auto &v : vertices){
                float3 y = v.position + timestep * v.velocity + timestep * timestep * gravity;
                float3 x_new = y;

                for(int i = 0; i < 5; ++i){
                    float3 f = - mass * (x_new - y) / (timestep * timestep );
                    float3 H = float3(mass / (timestep * timestep));

                    float3 diff = float3(0.0, planey - x_new.y, 0.0);
                    float pen = dot(diff, planen);
                    if (pen > 0) {
                        f += stiffness * planen * diff;
                        H.y += stiffness;
                    }
                    float3x3 Hmat;
                    for(int i = 0; i < 3; ++i){
                        Hmat[i][i] = H[i];
                    }

                    

                    float3 dx = solveHouseholderQR(Hmat, f);
                    if(length(dx) > 1){
                        printf("length(dx)=  %f\n", length(dx));
                        printf("H = I[%f %f %f]^T\n", H.x, H.y, H.z);
                        // exit(-1);
                    }

                    // float3 dx = float3(f.x/H.x, f.y/H.y, f.z/H.z);
                    x_new += dx;
                }
                
                // pred - v.position + float3(0.0, () * (stiffness * pen), 0.0);

                // no external forces yields the following
                v.velocity = (x_new - v.position) / timestep;
                v.position = x_new;
            }

            accumulator -= timestep;
        }

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw our first triangle

        for(auto &v : vertices){
            
        float4x4 identity = linalg::identity;
        identity *= 10;
        identity.w = float4(v.position.x,v.position.y, v.position.z, 1.0);

        // glUseProgram(shaderProgram);
        defaultShader->useShader();


        const GLint modelLoc = defaultShader->getUniformLocation("model");
        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            &identity[0][0]
        );

        float4x4 viewMat = lookAtMatrix();

        const GLint viewLoc = defaultShader->getUniformLocation("view");
        glUniformMatrix4fv(
            viewLoc,
            1,
            GL_FALSE,
            &viewMat[0][0]
        );

        float globalAspectRatio = float(config.width) / float(config.height);
        float globalFOV = 45.0f; // vertical field of view
        float globalDepthMin = 5.0E-02; // for rasterization
        float globalDepthMax = 100.0f; // for rasterization

        float4x4 projMat = perspectiveMatrix(globalFOV, globalAspectRatio, globalDepthMin, globalDepthMax);

        const GLint projLoc = defaultShader->getUniformLocation("projection");
        glUniformMatrix4fv(
            projLoc,
            1,
            GL_FALSE,
            &projMat[0][0]
        );

        glBindVertexArray(VAO);
        glDrawElements(
            GL_TRIANGLES,
            obj.faces.size(),                  // number of indices
            GL_UNSIGNED_INT,
            nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
        glBindVertexArray(0);
        
        }

        glfwSwapBuffers(window);
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    
}


// mouse button events (you do not need to modify it unless you want to)
void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y) {
	
}