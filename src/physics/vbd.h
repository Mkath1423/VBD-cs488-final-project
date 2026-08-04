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


class VBDSolver {

    std::vector<Resource<PhysicsObject>> objects;

    VBDSolver() {}




};