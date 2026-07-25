#include "shader.h"

std::string read_file(const std::filesystem::path &resourcePath) {
    std::ifstream file(resourcePath);
    if (!file) return "";

    std::stringstream source;
    source << file.rdbuf();

    return source.str();
}


bool Shader::init_impl()  {
    auto vertexShaderSource = read_file(vert);
    auto fragmentShaderSource = read_file(frag);

    
    const char* vertSource = vertexShaderSource.c_str();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSource, NULL);
    glCompileShader(vertexShader);
    
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        glDeleteShader(vertexShader);
        return false;
    }

    // Fragment shader
    const char* fragSource = fragmentShaderSource.c_str();
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);

    // Check for compile time errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    // link
    auto shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(shaderProgram);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // if everything was successful, then save the id
    shaderProgramID = shaderProgram;
    return true;
}
    
void Shader::cleanup_impl() {
    glDeleteProgram(shaderProgramID);
}

void Shader::useShader() const {
    glUseProgram(shaderProgramID);
}

GLint Shader::getUniformLocation(const std::string &name) const {
    return glGetUniformLocation(shaderProgramID, name.c_str());
}

