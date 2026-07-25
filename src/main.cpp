#include <iostream>

#include <fstream>
#include <sstream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "config.h"


#include "resources/resources.h"
#include "resources/shader.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

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


int main(int argc, char *argv[])
{

    ResourceManager rm;

    config = parse_args(argc, argv);

    // vertexShaderSource = read_file(config.resources / "shaders" / "default.vert");
    // fragmentShaderSource = read_file(config.resources / "shaders" / "default.frag");

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


    // // Build and compile our shader program
    // // Vertex shader
    // const char* vertSource = vertexShaderSource.c_str();
    // GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // glShaderSource(vertexShader, 1, &vertSource, NULL);
    // glCompileShader(vertexShader);
    // // Check for compile time errors
    // GLint success;
    // GLchar infoLog[512];
    // glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    // if (!success)
    // {
    //     glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    //     std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    // }


    // // Fragment shader
    // const char* fragSource = fragmentShaderSource.c_str();
    // GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // glShaderSource(fragmentShader, 1, &fragSource, NULL);
    // glCompileShader(fragmentShader);
    // // Check for compile time errors
    // glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    // if (!success)
    // {
    //     glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    //     std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    // }
    // // Link shaders
    // GLuint shaderProgram = glCreateProgram();
    // glAttachShader(shaderProgram, vertexShader);
    // glAttachShader(shaderProgram, fragmentShader);
    // glLinkProgram(shaderProgram);
    // // Check for linking errors
    // glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    // if (!success) {
    //     glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    //     std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    // }
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);


    ResourceHandle defaultShader = rm.add(std::make_unique<Shader>(
        (config.resources / "shaders" / "default.vert"), 
        (config.resources / "shaders" / "default.frag")));

    

    GLfloat vertices[] = {
        // First triangle
        -0.9f, -0.5f, 0.0f,  // Left 
        -0.0f, -0.5f, 0.0f,  // Right
        -0.45f, 0.5f, 0.0f,  // Top 
        // Second triangle
         0.0f, -0.5f, 0.0f,  // Left
         0.9f, -0.5f, 0.0f,  // Right
         0.45f, 0.5f, 0.0f   // Top 
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw our first triangle


        float4x4 identity = linalg::identity;

        // glUseProgram(shaderProgram);
        rm.get<Shader>(defaultShader)->useShader();



        const GLint modelLoc = rm.get<Shader>(defaultShader)->getUniformLocation("model");
        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            &identity[0][0]
        );

        float4x4 viewMat = lookAtMatrix();

        const GLint viewLoc = rm.get<Shader>(defaultShader)->getUniformLocation("view");
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

        const GLint projLoc = rm.get<Shader>(defaultShader)->getUniformLocation("projection");
        glUniformMatrix4fv(
            projLoc,
            1,
            GL_FALSE,
            &projMat[0][0]
        );

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
        glBindVertexArray(0);

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