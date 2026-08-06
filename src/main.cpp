#include <iostream>

#include <fstream>
#include <sstream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "config.h"

#include "resources/shader.h"
#include "resources/meshes.h"

#include "linalg.h"

using namespace linalg::aliases;


#include "rendering.h"


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); 
void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y);

Config config;
GLFWwindow* window;

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

float3 solveHouseholderQR(float3x3 H, float3 f) {
    auto dot_from_k = [](float3 a, float3 b, int k) {
        float acc = 0;

        for (int i = k; i < 3; ++i) { // row
            acc += a[i] * b[i];
        }

        return acc;
        };

    for (int k = 0; k < 3; ++k) {
        float3 x = H[k];

        float3 vk = x;
        vk[k] += (x[k] > 0 ? 1 : -1) * sqrt(dot_from_k(x, x, k));
        vk /= sqrt(dot_from_k(vk, vk, k));

        // update lower right block
        for (int j = k; j < 3; ++j) { // col
            float vkTAj = dot_from_k(vk, H[j], k);

            // update R
            for (int i = k; i < 3; ++i) { // row
                H[j][i] -= 2 * vk[i] * vkTAj;
            }
        }

        // update QTb
        float vkTb = dot_from_k(vk, f, k);

        for (int i = k; i < 3; ++i) { // row
            f[i] -= 2 * vk[i] * vkTb;
        }
    }

    // backward solve
    for (int i = 3 - 1; i >= 0; --i) {
        float tmp = 0;
        for (int j = i + 1; j < 3; ++j) {
            tmp += H[i][j] * f[j];
        }
        f[i] = (f[i] - tmp) / H[i][i];
    }

    return f;
}


struct Vertex {
    float3 position;
    float3 prevPosition;
    float3 velocity;
    float mass;
};

struct PhysicsBody;

struct ForceElement {
    virtual void add_force_and_hessian(unsigned int vertex_id, float3& f, float3x3& H) = 0;
};

struct PhysicsBody {
    std::vector<Vertex> vertices;
    std::vector<std::shared_ptr<ForceElement>> forces;
};

struct PhysicsConfig {
    float timestep;
    float3 a_ext;
};


struct Entity {
    uint64_t id;
    std::unique_ptr<PhysicsBody> body;
    std::unique_ptr<RenderProxy> drawer;
    //std::shared_ptr<CollisionObject> geo;

    Entity() : id{0}, body{nullptr}, drawer{ nullptr } { }
};



Camera build_default_camera() {
    Camera c;

    c.origin = { 0.0, 5.0, -20 };
    c.poi = { 0.0, 5.0, 0.0 };
    c.up = { 0.0, 1.0, 0.0 };

    float globalAspectRatio = float(config.width) / float(config.height);
    float globalFOV = 45.0f; // vertical field of view
    float globalDepthMin = 5.0E-02; // for rasterization
    float globalDepthMax = 100.0f; // for rasterization

    c.aspect = globalAspectRatio;
    c.fovy = globalFOV;
    c.zNear = globalDepthMin;
    c.zFar = globalDepthMax;

    return c;
}

struct Particles {
    std::vector<float3> positions;
    std::vector<float3> velocities;

    std::shared_ptr<StaticTriangleMesh> mesh;
    std::shared_ptr<Shader> shader;
    float radius, mass;
};


class ParticleRenderProxy : public RenderProxy {

    // config parameters
    std::shared_ptr<Shader> shader;
    std::shared_ptr<StaticTriangleMesh> mesh;
    float radius;

    // synced state
    std::vector<float3> positions;

    std::weak_ptr<Entity> w_entity;

public:
    ParticleRenderProxy(Particles& config, std::weak_ptr<Entity> _entity) :
        w_entity{ std::move(_entity) },
        shader{ config.shader },
        mesh{ config.mesh },
        radius{ config.radius } {
    }

    void sync() override {
        auto entity = w_entity.lock();
        if (entity && entity->body) {

            positions.clear();
            for (const auto& v : entity->body->vertices) {
                positions.emplace_back(v.prevPosition);
            }
        }
    }

