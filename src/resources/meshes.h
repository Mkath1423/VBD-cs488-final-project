
#include "resources.h"

#include <iostream>

#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>

#include "../utilities/file_reading.h"



class RenderMesh  {
    private: 
    GLuint VBO = -1, VAO = -1, EBO = -1;
    bool buffers_initialized = false;

    size_t nVertices = -1, nIndicies = -1;


    void bind(){
        if(!buffers_initialized){
            init_buffers();
        }

        glBindVertexArray(VAO); 
        glBindBuffer(GL_ARRAY_BUFFER, VBO); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    }

    void unbind(){
        glBindVertexArray(0); // unbind first to safely unbind the buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void init_buffers(){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        unbind();

        buffers_initialized = true;
    }

    public:
    
    void free_buffers(){
        if(buffers_initialized){
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
    }

    void buffer_vertices(void *vertices, size_t _nVertices){
        bind();
        nVertices = _nVertices;
        glBufferData(GL_ARRAY_BUFFER, 
            nVertices * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
        unbind();
    }

    void buffer_data(void *vertices, size_t _nVertices, void *indicies, size_t _nIndicies){
        bind();

        nVertices = _nVertices;
        nIndicies = _nIndicies;
        glBufferData(GL_ARRAY_BUFFER, 
            nVertices * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
            
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            nIndicies * sizeof(unsigned int), indicies, GL_STATIC_DRAW);

        unbind();
    }


    void draw_triangles(){
        if(!buffers_initialized){
            printf("skipping draw! no buffers");
            return;
        }

        glBindVertexArray(VAO);
        glDrawElements(
            GL_TRIANGLES,
            nIndicies,
            GL_UNSIGNED_INT,
            nullptr);
        
        glBindVertexArray(0);
    }
};

struct StaticTriangleMesh : public ManagedResource {
    RenderMesh renderMesh;

    const std::filesystem::path objFile;

    TriangleMeshFileData mesh;
    
    StaticTriangleMesh(std::filesystem::path objFile) : objFile{objFile} {

    }

    
    virtual bool init_impl(){
        if(!load_obj_file(objFile, mesh)) {
            std::cerr << "StaticTriangleMesh: Failed to load file: " << objFile << std::endl;
            return false;
        }

        renderMesh.buffer_data(
            mesh.positions.data(), mesh.positions.size(), 
            mesh.faces.data(), mesh.faces.size());
        
        return true;
    }


    virtual void cleanup_impl(){
        renderMesh.free_buffers();
    }
    
    void draw(){
        renderMesh.draw_triangles();
    }
};


struct TetrahedraMesh : public ManagedResource {
    RenderMesh renderMesh;

    const std::filesystem::path nodePath, elementPath, facePath;
    
    TetGenFileData mesh;

    TetrahedraMesh(
        std::filesystem::path nodePath,
        std::filesystem::path elementPath,
        std::filesystem::path facePath) : 
        nodePath{nodePath},
        elementPath{elementPath}, 
        facePath{facePath} {}

    
    virtual bool init_impl(){
        if(!load_tet_gen_file(nodePath, elementPath, facePath, mesh)) {
            std::cerr << "TetrahedraMesh: Failed to load file: " << 
                nodePath << " " << elementPath << " " << facePath << std::endl;
            return false;
        }

        renderMesh.buffer_data(
            mesh.positions.data(), mesh.positions.size(), 
            mesh.faces.data(), mesh.faces.size());
        
        return true;
    }


    virtual void cleanup_impl(){
        renderMesh.free_buffers();
    }
    
    void draw(){
        renderMesh.draw_triangles();
    }
};