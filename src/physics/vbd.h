#pragma once

#include <vector>
#include "linalg.h"
#include "engine.h"
#include "resources/meshes.h"

using namespace linalg::aliases;


class PhysicsObject {
    virtual size_t num_vertices() = 0;
    virtual float &mass(size_t id) = 0;
    virtual float3 &prevPosition(size_t id) = 0;
    virtual float3 &position(size_t id) = 0;
    virtual float3 &velocity(size_t id) = 0;
};

class ParticleSystem : public PhysicsObject {
    struct Particle {
        float3 position;
        float3 velocity;
    };
    float mass;
    float radius;

    std::vector<Particle> particles;

    ParticleSystem(size_t n, float radius, float mass) : radius{radius}, mass{mass} {
        for(int i = 0; i < n; ++ i){
            particles.emplace_back();
            particles[i].position = float3(i, 1.0f, i);
            particles[i].velocity = float3(0.0f, 5.0 * i + 2.0, 0.0f);
        }
    }

    size_t num_vertices() override { particles.size();}
    float &mass(size_t id) override  { return mass; }
    float3 &position(size_t id) override { return particles.at(id).position; }
    float3 &velocity(size_t id) override { return particles.at(id).velocity; }

    // life time management is a no-op
    // resource system is used for sharing only
    bool init_impl() { return true; }
    void cleanup_impl() {}    
};

class VBDSolver {

    std::vector<Resource<PhysicsObject>> objects;

    VBDSolver() {}




};