    void draw(Renderer& renderer) override {
        auto entity = w_entity.lock();
        if (entity) { // only draw things that still exist
            renderer.setShader(shader);

            for (auto& p : positions) {
                float4x4 modelMat = linalg::identity;
                modelMat *= radius;
                modelMat.w = float4(p.x, p.y, p.z, 1.0);

                renderer.setTransform(modelMat);
                mesh->draw();
            }
        }
    }
};

enum EnergyModel {
    None,
    StandardNeoHookean,
    StableNeoHookeanNoLog,
    StableNeoHookean,
} ;

enum ElasticEnergyDifferentiationMode {
    FiniteDifferences,
    Analytical
} ;

struct SoftBody {
    float3 offset;
    float3 scale;

    std::shared_ptr<TetrahedraMesh> mesh;
    std::shared_ptr<Shader> shader;
    
    float mass;
    EnergyModel energy_model;
    ElasticEnergyDifferentiationMode differentiation_mode;


    float mu, lambda;

   /* static void create(SoftBody& config, std::shared_ptr<Entity> e) {

    }*/
};

class SoftBodyRenderProxy : public RenderProxy {
    // rendering things
    RenderMesh renderMesh;

    // config parameters
    std::shared_ptr<Shader> shader;
    std::shared_ptr<TetrahedraMesh> mesh;

    // synced state
    std::vector<float3> positions;

    std::weak_ptr<Entity> w_entity;

public:
    SoftBodyRenderProxy(SoftBody& config, std::weak_ptr<Entity> _entity) :
        w_entity{ std::move(_entity) },
        shader{ config.shader },
        mesh{ config.mesh } {

        renderMesh.buffer_elements(
            mesh->fileData.faces.data(), 
            mesh->fileData.faces.size());
    }

    void sync() override {
        auto entity = w_entity.lock();
        if (entity && entity->body) {

            std::vector<float> vertex_buffer;
            vertex_buffer.reserve(3 * entity->body->vertices.size());

            for (const auto& v : entity->body->vertices) {
                vertex_buffer.emplace_back(v.prevPosition.x);
                vertex_buffer.push_back(v.prevPosition.y);
                vertex_buffer.push_back(v.prevPosition.z);
            }

            renderMesh.buffer_vertices(
                vertex_buffer.data(), vertex_buffer.size());
        }
    }

    void draw(Renderer& renderer) override {
        auto entity = w_entity.lock();
        if (entity) { 
            renderer.setShader(shader);
            float4x4 I = linalg::identity;
            renderer.setTransform(I);
            renderMesh.draw_triangles();
        }
    }
};

namespace StandardNeoHookenModel {

    struct float9x9 {
        float data[81];

        float9x9() {}

        float& at(size_t r, size_t c) {
            return data[r + c * 9];
        }
    };

    static float energy(const float3x3 F, const float mu, const float lambda, float restVolume) {
        float J = linalg::determinant(F);

        float Ic = linalg::trace(mul(linalg::transpose(F), F));
        float phi = 0.5 * mu * (Ic - 3) + 0.5 * lambda * std::pow((J - 1), 2.0);

        float E = restVolume * phi;
        return E;
    }


    static void forceAndHessian(unsigned int vertexIndex, const float3x3 &Dminv, const float3x3 &F, const float mu, const float lambda, float restVolume, float3 &force, float3x3 &hessian) {

        // Force
        
        // derivative of energy w.r.t deformation gradient
        float J = linalg::determinant(F);

        float3x3 dJdF;
        dJdF[0] = cross(F[1], F[2]);
        dJdF[1] = cross(F[2], F[0]);
        dJdF[2] = cross(F[0], F[1]);

        float3x3 dEdF = restVolume * (mu * F + lambda * (J - 1) * dJdF);

        // derivative of deformation gradient w.r.t. vertex position
        float3 dFdx;
        if (vertexIndex == 0) {
            dFdx[0] = -sum(Dminv[0]);
            dFdx[1] = -sum(Dminv[1]);
            dFdx[2] = -sum(Dminv[2]);
        }
        else {
            dFdx[0] = Dminv[0][vertexIndex - 1];
            dFdx[1] = Dminv[1][vertexIndex - 1];
            dFdx[2] = Dminv[2][vertexIndex - 1];
        }
        
        // chain rule
        // dEdx = dFdx : dEdF
        // where dFdx is a third order tensor and ": dEdF" is the double contraction of its elements by dEdF
        // in the case of 3D tetrahedra dFdx is very sparse 
        // each sub matrix has one none zero row and they are all the same but shifted down
        float3 dEdx;
        for (int i = 0; i < 3; ++i) {
            dEdx[i] = dFdx[0] * dEdF[0][i] + dFdx[1] * dEdF[1][i] + dFdx[2] * dEdF[2][i];
        }

        force += -dEdx;

        // Hessian
        // this uses dense 9x9 matrices which represent flattened 4th order tensors
        //  (i wish i was still in pytorch land)
        
        // surely this will get inlined...
        // im not using attributes to make it happen though

        float9x9 dPhidFdF;

        // flattened indexing a 3x3 matrix
        auto ref = [](float3x3& mat, size_t idx) -> float& {
            return mat[idx % 3][idx / 3];
        };

        // outer product of vec(dJdF) and vec(dJdF)
        // i really hope this gets unrolled
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                dPhidFdF.at(i, j) = lambda * ref(dJdF, i) * ref(dJdF, j);
            }

