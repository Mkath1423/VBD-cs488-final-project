#pragma once
#include "resources.h"

#include <iostream>

#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

std::string read_file(const std::filesystem::path &resourcePath);



class Shader : public ManagedResource {
    const std::filesystem::path vert;
    const std::filesystem::path frag;

    GLuint shaderProgramID;

    protected:
    
    bool init_impl() override;
    
    void cleanup_impl() override;

    public:
    Shader(const std::filesystem::path &vert, const std::filesystem::path &frag) : 
        vert{vert}, frag{frag}, shaderProgramID{0} {}

    void useShader() const;
    GLint getUniformLocation(const std::string &name) const;
};

