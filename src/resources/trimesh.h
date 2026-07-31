
#include "resources.h"

#include <iostream>

#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>

// https://www.scratchapixel.com/lessons/3d-basic-rendering/obj-file-format//obj-file-format.html


class RenderMesh {
    private: 
    GLuint VBO = -1, VAO = -1, EBO = -1;
    bool buffers_initialized = false;
    size_t nVertices = 0, nIndicies = 0;
    
    protected:
    bool dirty;
    
    void free_buffers(){
        if(buffers_initialized){
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
    }

    RenderMesh() { 
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0); // unbind first to safely unbind the buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
    };

    void buffer_data(void *vertices, size_t nVertices, void *indicies, size_t nIndicies){
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 
            nVertices * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
            
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            nIndicies * sizeof(unsigned int), indicies, GL_STATIC_DRAW);
    }

    public:
    bool is_dirty() {
        return dirty;
    }

    // virtual void upload_mesh_data() = 0;

    virtual ~RenderMesh() {
        free_buffers();
    }
    
};

struct TriangleMesh {
  
    struct Vertex {

    };

    const std::filesystem::path objFile;

    
    TriangleMesh(std::filesystem::path objFile) : objFile{objFile} {

    }

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<unsigned int> faces;

    bool load(){
        positions.clear();
        normals.clear();
        uvs.clear();
        faces.clear();

        std::ifstream file(objFile);
        if (!file) {
            std::cerr << "TriangleMesh: Failed to load file: " << objFile << std::endl;
            return false;}
    
        std::string line;

        while(std::getline(file, line)){
            float x, y, z;
            int a, b, c, d;
            std::string tmp;
            std::string type;

            std::istringstream reader(line);

            // skip empty lines
            if(!(reader >> type)) continue;
            
            else if (type == "v" && (reader >> x >> y >> z)){
                positions.push_back(x);
                positions.push_back(y);
                positions.push_back(z);
            }
            else if (type == "vn" && (reader >> x >> y >> z)){
                normals.push_back(x);
                normals.push_back(y);
                normals.push_back(z);
            }
            else if (type == "vt" && (reader >> x >> y)){
                uvs.push_back(x);
                uvs.push_back(y);
            }
            else if (type == "f" && (reader >> a >> tmp >> b >> tmp >> c)){
                a = a > 0 ? a - 1 : (positions.size() / 3) - a;
                b = b > 0 ? b - 1 : (positions.size() / 3) - b;
                c = c > 0 ? c - 1 : (positions.size() / 3) - c;

                printf("face %d %d %d\n", a, b, c);

                faces.push_back(a);
                faces.push_back(b);
                faces.push_back(c);
            }
        }

        file.close();
        
        return true;
    }
};