            // diagonal mu * I_9x9 terms
            dPhidFdF.at(i, i) += mu;
        }

        // the wreched d2JdF2 term
        // (its a bunch of cross product matrices)
        float c = lambda * (J - 1);

        auto block = [&c, &dPhidFdF](float3 f, size_t block_row, size_t block_col) -> void {
            // 00
            dPhidFdF.at(1 + block_row * 3, 0 + block_col * 3) += c * f[2];
            dPhidFdF.at(2 + block_row * 3, 0 + block_col * 3) -= c * f[1];
            
            dPhidFdF.at(0 + block_row * 3, 1 + block_col * 3) -= c * f[2];
            // 11
            dPhidFdF.at(2 + block_row * 3, 1 + block_col * 3) += c * f[0];
            
            dPhidFdF.at(0 + block_row * 3, 2 + block_col * 3) += c * f[1];
            dPhidFdF.at(1 + block_row * 3, 2 + block_col * 3) -= c * f[0];
            // 22
        };

        // block(0, 0) = 0
        block( F[2], 1, 0); // block(1, 0) =  hatf2
        block(-F[1], 2, 0); // block(2, 0) = -hatf1
        
        block(-F[2], 0, 1); // block(0, 1) = -hatf2
        // block(1, 1) = 0
        block( F[0], 2, 1); // block(2, 1) =  hatf0
        
        block( F[1], 0, 2); // block(0, 2) =  hatf1
        block(-F[0], 1, 2); // block(1, 2) = -hatf0
        // block(2, 2) = 0

        for (int i = 0; i < 9 * 9; ++i) dPhidFdF.data[i] *= restVolume;

        float3x3 dPhidxdx;
        
        // this exploits the sparity of the dFdx matrix...
        // ...but at a small cost
        // have fun reading that one!
        for (size_t row = 0; row < 3; ++row) {
            for (size_t col = 0; col < 3; ++col) {
                auto vecdFdxT_dPhidFdF = [&dFdx, &dPhidFdF](size_t r, size_t c) -> float {
                    return 
                        dFdx[0] * dPhidFdF.at(r + 0, c) + 
                        dFdx[1] * dPhidFdF.at(r + 3, c) + 
                        dFdx[2] * dPhidFdF.at(r + 6, c);
                };
                /*
                dPhidxdx[col][row] = (vecdFdx^T * dPhidFdF)[row, :] . vecdFdx[col]
                dPhidxdx[col][row] = 
                    (vecdFdx ^ T * dPhidFdF)[row, 0 + col] * vecdFdx[row + 0, col] + 
                    (vecdFdx ^ T * dPhidFdF)[row, 3 + col] * vecdFdx[row + 3, col] + 
                    (vecdFdx ^ T * dPhidFdF)[row, 6 + col] * vecdFdx[row + 6, col]

                dPhidxdx[col][row] = 
                    (vecdFdx ^ T * dPhidFdF)[row, 0 + col] * dFdx[0]+ 
                    (vecdFdx ^ T * dPhidFdF)[row, 3 + col] * dFdx[1]+ 
                    (vecdFdx ^ T * dPhidFdF)[row, 6 + col] * dFdx[2]
                
                */

                dPhidxdx[col][row] =
                    dFdx[0] * vecdFdxT_dPhidFdF(row, col + 0) +
                    dFdx[1] * vecdFdxT_dPhidFdF(row, col + 3) +
                    dFdx[2] * vecdFdxT_dPhidFdF(row, col + 6);
            }
        }

        // i lied this is actually the energy not the energy density :)
        hessian += dPhidxdx;

    }
};

