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
};


template <size_t vertex_degree>
class Integrator {
    typename vec<float, vertex_degree> floatN;
    typename mat<float, vertex_degree, vertex_degree> floatNxN;


    class VertexProxy {
        vec<float, vertex_degree> position;
    };

    // internal representation is decided by the implementaion
    virtual void get_body(size_t id, size_t vertices) = 0;
    virtual void get_vertex_position()
};

class Integrator3dof {


    Integrator3dof(size_t num_blocks) {

    }

    inline void integrate(size_t vid, float3& position, float& mass, float3 f, float3x3 H) {

    }
};

class SimulationObject {

};

struct Entities {
    std::shared_ptr<SimulationObject> obj;
    uint64_t render_proxy;

    Entities(): obj{nullptr}, render_proxy{0} {}
};


class World {
    std::vector<SimulationObject*> simObjects;

    /*init()
    create all the objects in the world
    configure a physics system and register things with that

    load all the shaders and file resources that it needs

    setup rendering information when building the various scene objects

    stepPhysics()
    invoke the physics system

    draw()
    for every sim object*/

public:
    virtual void step(float dt) = 0;


    /*void draw() {
        for (auto o : simObjects) {
            o->draw();
        }
    }*/
};

struct Particles : SimulationObject {
    std::vector<Vertex> vertices;

    float radius;

    
    public:
    
    Particles(size_t n, float radius) :
        radius { radius }
    {
        for (int i = 0; i < 10; ++i) {
            vertices.emplace_back();
            vertices[i].position = float3(i, 1.0f, i);
            vertices[i].prevPosition = float3(i, 1.0f, i);
            vertices[i].velocity = float3(0.0f, 5.0 * i + 2.0, 0.0f);
        }
    }

    friend class ParticleRenderProxy;
};

class ParticleRenderProxy : public RenderProxy {

    // rendering parameters
    std::shared_ptr<Shader> shader;
    std::shared_ptr<StaticTriangleMesh> mesh;

    // synced state
    std::vector<float3> positions;
    float radius;

    std::shared_ptr<Particles> simulationObject;

public:
    ParticleRenderProxy(std::shared_ptr<Particles> _simulationObject, std::shared_ptr<Shader> _shader, std::shared_ptr<StaticTriangleMesh> _mesh) :
        simulationObject{ std::move(_simulationObject) },
        shader{ std::move(_shader) },
        mesh{ std::move(_mesh) } {
    }

    void sync() override {
        if (!simulationObject) return;

        positions.clear();
        for (const auto& v : simulationObject->vertices) {
            positions.emplace_back(v.position);
        }

        radius = simulationObject->radius;
    }

    void draw(Renderer& renderer) override {
        if (!simulationObject) return;

        renderer.setShader(shader);

        for (auto& p : positions) {
            float4x4 modelMat = linalg::identity;
            modelMat *= radius;
            modelMat.w = float4(p.x, p.y, p.z, 1.0);

            renderer.setTransform(modelMat);
            mesh->draw();
        }
    }
};


//class VBDStepper3dof {
//    inline void initialize(float timestep, float3& prevPosition, float3& position, float3& velocity) {
//
//        float3 y = prevPosition + timestep * velocity + timestep * timestep * gravity;
//        float3 x_new = y;
//    }
//
//    inline void solveStep(float3& prevPosition, float3& position, float3& velocity, float3 f, float3x3 H) {
//        float3 dx = solveHouseholderQR(H, f);
//        if (length(dx) > 1) {
//            printf("length(dx)=  %f\n", length(dx));
//            printf("H = I[%f %f %f]^T\n", H.x, H.y, H.z);
//            // exit(-1);
//        }
//        // optionally do a line search
//        // float3 dx = float3(f.x/H.x, f.y/H.y, f.z/H.z);
//        position += dx;
//    }
//    
//    inline void updateVelocity(float3 &prevPosition, float3 &position, float3 &velocity) {
//
//    }
//};

void run(){


    Camera mainCamera;
    mainCamera.origin = { 0.0, 0.0, -10 };
    mainCamera.poi = { 0.0, 0.0, 0.0 };
    mainCamera.up = { 0.0, 1.0, 0.0 };

    float globalAspectRatio = float(config.width) / float(config.height);
    float globalFOV = 45.0f; // vertical field of view
    float globalDepthMin = 5.0E-02; // for rasterization
    float globalDepthMax = 100.0f; // for rasterization

    mainCamera.aspect = globalAspectRatio;
    mainCamera.fovy = globalFOV;
    mainCamera.zNear = globalDepthMin;
    mainCamera.zFar = globalDepthMax;

    mainCamera.buildMatrices();

    Renderer renderer(mainCamera);

    std::shared_ptr<Shader> defaultShader = Shader::create(
        (config.resources / "shaders" / "default.vert"), 
        (config.resources / "shaders" / "default.frag")
    );

    std::shared_ptr<StaticTriangleMesh> obj = StaticTriangleMesh::create(
        (config.resources / "models" / "smallsphere.obj")
    );

    auto base_tet_model = (config.resources / "models" / "tet_sphere");
    std::shared_ptr<TetrahedraMesh> obj2 = TetrahedraMesh::create(
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

    // particle system
    std::shared_ptr<Particles> particles = std::make_shared<Particles>(10, 10.0);
    auto particles_proxy_id = renderer.addProxy(std::make_unique<ParticleRenderProxy>(particles, defaultShader, obj));

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

        if (accumulator >= timestep) {
            while (accumulator >= timestep)
            {
                // naive physics update
                for (auto& v : particles->vertices) {
                    float3 y = v.position + timestep * v.velocity + timestep * timestep * gravity;
                    float3 x_new = y;

                    for (int i = 0; i < 5; ++i) {
                        float3 f = -mass * (x_new - y) / (timestep * timestep);
                        float3 H = float3(mass / (timestep * timestep));

                        float3 diff = float3(0.0, planey - x_new.y, 0.0);
                        float pen = dot(diff, planen);
                        if (pen > 0) {
                            f += stiffness * planen * diff;
                            H.y += stiffness;
                        }
                        float3x3 Hmat;
                        for (int i = 0; i < 3; ++i) {
                            Hmat[i][i] = H[i];
                        }



                        float3 dx = solveHouseholderQR(Hmat, f);
                        if (length(dx) > 1) {
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

            renderer.sync();
        }

        renderer.render();
        

        
        
        //// Render
        //// Clear the colorbuffer
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //particles.draw(renderer);
        
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