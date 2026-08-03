#include <iostream>

#include <fstream>
#include <sstream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "config.h"

#include "engine.h"
#include "resources/shader.h"
#include "resources/meshes.h"

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


#include "rendering.h"



GLFWwindow *setup_window(){
    if (!glfwInit()) {
        std::cerr << "[Init Failed] Could not start glfw." << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    glfwWindowHint(GLFW_DEPTH_BITS, 24);

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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return window;
}



struct Vertex {
    float3 position;
    float3 velocity;
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

GLFWwindow *window;
void run(){
    Engine engine;


    // vertexShaderSource = read_file(config.resources / "shaders" / "default.vert");
    // fragmentShaderSource = read_file(config.resources / "shaders" / "default.frag");

    Resource<Camera> mainCamera = engine.create<Camera>();
    mainCamera->origin = { 0.0, 0.0, -10 };
    mainCamera->poi = { 0.0, 0.0, 0.0 };
    mainCamera->up = { 0.0, 1.0, 0.0 };

    float globalAspectRatio = float(config.width) / float(config.height);
    float globalFOV = 45.0f; // vertical field of view
    float globalDepthMin = 5.0E-02; // for rasterization
    float globalDepthMax = 100.0f; // for rasterization

    mainCamera->aspect = globalAspectRatio;
    mainCamera->fovy = globalFOV;
    mainCamera->zNear = globalDepthMin;
    mainCamera->zFar = globalDepthMax;

    mainCamera->buildMatrices();

    Resource<Shader> defaultShader = engine.create<Shader>(
        (config.resources / "shaders" / "default.vert"), 
        (config.resources / "shaders" / "default.frag")
    );

    Resource<StaticTriangleMesh> obj = engine.create<StaticTriangleMesh>(
        (config.resources / "models" / "smallsphere.obj")
    );

    auto base_tet_model = (config.resources / "models" / "tet_sphere");
    Resource<TetrahedraMesh> obj2 = engine.create<TetrahedraMesh>(
        base_tet_model / "tet_sphere.node",
        base_tet_model / "tet_sphere.ele",
        base_tet_model / "tet_sphere.face"
    );
    
    base_tet_model = (config.resources / "models" / "single_tet");
    Resource<TetrahedraMesh> single_tet = engine.create<TetrahedraMesh>(
        base_tet_model / "single_tet.node",
        base_tet_model / "single_tet.ele",
        base_tet_model / "single_tet.face"
    );

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

        // physics stepping
        while (accumulator >= timestep)
        {
            // naive physics update
            for(auto &v : vertices){
                float3 y = v.position + timestep * v.velocity + timestep * timestep * gravity;
                float3 x_new = y;

                for(int i = 0; i < 5; ++i){
                    float3 f = - mass * (x_new - y) / (timestep * timestep);
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        const GLint viewLoc = defaultShader->getUniformLocation("view");
        glUniformMatrix4fv(
            viewLoc,
            1,
            GL_FALSE,
            &mainCamera->viewMat()[0][0]
        );

        const GLint projLoc = defaultShader->getUniformLocation("projection");
        glUniformMatrix4fv(
            projLoc,
            1,
            GL_FALSE,
            &mainCamera->projMat()[0][0]
        );
        
        obj->draw();

        // single_tet->draw();
        
        }

        glfwSwapBuffers(window);
    }
}

int main(int argc, char *argv[])
{

    config = parse_args(argc, argv);

    window = setup_window();

    run();
    
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