struct DeformationForceElement : public ForceElement {
    // config parameters
    float mu, lambda;
    std::shared_ptr<TetrahedraMesh> mesh;
    float3 scale;
    EnergyModel energy_model;
    ElasticEnergyDifferentiationMode differentiation_mode;
    
    std::weak_ptr<Entity> w_entity;

    float alpha(float mu, float lambda) {
        return 1 + mu / lambda;
    }

    float3x3 deformationGradient(TetrahedraMesh::Element &ele, float3 positions[4]) {

        // Dm = D(scale) * Dm_original <- scaleing the points
        // Dm^-1 = (Dm_original^-1) * D(scale)^-1
        auto Dminv = ele.Dminv;
        Dminv[0] /= scale[0];
        Dminv[1] /= scale[1];
        Dminv[2] /= scale[2];

        auto Ds = float3x3(positions[1] - positions[0], positions[2] - positions[0], positions[3] - positions[0]);

        return mul(Ds, Dminv);
    }

    float computeEnergy(TetrahedraMesh::Element& ele, float3 positions[4]) {

        auto F = deformationGradient(ele, positions);

        switch (energy_model) {
        case EnergyModel::StandardNeoHookean:
            return StandardNeoHookenModel::energy(F, mu, lambda, ele.rest_volume);
        default:
            return 0;
        }
    }



    void add_force_and_hessian_from_tet(PhysicsBody &body, unsigned int vertex, unsigned int tet, unsigned int vertexInTet, float3& f, float3x3& H, ElasticEnergyDifferentiationMode mode) {
        auto& ele = mesh->elementData[tet];
        
        float3 positions[4] = {
                body.vertices[ele.indicies.x].position,
                body.vertices[ele.indicies.y].position,
                body.vertices[ele.indicies.z].position,
                body.vertices[ele.indicies.w].position
        };

        if (mode == ElasticEnergyDifferentiationMode::FiniteDifferences) {

            float h = 1e-4;

            const float3 x(1.0, 0.0, 0.0);
            const float3 y(0.0, 1.0, 0.0);
            const float3 z(0.0, 0.0, 1.0);

            

            // float E_ref = computeEnergy(ele, positions);

            for (int i = 0; i < 3; ++i) {
                float3 plus[4] = {
                    positions[0],
                    positions[1],
                    positions[2],
                    positions[3]
                };

                float3 minus[4] = {
                    positions[0],
                    positions[1],
                    positions[2],
                    positions[3]
                };

                plus[vertexInTet][i] += h;
                minus[vertexInTet][i] -= h;

                f[i] -= (computeEnergy(ele, plus) - computeEnergy(ele, minus)) / (2.0f * h);
            }

            for (int i = 0; i < 3; ++i) {
                for (int j = i; j < 3; ++j) {

                    float h = 1e-3;

                    float3 ipjp[4] = {
                        positions[0],
                        positions[1],
                        positions[2],
                        positions[3]
                    };

                    float3 ipjm[4] = {
                        positions[0],
                        positions[1],
                        positions[2],
                        positions[3]
                    };
                    
                    float3 imjp[4] = {
                        positions[0],
                        positions[1],
                        positions[2],
                        positions[3]
                    };

                    float3 imjm[4] = {
                        positions[0],
                        positions[1],
                        positions[2],
                        positions[3]
                    };

                    ipjp[vertexInTet][i] += h;
                    ipjp[vertexInTet][j] += h;

                    ipjm[vertexInTet][i] += h;
                    ipjm[vertexInTet][j] -= h;

                    imjp[vertexInTet][i] -= h;
                    imjp[vertexInTet][j] += h;

                    imjm[vertexInTet][i] -= h;
                    imjm[vertexInTet][j] -= h;

                    // this is a huge waste of stack space...
                    // ...the compiler can have fun optimizing it though
                    // its not even 100 floats, not that bad really

                    float H_ij = computeEnergy(ele, ipjp) - computeEnergy(ele, ipjm) - (computeEnergy(ele, imjp) - computeEnergy(ele, imjm));

                    H_ij /= 4 * h * h;

                    H[i][j] += H_ij;

                    // hessian is symmetric
                    if(i != j) H[j][i] += H_ij;
                }
            }



        }
        else {
            auto F = deformationGradient(ele, positions);

            auto Dminv = ele.Dminv;
            Dminv[0] /= scale[0];
            Dminv[1] /= scale[1];
            Dminv[2] /= scale[2];


            switch (energy_model) {
            case EnergyModel::StandardNeoHookean:
                StandardNeoHookenModel::forceAndHessian(vertexInTet, Dminv, F, mu, lambda, ele.rest_volume, f, H);
                return;
            default:
                return;
            }
        }
        //printf("deformation energy on %d from %d = %f\n", vertex, tet, E);
    }

public:
    DeformationForceElement(SoftBody& config, std::weak_ptr<Entity> _entity) :
        w_entity{ std::move(_entity) },
        mesh{ config.mesh },
        scale{ config.scale },
        mu{ config.mu },
        lambda{ config.lambda },
        energy_model{ config.energy_model },
        differentiation_mode{ config.differentiation_mode } {
    }
    

