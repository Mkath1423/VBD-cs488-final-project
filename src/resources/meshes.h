
#include "resources.h"

#include <iostream>

#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>

#include "../utilities/file_reading.h"

#include <map>
#include <vector>

#include "linalg.h"
using namespace linalg::aliases;



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

    void buffer_elements(void* indicies, size_t _nIndicies) {
        bind();

        nIndicies = _nIndicies;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            nIndicies * sizeof(unsigned int), indicies, GL_STATIC_DRAW);

        unbind();
    }

    void buffer_vertices(void* vertices, size_t _nVertices) {
        bind();

        nVertices = _nVertices;
        glBufferData(GL_ARRAY_BUFFER,
            nVertices * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

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

struct StaticTriangleMesh : public SharableResource<StaticTriangleMesh> {
    RenderMesh renderMesh;

    const std::filesystem::path objFile;

    float radius;

    TriangleMeshFileData mesh;
    
    StaticTriangleMesh(std::filesystem::path objFile, float radius = -1) : objFile{ objFile }, radius{ radius } {

    }

    bool init() {
        if(!load_obj_file(objFile, mesh)) {
            std::cerr << "StaticTriangleMesh: Failed to load file: " << objFile << std::endl;
            return false;
        }

        if (radius > 0) {
            float3 center;
            for (int i = 0; i < mesh.positions.size(); i += 3) {
                center += float3(
                    mesh.positions[i + 0], 
                    mesh.positions[i + 1], 
                    mesh.positions[i + 2]);
            }

            float maxDist = 0;
            for (int i = 0; i < mesh.positions.size(); i += 3) {
                maxDist = std::max(
                    maxDist, 
                    length(float3(
                        mesh.positions[i + 0],
                        mesh.positions[i + 1],
                        mesh.positions[i + 2]) - center));
            }

            for (int i = 0; i < mesh.positions.size(); i += 3) {
                float3 pos = float3(
                    mesh.positions[i + 0],
                    mesh.positions[i + 1],
                    mesh.positions[i + 2]);
                pos = (radius / maxDist) * (pos - center) + center;
                mesh.positions[i + 0] = pos.x;
                mesh.positions[i + 1] = pos.y;
                mesh.positions[i + 2] = pos.z;
            }
        }

        renderMesh.buffer_data(
            mesh.positions.data(), mesh.positions.size(), 
            mesh.faces.data(), mesh.faces.size());
        
        return true;
    }

    
    
    ~StaticTriangleMesh() {
        if (initialized) {
            renderMesh.free_buffers();
        }
    }

    void draw(){
        renderMesh.draw_triangles();
    }
};


struct TetrahedraMesh : public SharableResource<TetrahedraMesh> {
    const std::filesystem::path nodePath, elementPath, facePath;
    
    TetGenFileData fileData;

    size_t num_elements;
    size_t num_vertices;
    size_t num_faces;

    float volume = 0;
    float3 center;

    struct Element {
        uint4 indicies;
        float rest_volume;
        float3x3 Dminv;
    };

    std::vector<Element> elementData;

    struct VertexElementIndex {
        unsigned int elementIdx, subIdx;

        VertexElementIndex(): elementIdx{0}, subIdx{0} {}
        VertexElementIndex(unsigned int elementIdx, unsigned int subIdx) : elementIdx{ elementIdx }, subIdx{ subIdx } {}
    };

    std::map<unsigned int, std::vector<VertexElementIndex>> vertex_adjacency;

    TetrahedraMesh(
        std::filesystem::path nodePath,
        std::filesystem::path elementPath,
        std::filesystem::path facePath) : 
        nodePath{nodePath},
        elementPath{elementPath}, 
        facePath{facePath},
        num_elements{ 0 },
        num_vertices{ 0 },
        num_faces{ 0 },
        center{0, 0, 0} {
    }
    
    float3 position(size_t idx) {
        return float3(
            fileData.positions[3 * idx + 0],
            fileData.positions[3 * idx + 1],
            fileData.positions[3 * idx + 2]
        );
    }

    uint3 face(size_t idx) {
        return uint3(
            fileData.faces[3 * idx + 0],
            fileData.faces[3 * idx + 1],
            fileData.faces[3 * idx + 2]
        );
    }

    uint4 element(size_t idx) {
        return uint4(
            fileData.elements[fileData.element_size * idx + 0],
            fileData.elements[fileData.element_size * idx + 1],
            fileData.elements[fileData.element_size * idx + 2],
            fileData.elements[fileData.element_size * idx + 3]
        );
    }

    bool init() {
        if(!load_tet_gen_file(nodePath, elementPath, facePath, fileData)) {
            std::cerr << "TetrahedraMesh: Failed to load file: " << 
                nodePath << " " << elementPath << " " << facePath << std::endl;
            return false;
        }

        num_elements = fileData.elements.size() / fileData.element_size;
        num_vertices = fileData.positions.size() / 3;
        num_faces = fileData.faces.size() / 3;

        center = float3(0.0);

        for (int i = 0; i < num_vertices; ++i) {
            center += position(i);
        }

        center /= num_vertices;

        volume = 0.0;

        for (unsigned int i = 0; i < num_elements; ++i) {
            auto &ele = elementData.emplace_back();
            
            ele.indicies = element(i);

            vertex_adjacency[ele.indicies.x].push_back(VertexElementIndex( i, 0 ));
            vertex_adjacency[ele.indicies.y].push_back(VertexElementIndex( i, 1 ));
            vertex_adjacency[ele.indicies.z].push_back(VertexElementIndex( i, 2 ));
            vertex_adjacency[ele.indicies.w].push_back(VertexElementIndex( i, 3 ));

            float3 a = position(ele.indicies.x);
            float3 b = position(ele.indicies.y);
            float3 c = position(ele.indicies.z);
            float3 d = position(ele.indicies.w);

            ele.rest_volume = std::abs(dot(cross(b - a, c - a), d - a)) / 6.0;
            volume += ele.rest_volume;

            ele.Dminv = linalg::inverse(float3x3(b - a, c - a, d - a));

        }

        return true;
    }
};