    void add_force_and_hessian(unsigned int vertex_id, float3& f, float3x3& H) override {
        if (!mesh || energy_model == EnergyModel::None) return;
        auto entity = w_entity.lock();
        if (entity && entity->body) {

            for (auto& pair : mesh->vertex_adjacency[vertex_id]) {

                add_force_and_hessian_from_tet(*entity->body, vertex_id, pair.elementIdx, pair.subIdx, f, H, differentiation_mode);

            }
        }
    }
};

void createSoftBody(SoftBody& config, std::shared_ptr<Entity> entity) {
    entity->drawer = std::move(std::make_unique<SoftBodyRenderProxy>(config, entity));
    entity->body = std::move(std::make_unique<PhysicsBody>());

    float scale_factor = config.scale.x * config.scale.y * config.scale.z;

    float density = config.mass / config.mesh->volume;

    for (int i = 0; i < config.mesh->num_vertices; ++i) {
        Vertex v;

        float3 pos = config.mesh->position(i);
        pos -= config.mesh->center;

        pos = cmul(pos, config.scale + (i != 0 ? 0.9f : 0.0f));
        pos += config.offset;

        v.prevPosition = pos;
        v.velocity = float3(0.0);
        
        float volume = 0;

        // add up the total volume of adjacent elements
        for (auto &ele : config.mesh->vertex_adjacency[i]) {
            volume += config.mesh->elementData[ele.elementIdx].rest_volume;
        }

        // each vertex gets a quarter of the adjacent mass
        v.mass = density * (0.25 * volume);

        entity->body->vertices.emplace_back(v);
    }

    entity->body->forces.emplace_back(std::make_shared<DeformationForceElement>(config, entity));
}


struct World {

    std::shared_ptr<Entity> e_particles;
    std::shared_ptr<Entity> e_tet;

    Renderer renderer;
    PhysicsConfig physics;

    World() : renderer{ build_default_camera() }, 
        e_particles{ std::make_shared<Entity>() },
        e_tet{ std::make_shared<Entity>() } {

        std::shared_ptr<Shader> defaultShader = Shader::create(
            (config.resources / "shaders" / "default.vert"),
            (config.resources / "shaders" / "default.frag")
        );

        std::shared_ptr<StaticTriangleMesh> obj = StaticTriangleMesh::create(
            (config.resources / "models" / "smallsphere.obj")
        );


        // particle system
        Particles particle_config;
        particle_config.radius = 10.0f;
        particle_config.mass = 1.0;
        particle_config.mesh = obj;
        particle_config.shader = defaultShader;


        auto n_vertices = 10;
        for (int i = 0; i < n_vertices; ++i) {
            particle_config.positions.emplace_back(i, 1.0f, i);
            particle_config.velocities.emplace_back(0.0f, 5.0 * i + 2.0, 0.0f);
        }

        e_particles->drawer = std::move(std::make_unique<ParticleRenderProxy>(particle_config, e_particles));
        e_particles->body = std::move(std::make_unique<PhysicsBody>());
        

        for (int i = 0; i < n_vertices; ++i) {
            Vertex v;
            v.prevPosition = particle_config.positions[i];
            v.velocity = particle_config.velocities[i];
            v.mass = particle_config.mass;

            e_particles->body->vertices.emplace_back(v);
        }

        // soft body 

        auto base_tet_model = (config.resources / "models" / "tet_sphere");
        std::shared_ptr<TetrahedraMesh> tet_sphere = TetrahedraMesh::create(
            base_tet_model / "tet_sphere.node",
            base_tet_model / "tet_sphere.ele",
            base_tet_model / "tet_sphere.face"
        );

        base_tet_model = (config.resources / "models" / "single_tet");
        std::shared_ptr<TetrahedraMesh> single_tet = TetrahedraMesh::create(
            base_tet_model / "single_tet.node",
            base_tet_model / "single_tet.ele",
            base_tet_model / "single_tet.face"
        );


        SoftBody softbody_config;
        softbody_config.offset = float3(0.0, 5.0, 0.0);
        softbody_config.scale = float3(1.0f);

        softbody_config.mesh = single_tet;
        softbody_config.shader = defaultShader;

        softbody_config.energy_model = EnergyModel::StandardNeoHookean;
        softbody_config.differentiation_mode = ElasticEnergyDifferentiationMode::Analytical;
        softbody_config.mass = 1.0f;
        /*softbody_config.mu = 1.0714e+3;
        softbody_config.lambda = 4.2857e+3f;*/
        softbody_config.mu = 109;
        softbody_config.lambda = 27;

        createSoftBody(softbody_config, e_tet);
        

        // Physics System configuration

        //physics.a_ext = float3(0.0, -9.8, 0.0);
        physics.a_ext = float3(0.0, 0.0, 0.0);
        physics.timestep = 1.0 / 60.0;
    }
    
    void syncOne(std::shared_ptr<Entity>& e) {
        if (e && e->drawer) e->drawer->sync();
    }

    void drawOne(std::shared_ptr<Entity>&e) {
        if (e && e->drawer) e->drawer->draw(renderer);
    }

public:
    void stepPhysics(float timestep) {

        float stiffness = 1e+5;
        float planey = -1;
        float3 planen(0.0, 1.0, 0.0);


        std::vector<PhysicsBody*> bodies;
        if (e_tet && e_tet->body) bodies.push_back(e_tet->body.get());
        if (e_particles && e_particles->body) bodies.push_back(e_particles->body.get());

        // run collision pass

        // make initial guess

        for (auto* body : bodies) {
            for (auto& v : body->vertices) {
                float3 y = v.prevPosition + timestep * v.velocity + timestep * timestep * physics.a_ext;
                v.position = y;
            }
        }

        for (int i = 0; i < 5; ++i) {
                // do a collision update

            for (auto* body : bodies) {
                for (unsigned int vidx  = 0; vidx < body->vertices.size(); ++vidx) {
                    auto& v = body->vertices[vidx];

                    float3 y = v.prevPosition + timestep * v.velocity + timestep * timestep * physics.a_ext;

                    float3 f = -v.mass * (v.position - y) / (timestep * timestep);
                    float3 H = float3(v.mass / (timestep * timestep));

                    // apply a collision constraint
                    float3 diff = float3(0.0, planey - v.position.y, 0.0);
                    float pen = dot(diff, planen);
                    if (pen > 0) {
                        f += stiffness * planen * diff;
                        H.y += stiffness;
                    }

                    float3x3 Hmat;
                    for (int i = 0; i < 3; ++i) {
                        Hmat[i][i] = H[i];
                    }

                    // compute body forces
                    for (auto& force : body->forces) {
                        if (force) force->add_force_and_hessian(vidx, f, Hmat);
                    }


                    float3 dx = solveHouseholderQR(Hmat, f);
                    if (length(dx) > 1) {
                        printf("length(dx)=  %f\n", length(dx));
                        printf("H = I[%f %f %f]^T\n", H.x, H.y, H.z); 
                    }

                    // float3 dx = float3(f.x/H.x, f.y/H.y, f.z/H.z);
                    v.position += dx;
                }

            }
        }

        e_tet->body->vertices[0].position = e_tet->body->vertices[0].prevPosition;

        // commit the changes
        for (auto* body : bodies) {
            for (auto& v : body->vertices) {
                v.velocity = (v.position - v.prevPosition) / timestep;
                v.prevPosition = v.position;
            }
        }

        
    }
    
    void sync() {
        syncOne(e_particles);
        syncOne(e_tet);
    }

    void draw() {
        renderer.startFrame();
        drawOne(e_particles);
        drawOne(e_tet);
    }
};

void run(World world){
    const float timestep = 1.0/60.0;

    const double dt = 1.0 / 60.0;
    double last = glfwGetTime();
    double accumulator = 0.0;

    // sync at the start of the first frame
    world.sync();

    while (!glfwWindowShouldClose(window))
    {

        double now = glfwGetTime();
        accumulator += now - last;
        last = now;
        
        glfwPollEvents();

        // physics stepping
        if (accumulator >= timestep) {
            while (accumulator >= timestep)
            {
                world.stepPhysics(timestep);
                
                accumulator -= timestep;
            }

            world.sync();
        }

        /*world.stepPhysics(timestep);
        world.sync();*/

        world.draw();
        
        glfwSwapBuffers(window);
    }
}

int main(int argc, char *argv[])
{

    config = parse_args(argc, argv);

    window = setup_window();
    while (!glfwWindowShouldClose(window)) {

        // todo: switch on which world to run
        run(World());

    }
    
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

/*
class Physics


class SimulationObject {
    virtual inertialComponent() = default

    virtual constraintComponent() = 0
        different types of bodies will expect different types of collision results (i believe)
        maybe split this up into several cases

    virtual bodyComponent() = do nothing

    virtual draw() = do nothing
}

class Particle : SimO {
    one physics vertex

    has a radius
    has a shared ptr to mesh and shared ptr to shader
}

class ElasticBody : SimO {
    has many vertices
    has boundary face indices
    has element indices 

    tracks which vertices and elements are on the boundary
    tracks which elements belong to each vertex
    tracks which elements should not collide with each other (i.e. neighbors)
    boundary tets also track which face is the boundary (i.e. which vertex is not a boundary)
}

class PhysicsSystem {
    has references to all the bodies

    generate constraints and contacts
        build acceleration structure    

        sweep for possible interation // or do all pairs
            for a possible interation
                go to the interaction case

        particle particle
            radius test
        
        particle plane
            radius test

            
        particle plane
            radius test

        particle boundary-mesh-face
            ...


        // for the elastic meshes

        vertex face 
            same as the previous actually

        edge edge
            ???


        vertex boundary-plane
            same as vertex face basically

        // scratch the below
        particle boundary-tet
            test against boundary face

        boundary-tet boundary-tet
            check if they should be masked
            use GJK to find initial simplex
            use EPA to find the supporting simplex on each tet
            this tells us if we have a p-f or e-e collision

            store a contact on either the p e or f
             * also check if the contact has already been generated
             * several tets can have a vertex A, 
               * if A is contacting face B then it will be doing so in several tets
             * not sure if there is a way to avoid that extra work 
            

            need the barycentric coordinates (at least one will be 0)
             * we can write the provided energy term as a function of the tet vertices
             * then we can differentiate wrt each vertex

        boundary-tet plane
            

        
        for every particle
            test against every particle
             * generate: ParticleParticle p1 p2
            test every particle against the faces of every elastic
            test against the faces of every static
            test against plane primatives




    step()
        determine constraints on all the physics vertices
         * contact
         * boundaries 
        
        until converged or stop
          for every body
            for every vertex
                create initial guess

          for every body 
            for every vertex
                build f and H using

        steps the physics

}

class Simulation {
    init()
        create all the objects in the world
        configure a physics system and register things with that

        load all the shaders and file resources that it needs

        setup rendering information when building the various scene objects

    stepPhysics()
        invoke the physics system

    draw()
        for every sim object

}

*/